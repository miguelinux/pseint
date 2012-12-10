#include "FlowEditionManager.h"
#include <wx/socket.h>
#include "mxMainWindow.h"
#include "ConfigManager.h"
#include <iostream>
#include "mxHelpWindow.h"
using namespace std;

FlowEditionManager *flow_editor=NULL;

FlowEditionManager::FlowEditionManager() {
	last_id=0; port=-1;
	flow_editor=this;
	do {
		wxIPV4address adrs;
		adrs.Hostname(_T("127.0.0.1"));
		adrs.Service(port=config->GetFlowPort());
		server=NULL;
		if (server) delete server;
		server = new wxSocketServer(adrs,wxSOCKET_NOWAIT);
		server->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
		server->SetNotify(wxSOCKET_CONNECTION_FLAG);
		server->Notify(true);
	} while (!server->IsOk());
}

void FlowEditionManager::SocketEvent(wxSocketEvent *event) {
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
					ProcData(event->GetSocket(),sbuffer);
					sbuffer=_T("");
					aux=buf+i+1;
				}
			event->GetSocket()->Read(buf,255);
			n = event->GetSocket()->LastCount();
		}
	} else if (event->GetSocketEvent()==wxSOCKET_LOST) {
		if (socket==event->GetSocket()) { socket=NULL; }
		else {
			mxSource *src=main_window->FindFlowSocket(event->GetSocket());
			if (src) src->SetFlowSocket(NULL);
		}
		event->GetSocket()->Destroy();
	}
}

FlowEditionManager::~FlowEditionManager() {
	flow_editor=NULL;
}

void FlowEditionManager::ProcData(wxSocketBase *s, wxString data) {
	if (s==socket) {
		if (data.StartsWith("hello ")) {
			long id=-1; data.AfterFirst(' ').ToLong(&id);
			mxSource *src = main_window->FindSourceById(id);
			if (src) {
				src->SetFlowSocket(s); socket=NULL;
			}
		}
	} else {
		if (data=="help") {
			if (!helpw) helpw = new mxHelpWindow();
			helpw->ShowHelp(_T("flow.html"));
			return;
		}
		mxSource *src=main_window->FindFlowSocket(s);
		if (!src) return;
		if (data=="reload") {
			src->ReloadTemp(config->GetTempPSD());
			main_window->SelectSource(src);
		} else if (data=="run") {
			src->ReloadTemp(config->GetTempPSD());
			main_window->SelectSource(src);
			wxCommandEvent evt;
			main_window->OnRunRun(evt);
		}
	}
}

int FlowEditionManager::GetNextId ( ) {
	return ++last_id;
}

bool FlowEditionManager::HasSocket (wxObject * s) {
	return s && (s==server||s==socket||main_window->FindFlowSocket(s));
}

int FlowEditionManager::GetPort ( ) {
	return port;
}

