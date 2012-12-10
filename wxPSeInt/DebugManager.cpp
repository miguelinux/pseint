#include "DebugManager.h"
#include "mxSource.h"
#include "mxMainWindow.h"
#include "mxProcess.h"
#include "mxDesktopTest.h"
#include <iostream>
#include "mxEvaluateDialog.h"
#include "ConfigManager.h"
#include "mxDebugWindow.h"
#include "mxSubtitles.h"
using namespace std;

DebugManager *debug;

DebugManager::DebugManager() {
	do_desktop_test=false;
	debugging=false;
//	server=NULL;
//	port=-1;
	step_in=true;
	subtitles_on=false;
}

void DebugManager::Start(mxProcess *proc, mxSource *src) {
	socket=NULL;
	current_proc_name="<desconocido>";
	process = proc;
	sbuffer=_T("");
	source=src;
	debugging=true;
	paused=false;
	subtitles->Reset();
	desktop_test->ResetTest();
	if (do_desktop_test) 
		main_window->ShowDesktopTestGrid(true);
	
//	if (server!=NULL) return;
//	
//	do {
//		if (server) delete server;
//		wxIPV4address adrs;
//		adrs.Hostname(_T("127.0.0.1"));
//		adrs.Service(port=config->GetDebugPort());
//		server = new wxSocketServer(adrs,wxSOCKET_NOWAIT);
//		server->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
//		server->SetNotify(wxSOCKET_CONNECTION_FLAG);
//		server->Notify(true);
//	} while (!server->IsOk());
	return;
	
}

void DebugManager::ProcSocketData(wxString data) {
	if (data.StartsWith(_T("subtitulo "))) {
		subtitles->AddMessage(current_line,current_inst,data.Mid(10));
	} if (data.StartsWith(_T("proceso "))) {
		current_proc_name=data.Mid(8);
	} else if (data.StartsWith(_T("linea "))) {
		long l=-1,i=-1;
		if (data.Contains(":")) {
			data.Mid(6).BeforeFirst(':').ToLong(&l);
			data.AfterLast(':').ToLong(&i);
		} else {
			data.Mid(6).ToLong(&l);
		}
		if (l>=0 && source!=NULL) source->SetDebugLine(l-1,i-1);
		if (do_desktop_test) desktop_test->SetLine(current_proc_name,l,i);
		current_line=l; current_inst=i;
	} else if (data.StartsWith(_T("autoevaluacion "))) {
		long l=-1;
		data.Mid(15).BeforeFirst(' ').ToLong(&l);
		desktop_test->SetAutoevaluation(l,data.Mid(15).AfterFirst(' '));
	} else if (data.StartsWith(_T("estado "))) {
		wxString state = data.AfterFirst(' ');
		if (state==_T("inicializado")) {
			// cargar la prueba de escritorio
			if (do_desktop_test) {
				const wxArrayString &vars  = desktop_test->GetDesktopVars();
				for (unsigned int i=0;i<vars.GetCount();i++) {
					wxString str(wxString(_T("autoevaluar "))<<vars[i]<<_T("\n"));
					socket->Write(str.c_str(),str.Len());
				}
			}
			// configurar el tipo de paso a paso (step in o step over)
			wxString str1(_T("subprocesos ")); str1<<(step_in?1:0)<<_T("\n");
			socket->Write(str1.c_str(),str1.Len());
			wxString str2(_T("subtitulos ")); str2<<(subtitles_on?1:0)<<_T("\n");
			socket->Write(str2.c_str(),str2.Len());
			// iniciar la ejecución
			if ((paused=should_pause)) {
				wxString str(_T("paso\n"));
				socket->Write(str.c_str(),str.Len());
			}	else {
				wxString str(_T("comenzar\n"));
				socket->Write(str.c_str(),str.Len());
			}
		} else if (state==_T("pausa")) {
			debug_panel->SetState(DS_PAUSED);
			if (source) source->SetDebugPause();
		} else if (state==_T("paso"))
			debug_panel->SetState(DS_STEP);
		else if (state==_T("ejecutando"))
			debug_panel->SetState(DS_RESUMED);
		else if (state==_T("finalizado")) {
			debug_panel->SetState(DS_FINALIZED);
			if (source) source->SetDebugLine();
		} else {
			if (source) source->SetDebugLine();
			debug_panel->SetState(DS_NONE);
		}
	} else if (data.StartsWith(_T("evaluacion "))) {
		debug_panel->SetEvaluationValue(data.Mid(13),data[11]);
	}	
}

void DebugManager::Close(mxSource *src) {
	if (source==src) {
		if (debugging) Stop();
		source=NULL;
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

void DebugManager::SetStepIn(bool b) {
	step_in=b;
	if (debugging && socket) {
		wxString str(_T("subprocesos ")); str<<(step_in?1:0)<<_T("\n");
		socket->Write(str.c_str(),str.Len());
	}
}

void DebugManager::SetSubtitles(bool b) {
	subtitles_on=b;
	if (debugging && socket) {
		wxString str(_T("subtitulos ")); str<<(step_in?1:0)<<_T("\n");
		socket->Write(str.c_str(),str.Len());
	}
}

void DebugManager::SetSocket (wxSocketBase * s) {
	socket=s;
}

void DebugManager::ProcessSocketLost() {
	debugging=false;
	socket=NULL;
}
