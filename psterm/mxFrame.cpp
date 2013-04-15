#include "mxFrame.h"
#include <wx/sizer.h>
#include "mxConsole.h"
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/app.h>
#include <iostream>
using namespace std;

enum { FRAME_ID_BASE=wxID_HIGHEST+1000, FRAME_ID_PLAY, FRAME_ID_RELOAD };

BEGIN_EVENT_TABLE(mxFrame,wxFrame)
	EVT_BUTTON(FRAME_ID_RELOAD,mxFrame::OnButtonReload)
	EVT_BUTTON(FRAME_ID_PLAY,mxFrame::OnButtonPlay)
	EVT_SOCKET(wxID_ANY,mxFrame::OnSocketEvent)
	EVT_SCROLL(mxFrame::OnScroll)
	EVT_CLOSE(mxFrame::OnClose)
	EVT_ACTIVATE(mxFrame::OnFocus)
END_EVENT_TABLE()

mxFrame::mxFrame(wxString command, int port, int id, bool debug, win_props props)
	:wxFrame(NULL,wxID_ANY,"PSeInt - Ejecutando...",
	wxDefaultPosition, wxSize(props.width,props.height),
	wxDEFAULT_FRAME_STYLE|(props.always_on_top?wxSTAY_ON_TOP:0)) {
	
		debug_mode=debug;
		already_connected=false;
		src_id=id;
		scroll = new wxScrollBar(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxSB_VERTICAL);
		console=new mxConsole(this,scroll);
		play_from_here=new wxButton(this,FRAME_ID_PLAY," Ejecutar desde este punto ",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
		is_present=true; play_from_here->Hide();
		
		wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *sizer_in = new wxBoxSizer(wxVERTICAL);
		sizer_in->Add(console,wxSizerFlags().Proportion(1).Expand());
		sizer_in->Add(play_from_here,wxSizerFlags().Right());
#ifdef DEBUG
//		sizer_in->Add(new wxButton(this,FRAME_ID_RELOAD,"Reload"),wxSizerFlags().Right());
#endif
		sizer->Add(sizer_in,wxSizerFlags().Proportion(1).Expand());
		sizer->Add(scroll,wxSizerFlags().Proportion(0).Expand());
		SetSizer(sizer);
		Show();
		wxTimerEvent evt;
		if (port!=-1) InitSocket(port); else socket=NULL;
		console->Run(command);
		console->SetFocus();
		if (props.set_left||props.set_right||props.set_bottom||props.set_top) {
			wxYield();
			int x,y,w,h;
			GetScreenPosition(&x,&y); GetSize(&w,&h);
			if (props.set_left||props.set_right) x= props.set_left?props.left:(props.right-w);
			if (props.set_bottom||props.set_top) y= props.set_top?props.top:(props.bottom-h);
			Move(x,y);
		}
}

void mxFrame::OnButtonPlay (wxCommandEvent & evt) {
	console->PlayFromCurrentEvent();
	console->SetFocus();
}

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
		already_connected=true;
		if (tosend_after_hello.Len()) 
			socket->Write(tosend_after_hello.c_str(),tosend_after_hello.Len());
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
	else if (buffer=="raise") Raise();
	else if (buffer=="quit") Close();
	else if (buffer=="dimm") console->Dimm();
}

void mxFrame::OnProcessTerminated ( ) {
//	if (debug_mode) Close();
	if (!socket) return;
	wxString msg("terminated\n");
	if (already_connected)
		socket->Write(msg.c_str(),msg.Len());
	else 
		tosend_after_hello<<msg;
}

void mxFrame::OnClose (wxCloseEvent & event) {
	console->KillProcess();
	wxExit();
}

void mxFrame::OnScroll (wxScrollEvent & event) {
	console->SetTime(scroll->GetThumbPosition());
	console->SetFocus();
}

void mxFrame::SetIsPresent (bool is) {
	if (is==is_present || debug_mode) return;
	is_present=is;
	play_from_here->Show(!is);
	Layout();
}

void mxFrame::OnButtonReload (wxCommandEvent & evt) {
	console->Reload();
}

void mxFrame::OnFocus (wxActivateEvent & evt) {
	if (socket && already_connected && evt.GetActive()) {
		wxString msg("activated\n");
		socket->Write(msg.c_str(),msg.Len());
	}
	evt.Skip();
}

