#include "mxConsole.h"
#include <wx/dcclient.h>
#include <iostream>
#include <wx/msgdlg.h>
#include <wx/txtstrm.h>
using namespace std;

enum { CONSOLE_ID_BASE=wxID_HIGHEST, CONSOLE_ID_TIMER_SIZE, CONSOLE_ID_TIMER_CARET, CONSOLE_ID_TIMER_PROCESS };

BEGIN_EVENT_TABLE(mxConsole,wxPanel)
	EVT_PAINT(mxConsole::OnPaint)
	EVT_SIZE(mxConsole::OnSize)
	EVT_CHAR(mxConsole::OnChar)
	EVT_TIMER(CONSOLE_ID_TIMER_SIZE,mxConsole::OnTimerSize)
	EVT_TIMER(CONSOLE_ID_TIMER_CARET,mxConsole::OnTimerCaret)
	EVT_TIMER(CONSOLE_ID_TIMER_PROCESS,mxConsole::OnTimerProcess)
	EVT_END_PROCESS(wxID_ANY,mxConsole::OnProcessTerminate)
END_EVENT_TABLE()
	
#define _buffer(i,j) buffer[(i)*buffer_w+(j)]
#define _CARET_TIME 500
#define _PROCESS_TIME 10

static wxColour colors[16] = {
	wxColour(0  ,0  ,0),
	wxColour(127,0  ,0),
	wxColour(0  ,127,0),
	wxColour(127,127,0),
	wxColour(0  ,0  ,255),
	wxColour(127,0  ,127),
	wxColour(0  ,127,127),
	wxColour(127,127,127),
	wxColour(0  ,  0,0),
	wxColour(255,  0,0),
	wxColour(0  ,255,0),
	wxColour(255,255,0),
	wxColour(127,127,255),
	wxColour(255,  0,255),
	wxColour(0  ,255,255),
	wxColour(255,255,255)
};
	
mxConsole::mxConsole(wxWindow *parent):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize) {
	the_process=NULL;
	timer_size=new wxTimer(this,CONSOLE_ID_TIMER_SIZE);
	timer_caret=new wxTimer(this,CONSOLE_ID_TIMER_CARET);
	timer_process=new wxTimer(this,CONSOLE_ID_TIMER_PROCESS);
	buffer=NULL;
	Reset();
	SetFontSize(12);
	timer_size->Start(100,true);
	timer_caret->Start(_CARET_TIME,false);
}

void mxConsole::OnPaint (wxPaintEvent & event) {
	if (!buffer) return;
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetBackground(colors[bg]);
	dc.SetTextBackground(colors[bg]);
	dc.Clear();
	dc.SetFont(font);
	for(int i=0;i<buffer_h;i++) { 
		int lj=0; wxString line;
		for(int j=0;j<buffer_w;j++) {
			line<<_buffer(i,j).the_char;
			if (j+1==buffer_w||_buffer(i,j).fg!=_buffer(i,j+1).fg) {
				dc.SetTextForeground(colors[_buffer(i,j).fg]);
				dc.DrawText(line,margin+lj*char_w,margin+i*char_h);
				line.Clear();
				lj=j+1;
			}
		}
	}
	if (caret_visible && blinking_caret_aux) {
		dc.SetTextForeground(colors[cur_fg]);
		dc.DrawText(wxString()<<"|",margin+cur_x*char_w-char_w/2,margin+cur_y*char_h);
	}
}

void mxConsole::OnSize (wxSizeEvent & event) {
	timer_size->Start(100,true);
}

void mxConsole::OnChar (wxKeyEvent & event) {
	if (the_process) {
		wxOutputStream *output=the_process->GetOutputStream();
		char c=char(event.GetKeyCode());
		if (c=='\r') c='\n';
		output->Write(&c,1);
		Print(wxString()<<c);
		Refresh();
	}
}

void mxConsole::SetFontSize (int size) {
	font = wxFont(size,wxFONTFAMILY_TELETYPE,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	wxClientDC dc(this); dc.SetFont(font);
	char_h=dc.GetCharHeight(); char_w=dc.GetCharWidth();
}

void mxConsole::Print (wxString text, bool record) {
	if (record) history<<text;
	if (!buffer) return;
	int l=text.Len();
	for(int i=0;i<l;i++) {
		if (text[i]=='\n'||text[i]=='\r') {
			cur_x=buffer_w;
		} else if (text[i]=='\b') {
			if (--cur_x<0) {
				cur_x=buffer_w-1;
				if (cur_y>0) cur_y--;
			}
			_buffer(cur_y,cur_x).the_char=' ';
		} else {
			_buffer(cur_y,cur_x).the_char=text[i];
			_buffer(cur_y,cur_x).fg=cur_fg;
			cur_x++;
		}
		if (cur_x>=buffer_w) {
			cur_x=0; cur_y++;
			if (cur_y>=buffer_h) { // si hay que hacer scroll
				cur_y=buffer_h-1;
				for(int i=1;i<buffer_h;i++) { 
					for(int j=0;j<buffer_w;j++) { 
						_buffer(i-1,j)=_buffer(i,j);
					}
				}
				for(int j=0;j<buffer_w;j++) { 
					_buffer(buffer_h-1,j).the_char=' ';
					_buffer(buffer_h-1,j).fg=cur_fg;
				}
			}
		}
	}
}

void mxConsole::ShowCaret (bool show, bool record) {
	caret_visible=show;
	if (!caret_visible) timer_caret->Stop();
	else timer_caret->Start(_CARET_TIME,false);
	if (record) {
		if (show) history<<"\033[?25h";
		else history<<"\033[?25l";
	}
}

wxString mxConsole::GetInput ( ) {
	
}

void mxConsole::WaitForKey ( ) {
	
}

void mxConsole::Clear (bool record) {
	if (record) {
		history<<"\033[2J";
		last_clear=history.Len();
	}
	for(int i=0;i<buffer_h;i++) { 
		for(int j=0;j<buffer_w;j++) { 
			_buffer(i,j).the_char=' ';
		}
	}
}

void mxConsole::Process (wxString input, bool record) {
	int i=record?0:last_clear,l=input.Len(); int i0=i;
	while (i<l) {
		if (input[i]=='\033' && input[i+1]=='[') {
			if (i-i0) Print(input.Mid(i0,i-i0),record);	
			if (input[i+2]=='2' && input[i+3]=='J') {
				Clear(record); i+=3;
			} else if (input[i+2]=='?' && input[i+3]=='2' && input[i+4]=='5' && input[i+5]=='l') {
				ShowCaret(false,record); i+=5;
			} else if (input[i+2]=='?' && input[i+3]=='2' && input[i+4]=='5' && input[i+5]=='h') {
				ShowCaret(true,record); i+=5;
			} else if (input[i+2]=='4' && input[i+4]=='m') {
				// background color, ignored
				i+=4; 
			} else if (input[i+2]=='0' && input[i+3]=='m') {
				if (cur_fg>7) cur_fg-=8; 
				if (record) history<<"\033[0m";
				i+=3;
			} else if (input[i+2]=='1' && input[i+3]=='m') {
				if (cur_fg<8) cur_fg+=8; 
				if (record) history<<"\033[1m";
				i+=3;
			} else if (input[i+2]=='3' && (input[i+3]>='0'&&input[i+3]<='7') && input[i+4]=='m') {
				if (record) history<<"\033[3"<<input[i+3]<<"m";
				int last_fg=cur_fg;
				char c=input[i+3];
				cur_fg=(cur_fg/8)*8+(input[i+3]-'0');
				i+=4;
			} else {
				int x0=i+2; int x1=x0;
				while (input[x1]>='0'&&input[x1]<='9') x1++;
				char c=input[x1];
				int y0=x1+1; int y1=y0;
				while (input[y1]>='0'&&input[y1]<='9') y1++;
				if (x0!=x1 && y0!=y1 && c==';' && input[y1]=='H') {
					long x; input.Mid(x0,x1-x0).ToLong(&x);
					long y; input.Mid(y0,y1-y0).ToLong(&y);
					GotoXY(x-1,y-1,record);
				}
				i=y1;
			}
			i0=i+1;
		}
		i++;
	}
	if (i-i0) Print(input.Mid(i0,i0-i),record);	
}

void mxConsole::Reset ( ) {
	history=""; last_clear=0; 
	cur_x=cur_y=0;
	bg=0; cur_fg=15;
	blinking_caret_aux=false;
	caret_visible=true;
	Clear(false);
}

void mxConsole::OnTimerCaret (wxTimerEvent & event) {
	blinking_caret_aux=!blinking_caret_aux;
	Refresh();
}

void mxConsole::OnTimerSize (wxTimerEvent & event) {
	wxSize size = GetClientSize();
	buffer_w = (size.x - 2*margin) / char_w;
	buffer_h = (size.y - 2*margin) / char_h;
	if (buffer_w<1) buffer_w=1;
	if (buffer_h<1) buffer_h=1;
	delete [] buffer;
	buffer=new console_char[buffer_w*buffer_h];
	cur_x=cur_y=0; Process(history, false);
	Refresh();
}

void mxConsole::Run (wxString command) {
	Reset();
	if (the_process) { 
		the_process->Kill(the_process_pid,wxSIGKILL);
		the_process->Detach();
	}
	the_process=new wxProcess(this->GetEventHandler(),wxID_ANY);
	the_process->Redirect();
	the_process_pid=wxExecute(command,wxEXEC_ASYNC,the_process);
	if (the_process_pid>0) timer_process->Start(_PROCESS_TIME,false);
}

void mxConsole::OnTimerProcess (wxTimerEvent & event) {
	GetProcessOutput();
}
void mxConsole::GetProcessOutput () {
	if (!the_process) return;
	wxTextInputStream input(*(the_process->GetInputStream()));
	wxString line;
	while (the_process->IsInputAvailable())
		line<<input.GetChar();
	if (line.Len()) { Process(line); Refresh(); }
	for(int i=0;i<line.Len();i++) { if (line[i]=='\033') line[i]='#'; }
}

void mxConsole::OnProcessTerminate( wxProcessEvent &event ) {
	if (event.GetPid()==the_process_pid) {
		GetProcessOutput();
		the_process->Detach();
		the_process=NULL;
		timer_process->Stop();
		timer_caret->Stop(); caret_visible=false;
	}	
}

void mxConsole::GotoXY (int x, int y, bool record) {
	if (record) history<<"\033["<<y<<';'<<x<<'H';
	cur_x=x; if (x>=buffer_w) x=buffer_w;
	cur_y=y; if (y>=buffer_h) y=buffer_h;
}

