#include <wx/socket.h>
#include <iostream>
#include "CommunicationsManager.h"
#include "mxMainWindow.h"
#include "ConfigManager.h"
#include "mxHelpWindow.h"
#include "DebugManager.h"
#include "mxStatusBar.h"
using namespace std;

enum {MXS_TYPE_UNKNOWN, MXS_TYPE_DEBUG, MXS_TYPE_FLOW, MXS_TYPE_RUN};

#ifdef DEBUG
#define _ERROR(str) wxMessageBox(str)
#else
#define _ERROR(str)
#endif

CommunicationsManager *comm_manager=NULL;


mxSocketClient::mxSocketClient(wxSocketBase *s):type(MXS_TYPE_UNKNOWN),src_id(-1),socket(s){
	cerr<<"new mxSocketClient "<<this<<endl;
	socket->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
	socket->SetNotify(wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG);
	socket->Notify(true);
}

mxSocketClient::~mxSocketClient() {
	cerr<<"delete mxSocketClient "<<this<<endl;
	if (socket) delete socket;
}

void mxSocketClient::ProcessLost() {
	if (type==MXS_TYPE_DEBUG) debug->ProcSocketData(buffer);
	else if (type==MXS_TYPE_FLOW) ProcessLostFlow();
	else if (type==MXS_TYPE_RUN) ProcessLostRun();
	else _ERROR("mxSocketClient::ProcessLost error");
}

void mxSocketClient::ProcessLostRun() {
	mxSource *src=main_window->FindSourceById(src_id);
	if (src) src->SetRunSocket(NULL);
}

void mxSocketClient::ProcessLostFlow() {
	mxSource *src=main_window->FindSourceById(src_id);
	if (src) src->SetFlowSocket(NULL);
}

void mxSocketClient::ProcessInput (wxSocketEvent & evt) {
	// leer datos y procesar
	wxChar buf[256];
	socket->Read(buf,255);
	int n = socket->LastCount();
	while (n>0) {
		buf[n]='\0';
		wxChar *aux=buf;
		for (int i=0;i<n;i++)
			if (buf[i]=='\n') {
				buf[i]='\0';
				buffer<<aux;
				ProcessCommand();
				buffer=_T("");
				aux=buf+i+1;
			}
		socket->Read(buf,255);
		n = socket->LastCount();
	}
}

void mxSocketClient::ProcessCommand ( ) {
	if (type==MXS_TYPE_DEBUG) debug->ProcSocketData(buffer);
	else if (type==MXS_TYPE_FLOW) ProcessCommandFlow();
	else if (type==MXS_TYPE_RUN) ProcessCommandRun();
	else if (buffer.StartsWith("hello-")) {
		wxString stype=buffer.AfterFirst('-').BeforeFirst(' ');
		if (stype=="flow") {
			long id=-1; if (!buffer.AfterFirst(' ').ToLong(&id)) { _ERROR("mxSocketClient::ProcessCommand::hello-id error"); return; } src_id=id;
			type=MXS_TYPE_FLOW;
			mxSource *src=main_window->FindSourceById(src_id);
			if (src) src->SetFlowSocket(socket);
		} else if (stype=="run") {
			long id=-1; if (!buffer.AfterFirst(' ').ToLong(&id)) { _ERROR("mxSocketClient::ProcessCommand::hello-id error"); return; } src_id=id;
			type=MXS_TYPE_RUN;
			mxSource *src=main_window->FindSourceById(src_id);
			if (src) src->SetRunSocket(socket);
		} else if (stype=="debug") {
			type=MXS_TYPE_DEBUG;
			debug->SetSocket(socket);
		}
		else { _ERROR("mxSocketClient::ProcessCommand::hello-type error"); return; }
		
	}
}

void mxSocketClient::ProcessCommandRun() {
	if (buffer=="terminated") {
		mxSource *src=main_window->FindSourceById(src_id);
		if (src) main_window->ParseResults(src);
	} else if (buffer=="activated") {
		mxSource *src=main_window->FindSourceById(src_id);
		if (src && src->GetStatus()==STATUS_RUNNING_CHANGED) src->UpdateRunningTerminal(false);
	}
}

void mxSocketClient::ProcessCommandFlow() {
	if (buffer=="help") {
		if (!helpw) helpw = new mxHelpWindow();
		helpw->ShowHelp(_T("flow.html"));
		return;
	}
	mxSource *src=main_window->FindSourceById(src_id);
	if (!src) { _ERROR("mxSocketClient::ProcessCommandFlow::src error"); return; }
	if (buffer=="reload"||buffer=="run"||buffer=="export") {
		src->ReloadFromTempPSD();
		main_window->SelectSource(src);
		if (buffer=="run") main_window->RunCurrent(true);
		else if (buffer=="export") { wxCommandEvent evt; main_window->OnRunSaveFlow(evt); }
	}
}

CommunicationsManager::CommunicationsManager() {
	server_port=-1;
	server=NULL;
	do {
		wxIPV4address adrs;
		adrs.Hostname(_T("127.0.0.1"));
		adrs.Service(server_port=config->GetCommPort());
		if (server) delete server;
		server = new wxSocketServer(adrs,wxSOCKET_NOWAIT);
		server->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
		server->SetNotify(wxSOCKET_CONNECTION_FLAG);
		server->Notify(true);
	} while (!server->IsOk());
}

void CommunicationsManager::SocketEvent(wxSocketEvent &event) {
	wxSocketBase *s=event.GetSocket();
	if (s==server) { // si se pedia una nueva conexion, se crea un socket en sockets
		clients.push_back(new mxSocketClient(server->Accept(false)));
	} else if (event.GetSocketEvent()==wxSOCKET_INPUT) { // si son datos de entrada...
		list<mxSocketClient*>::iterator it1=clients.begin(), it2=clients.end();
		while (it1!=it2) {
			if ((**it1)==s) { 
				(*it1)->ProcessInput(event);
				return;
			}
			it1++;
		}
		_ERROR("CommunicationsManager::SocketEvent::Input error");
	} else if (event.GetSocketEvent()==wxSOCKET_LOST) { // si por algo anormal se corto una conexion pendiente, liberar en sockets
		list<mxSocketClient*>::iterator it1=clients.begin(), it2=clients.end();
		while (it1!=it2) {
			if ((**it1)==s) {
				(*it1)->ProcessLost();
				delete *it1;
				clients.erase(it1);
				return;
			}
			it1++;
		}
		_ERROR("CommunicationsManager::SocketEvent::Lost error");
	}
}

int CommunicationsManager::GetServerPort ( ) {
	return server_port;
}

CommunicationsManager::~CommunicationsManager() {
	comm_manager=NULL;
}

