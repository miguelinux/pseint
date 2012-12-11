#include "mxFrame.h"
#include <wx/sizer.h>
#include "mxConsole.h"
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/app.h>

enum { FRAME_ID_BASE=wxID_HIGHEST+1000/*, FRAME_ID_RELOAD*/ };

BEGIN_EVENT_TABLE(mxFrame,wxFrame)
//	EVT_BUTTON(FRAME_ID_RELOAD,mxFrame::OnButtonReload)
	EVT_SOCKET(wxID_ANY,mxFrame::OnSocketEvent)
END_EVENT_TABLE()

mxFrame::mxFrame(wxString command, int port, int id):wxFrame(NULL,wxID_ANY,"console test",wxDefaultPosition,wxDefaultSize) {
	src_id=id;
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	console=new mxConsole(this);
	sizer->Add(console,wxSizerFlags().Proportion(1).Expand());
//	wxBoxSizer *sizer_but = new wxBoxSizer(wxHORIZONTAL);
//	sizer_but->Add(new wxButton(this,FRAME_ID_RELOAD,"Reload"));
//	sizer->Add(sizer_but,wxSizerFlags().Proportion(0).Expand());
//	SetSizer(sizer);
	Show();
	wxTimerEvent evt;
	if (port!=-1) InitSocket(port);
	console->Run(command);
	console->SetFocus();
}

//void mxFrame::OnButtonReload (wxCommandEvent & evt) {
//	console->Reload();
//	console->SetFocus();
//}

void mxFrame::InitSocket (int port) {
	socket=new wxSocketClient(wxSOCKET_NOWAIT);
	socket->SetEventHandler(*(this->GetEventHandler()),wxID_ANY);
	wxIPV4address address;
	address.Hostname("localhost");
	address.Service(port);
	socket->SetNotify(wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG|wxSOCKET_CONNECTION_FLAG);
	socket->Notify(true);
	socket->Connect(address,false);
}

void mxFrame::OnSocketEvent (wxSocketEvent & event) {
	if (event.GetSocketEvent()==wxSOCKET_CONNECTION) {
		wxString msg("hello-run "); msg<<src_id<<"\n";
		socket->Write(msg.c_str(),msg.Len());
	} if (event.GetSocketEvent()==wxSOCKET_INPUT) {
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
	} if (event.GetSocketEvent()==wxSOCKET_LOST) {
		delete socket;
		socket=NULL;
	}
}

void mxFrame::ProcessCommand ( ) {
	if (buffer=="reload") console->Reload();
	else if (buffer=="quit") wxExit();
}

void mxFrame::OnProcessTerminated ( ) {
	if (!socket) return;
	wxString msg("terminated\n");
	socket->Write(msg.c_str(),msg.Len());
}

