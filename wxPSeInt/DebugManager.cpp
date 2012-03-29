#include "DebugManager.h"
#include "mxSource.h"
#include "mxMainWindow.h"
#include "mxProcess.h"
#include "mxDesktopTest.h"
#include <iostream>
#include "mxEvaluateDialog.h"
#include "ConfigManager.h"
using namespace std;

DebugManager *debug;

DebugManager::DebugManager(mxDesktopTest *dt, mxEvaluateDialog *ed) {
	desktop_test=dt;
	evaluate_dialog=ed;
	do_desktop_test=false;
	debugging=false;
	server=NULL;
	port=-1;
}

DebugManager::~DebugManager() {
	
}

void DebugManager::Start(mxProcess *proc, mxSource *src) {
	
	process = proc;
	main_window->debug_status->SetLabel(_T("iniciando..."));
	sbuffer=_T("");
	source=src;
	debugging=true;
	paused=false;

	desktop_test->ResetTest();
	if (do_desktop_test)
		main_window->ShowDesktopTestGrid();
	
	if (server!=NULL) return;
	
	do {
		if (server) delete server;
		wxIPV4address adrs;
		adrs.Hostname(_T("127.0.0.1"));
		adrs.Service(port=config->GetDebugPort());
		server = new wxSocketServer(adrs,wxSOCKET_NOWAIT);
		server->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
		server->SetNotify(wxSOCKET_CONNECTION_FLAG);
		server->Notify(true);
	} while (!server->IsOk());
	return;
	
}

void DebugManager::ProcData(wxString data) {
	if (data.StartsWith(_T("linea "))) {
		long l=-1;
		data.Mid(6).ToLong(&l);
		if (l>=0 && source!=NULL)
			source->SetDebugLine(l);
//			source->SetSelection(source->GetLineEndPosition(l),source->GetLineIndentPosition(l));
		if (do_desktop_test)
			desktop_test->SetLine(l+1);
	} else if (data.StartsWith(_T("autoevaluacion "))) {
		long l=-1;
		data.Mid(15).BeforeFirst(' ').ToLong(&l);
		desktop_test->SetAutoevaluation(l,data.Mid(15).AfterFirst(' '));
	} else if (data.StartsWith(_T("estado "))) {
		wxString state = data.AfterFirst(' ');
		if (state==_T("inicializado")) {
			if (do_desktop_test) {
				const wxArrayString &vars  = desktop_test->GetDesktopVars();
				for (unsigned int i=0;i<vars.GetCount();i++) {
					wxString str(wxString(_T("autoevaluar "))<<vars[i]<<_T("\n"));
					socket->Write(str.c_str(),str.Len());
				}
			}
			if ((paused=should_pause)) {
				wxString str(_T("paso\n"));
				socket->Write(str.c_str(),str.Len());
			}	else {
				wxString str(_T("comenzar\n"));
				socket->Write(str.c_str(),str.Len());
			}
		} else if (state==_T("pausa"))
			main_window->SetDebugState(DS_PAUSED);
		else if (state==_T("paso"))
			main_window->SetDebugState(DS_STEP);
		else if (state==_T("ejecutando"))
			main_window->SetDebugState(DS_RESUMED);
		else {
			source->SetDebugLine();
			main_window->SetDebugState(DS_NONE);
		}
	} else if (data.StartsWith(_T("evaluacion "))) {
		evaluate_dialog->SetEvaluationValue(data.Mid(11));
	}	
}

void DebugManager::Close(mxSource *src) {
	if (source==src) source=NULL;
}

void DebugManager::SocketEvent(wxSocketEvent *event) {
	if (event->GetSocket()==server) {
		socket = server->Accept(false);
		socket->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
		socket->SetNotify(wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG);
		socket->Notify(true);
	} else if (event->GetSocketEvent()==wxSOCKET_INPUT) {
		wxChar buf[256];
		event->GetSocket()->Read(buf,255);
		int n = event->GetSocket()->LastCount();
		while (n>0) {
			buf[n]='\0';
			wxChar *aux=buf;
			for (int i=0;i<n;i++)
				if (buf[i]=='\n') {
					buf[i]='\0';
					sbuffer<<aux;
					ProcData(sbuffer);
					sbuffer=_T("");
					aux=buf+i+1;
				}
			event->GetSocket()->Read(buf,255);
			n = event->GetSocket()->LastCount();
		}
	} else if (event->GetSocketEvent()==wxSOCKET_LOST) {
		if (socket==event->GetSocket()) {
			debugging=false;
			socket=NULL;
		}
		event->GetSocket()->Destroy();
//	} else if (event->GetSocketEvent()==wxSOCKET_CONNECTION) {
	}
}

void DebugManager::SetSpeed(int speed) {
	if (debugging && socket) {
		int sp = _calc_delay(speed);
		wxString str(_T("delay "));
		str<<sp<<'\n';
		socket->Write(str.c_str(),str.Len());
	}
}

int DebugManager::GetSpeed(int sp) const {
	return 100-((sp-25)/20);
}

void DebugManager::Step() {
	if (debugging && socket) {
		wxString str(_T("paso\n"));
		socket->Write(str.c_str(),str.Len());
	}
}

bool DebugManager::Pause() {
	paused = !paused;
	if (debugging && socket) {
		wxString str(_T("pausa\n"));
		socket->Write(str.c_str(),str.Len());
	}
	return paused;
}

void DebugManager::Stop() {
	if (process && process->pid!=0 && process->Exists(process->pid)) {
		wxProcess::Kill(process->pid,wxSIGKILL);
	}
}

void DebugManager::SetDoDesktopTest(bool val) {
	do_desktop_test = val;
}

void DebugManager::SendEvaluation(wxString exp) {
	wxString str(_T("evaluar "));
	str<<exp<<_T("\n");
	socket->Write(str.c_str(),str.Len());	
}

bool DebugManager::HasSocket (wxObject *s) {
	return s && (s==socket||s==server);
}

int DebugManager::GetPort ( ) {
	return port;
}

