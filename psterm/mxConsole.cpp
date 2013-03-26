#include <iostream>
#include <wx/dcclient.h>
#include <wx/msgdlg.h>
#include <wx/txtstrm.h>
#include <wx/app.h>
#include <wx/menu.h>
#include <wx/clipbrd.h>
#include "mxFrame.h"
#include "mxConsole.h"
#include <wx/dataobj.h>
using namespace std;

enum { CONSOLE_ID_BASE=wxID_HIGHEST, CONSOLE_ID_TIMER_SIZE, CONSOLE_ID_TIMER_CARET, CONSOLE_ID_TIMER_PROCESS, CONSOLE_ID_PASTE, CONSOLE_ID_COPY };

BEGIN_EVENT_TABLE(mxConsole,wxPanel)
	EVT_PAINT(mxConsole::OnPaint)
	EVT_SIZE(mxConsole::OnSize)
	EVT_CHAR(mxConsole::OnChar)
	EVT_TIMER(CONSOLE_ID_TIMER_SIZE,mxConsole::OnTimerSize)
	EVT_TIMER(CONSOLE_ID_TIMER_CARET,mxConsole::OnTimerCaret)
	EVT_TIMER(CONSOLE_ID_TIMER_PROCESS,mxConsole::OnTimerProcess)
	EVT_END_PROCESS(wxID_ANY,mxConsole::OnProcessTerminate)
	EVT_MOUSEWHEEL(mxConsole::OnMouseWheel)
	EVT_RIGHT_DOWN(mxConsole::OnMouseRightDown)
	EVT_LEFT_DOWN(mxConsole::OnMouseLeftDown)
	EVT_LEFT_UP(mxConsole::OnMouseLeftUp)
	EVT_MOTION(mxConsole::OnMouseMotion)
	EVT_MENU(CONSOLE_ID_PASTE,mxConsole::OnPaste)
	EVT_MENU(CONSOLE_ID_COPY,mxConsole::OnCopy)
END_EVENT_TABLE()
	
#define _buffer(i,j) buffer[(i)*buffer_w+(j)]
#define _CARET_TIME 500
#define _SIZE_TIME 100
#define _PROCESS_TIME 10

static wxColour colors[16][2] = {
	wxColour(0  ,0  ,0),	wxColour(127,127,127),
	wxColour(127,0  ,0),	wxColour(0,0,0),
	wxColour(0  ,127,0),	wxColour(0,0,0),
	wxColour(127,127,0),	wxColour(0,0,0),
	wxColour(0  ,0  ,255),	wxColour(0,0,0),
	wxColour(127,0  ,127),	wxColour(0,0,0),
	wxColour(0  ,127,127),	wxColour(0,0,0),
	wxColour(127,127,127),	wxColour(0,0,0),
	wxColour(0  ,  0,0),	wxColour(0,0,0),
	wxColour(255,  0,0),	wxColour(0,0,0),
	wxColour(0  ,255,0),	wxColour(0,0,0),
	wxColour(255,255,0),	wxColour(0,0,0),
	wxColour(127,127,255),	wxColour(0,0,0),
	wxColour(255,  0,255),	wxColour(0,0,0),
	wxColour(0  ,255,255),	wxColour(0,0,0),
	wxColour(255,255,255),	wxColour(0,0,0)
};


	
mxConsole::mxConsole(mxFrame *parent, wxScrollBar *scroll):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,0) {
	
	selection_start=-1; selecting=false;
	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_CTRL, 'v', CONSOLE_ID_PASTE);
	entries[1].Set(wxACCEL_CTRL, 'c', CONSOLE_ID_COPY);
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	
	for(int i=0;i<16;i++) 
		colors[i][1]=wxColour(colors[i][0].Red()/2,colors[i][0].Green()/2,colors[i][0].Blue()/2);
	this->scroll=scroll;
	if (scroll) scroll->SetScrollbar(0,1,1,1,false);
	this->parent=parent;
	margin=2;
	the_process=NULL;
	timer_size=new wxTimer(this,CONSOLE_ID_TIMER_SIZE);
	timer_caret=new wxTimer(this,CONSOLE_ID_TIMER_CARET);
	timer_process=new wxTimer(this,CONSOLE_ID_TIMER_PROCESS);
	buffer=NULL;
	Reset(true);
	SetFontSize(11);
	SetBackgroundStyle(wxBG_STYLE_CUSTOM); // para evitar el flickering en windows
}

void mxConsole::Reset (bool hard) {
	dimmed=0;
	events.clear();
	cur_event=-1;
	if (hard) {
		input_history.clear();
		current_input="";
	}
	want_input=wait_one_key=false;
	input_history_position=0;
	history=""; last_clear=0; 
	cur_x=cur_y=0;
	bg=0; cur_fg=15;
	blinking_caret_aux=false;
	ClearBuffer();
}

void mxConsole::OnPaint (wxPaintEvent & event) {
	if (!buffer) CalcResize();
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetBackground(colors[bg][0]);
	dc.SetTextBackground(colors[bg][0]);
	dc.Clear();
	if (selection_start!=-1) {
		wxColour sel_color(100,100,100);
		dc.SetPen(wxPen(sel_color));
		dc.SetBrush(wxBrush(sel_color));
		if (selection_start<selection_end)
			for(int i=selection_start;i<=selection_end;i++)
				dc.DrawRectangle(margin+(i%buffer_w)*char_w,margin+(i/buffer_w)*char_h,char_w,char_h);
		else
			for(int i=selection_end;i<=selection_start;i++)
				dc.DrawRectangle(margin+(i%buffer_w)*char_w,margin+(i/buffer_w)*char_h,char_w,char_h);
	}
	dc.SetFont(font);
	bool selected=false;
	for(int i=0;i<buffer_h;i++) { 
		int lj=0; wxString line;
		for(int j=0;j<buffer_w;j++) {
			line<<_buffer(i,j).the_char;
			if (j+1==buffer_w||_buffer(i,j).fg!=_buffer(i,j+1).fg) {
				dc.SetTextForeground(colors[_buffer(i,j).fg][dimmed]);
				dc.DrawText(line,margin+lj*char_w,margin+i*char_h);
				line.Clear();
				lj=j+1;
			}
		}
	}
	if (want_input && !wait_one_key && blinking_caret_aux) {
		dc.SetTextForeground(colors[cur_fg][dimmed]);
		dc.DrawText(wxString()<<"|",margin+cur_x*char_w-char_w/2,margin+cur_y*char_h);
	}
}

void mxConsole::OnSize (wxSizeEvent & event) {
	timer_size->Start(_SIZE_TIME,true);
}

void mxConsole::OnChar (wxKeyEvent & event) {
	if (event.GetKeyCode()==3) { parent->Close(); return; }
	if (the_process) {
		if (!want_input || cur_event!=-1) return;
		wxOutputStream *output=the_process->GetOutputStream();
		char c=char(event.GetKeyCode());
		if (wait_one_key) {
		 	wait_one_key=false;
			RecordInput(wxString()<<c);
			output->Write(&c,1);
		} else {
			if (c==27) { 
				for(int i=current_input.Len();i>0;i--) 
					Process("\b"); 
				current_input="";
				Refresh();
				return;
			} else if (c=='\r'||c=='\n') { 
				current_input<<"\n";
				output->Write(current_input.c_str(),current_input.Len());
				RecordInput(current_input);
				c='\n';
			} else if (c=='\b') {
				if (!current_input.Len()) return;
				current_input.RemoveLast();
			} else
				current_input<<c;
			Print(wxString()<<c,true/*,true*/);
			Refresh();
		}
	} else parent->Close();
}

void mxConsole::SetFontSize (int size) {
	font = wxFont(size,wxFONTFAMILY_TELETYPE,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	wxClientDC dc(this); dc.SetFont(font);
	char_h=dc.GetCharHeight(); char_w=dc.GetCharWidth();
}

void mxConsole::Print (wxString text, bool record/*, bool do_print*/) {
	if (record) history<<text;
	if (!buffer) return;
	int l=text.Len();
	for(int i=0;i<l;i++) {
		if (text[i]=='\n') {
			if (record) MarkEvent();
			cur_x=buffer_w;
		} else if (text[i]=='\r') {
			cur_x=0;
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

void mxConsole::ClearBuffer() {
	for(int i=0;i<buffer_h;i++) { 
		for(int j=0;j<buffer_w;j++) { 
			_buffer(i,j).the_char=' ';
		}
	}
}

void mxConsole::Process (wxString input, bool record/*, bool do_print*/) {
	int i,l;
	if (record) { // normal, viene un comando nuevo, se procesa todo
		i=0; l=input.Len();
	} else if (cur_event==-1) { // tiempo actual, se reprocesa desde el ultimo last_clear hasta el final
		i=last_clear; l=input.Len();
	} else { // viaje en el tiempo, se reprocesa hasta cierto evento
		i=events[cur_event].last_clear; l=events[cur_event].pos;
	}
	int i0=i;
	while (i<l) {
		if (input[i]=='\033' && input[i+1]=='[') {
			if (i-i0) Print(input.Mid(i0,i-i0),record/*,do_print*/);	
			if (input[i+2]=='z' && input[i+3]=='r') { // raise window
				GetParent()->Raise(); i+=3;
			} else if (input[i+2]=='z' && input[i+3]=='t') { // change window title
				static bool skip=true;
				if (skip) {
					skip=false;
					i--; break;
				}
				int j=i+4; while (j<input.Len() && input[j]!='\n') j++;
				parent->SetTitle(wxString("PSeInt - Ejecutando proceso ")+input.SubString(i+4,j-1));
				i=j;
			} else if (input[i+2]=='z' && input[i+3]=='k') { // getKey
				if (input_history_position>=int(input_history.size())) { 
					want_input=true; wait_one_key=true;
				} else {
					wxString aux=input_history[input_history_position++];
					wxOutputStream *output=the_process->GetOutputStream();
					output->Write(aux.c_str(),aux.Len());
				}
				i+=3;
			} else if (input[i+2]=='z' && input[i+3]=='l') { // getLine
				if (input_history_position>=int(input_history.size())) { 
					want_input=true; wait_one_key=false;
//					wxOutputStream *output=the_process->GetOutputStream();
//					output->Write(current_input.c_str(),current_input.Len());
					Print(current_input,true/*,true*/); // true,true, porque estas cosas solo llegan en vivo, no se guardan en el historial
				} else {
					wxString aux=input_history[input_history_position++];
					wxOutputStream *output=the_process->GetOutputStream();
					output->Write(aux.c_str(),aux.Len());
					Print(aux,true/*,true*/);  // true,true, porque estas cosas solo llegan en vivo, no se guardan en el historial
				}
				i+=3;
			} else if (input[i+2]=='2' && input[i+3]=='J') {
				ClearBuffer(); i+=3;
				if (record) { history<<"\033[2J"; last_clear=history.Len(); }
			} else if (input[i+2]=='?' && input[i+3]=='2' && input[i+4]=='5' && (input[i+5]=='l'||input[i+5]=='h')) {
				// hide/show caret, ignored
				i+=5;
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
	if (i-i0) Print(input.Mid(i0,i-i0),record/*,do_print*/);	
}

void mxConsole::OnTimerCaret (wxTimerEvent & event) {
	if (!want_input || wait_one_key) return; // no redibujar si no cambia nada
	blinking_caret_aux=!blinking_caret_aux;
	Refresh();
}

void mxConsole::OnTimerSize (wxTimerEvent & event) {
	CalcResize();
}

void mxConsole::CalcResize() {
	wxSize size = GetClientSize();
	buffer_w = (size.x - 2*margin) / char_w;
	buffer_h = (size.y - 2*margin) / char_h;
	if (buffer_w<1) buffer_w=1;
	if (buffer_h<1) buffer_h=1;
	delete [] buffer;
	buffer=new console_char[buffer_w*buffer_h];
	RebuildBuffer();
}

void mxConsole::Run (wxString command) {
	this->command=command;
	KillProcess();
	the_process=new wxProcess(this->GetEventHandler(),wxID_ANY);
	the_process->Redirect();
	the_process_pid=wxExecute(command,wxEXEC_ASYNC,the_process);
	if (the_process_pid>0) {
		timer_process->Start(_PROCESS_TIME,false);
		timer_caret->Start(_CARET_TIME,false);
	} else {
		delete the_process;
		the_process=NULL;
	}
}

void mxConsole::OnTimerProcess (wxTimerEvent & event) {
	GetProcessOutput();
}

void mxConsole::GetProcessOutput (bool refresh) {
	if (!the_process) return;
	wxTextInputStream input(*(the_process->GetInputStream()));
	wxString line;
	while (the_process->IsInputAvailable())
		line<<input.GetChar();
	if (line.Len()) { 
		if (cur_event!=-1) SetTime(int(events.size()));
		Process(line,true/*,cur_event==-1*/); 
		if (refresh) Refresh(); wxYield(); 
		// el wxYield es neceasario para procesos tipo "mientras verdero hacer escribir "Hola"; finmientras" porque sino no tiene oportunidad de redibujar por el intenso procesamiento
	}
}

void mxConsole::OnProcessTerminate( wxProcessEvent &event ) {
	if (event.GetPid()==the_process_pid) {
		GetProcessOutput(false);
		want_input=false;
		Refresh();
		the_process->Detach();
		the_process=NULL;
		timer_process->Stop();
		parent->OnProcessTerminated();
	}	
}

void mxConsole::GotoXY (int x, int y, bool record) {
	if (record) history<<"\033["<<y<<';'<<x<<'H';
	cur_x=x; if (x>=buffer_w) x=buffer_w;
	cur_y=y; if (y>=buffer_h) y=buffer_h;
}

void mxConsole::Reload (int to) {
	KillProcess();
	Reset(false);
	Run(command);
	parent->SetIsPresent(true);
}

void mxConsole::KillProcess ( ) {
	if (the_process) { 
		the_process->Kill(the_process_pid,wxSIGKILL);
		the_process->Detach();
	}
	the_process=NULL;
}

void mxConsole::RecordInput (wxString input) {
	if (input_history_position==int(input_history.size())) 
		input_history_position++;
	input_history.push_back(input);
	current_input.Clear();
	want_input=false;
}


void mxConsole::OnMouseWheel (wxMouseEvent & evt) {
	if (evt.ControlDown()) {
		int fsize=font.GetPointSize();
		if (evt.m_wheelRotation<0) {
			fsize++;
		} else if (fsize>4) fsize--;
		SetFontSize(fsize);
		timer_size->Start(_SIZE_TIME,true);
		Refresh();
	} else {
		int i=scroll->GetThumbPosition();
		if (evt.m_wheelRotation<0) {
			if (i<events.size()) {
				scroll->SetThumbPosition(++i);
				SetTime(scroll->GetThumbPosition());
				Refresh();
			}
		} else {
			if (i>0) {
				scroll->SetThumbPosition(--i);
				SetTime(scroll->GetThumbPosition());
				Refresh();
			}
		}
	}
	
}

void mxConsole::MarkEvent ( ) {
	if (!scroll) return;
	history_event h;
	h.pos=history.Len();
	h.last_clear=last_clear;
	h.input_count=input_history.size();
	if (input_history_position<h.input_count) 
		h.input_count=input_history_position;
	events.push_back(h);
//	int pos=cur_event==-1?h.pos:cur_event;
	scroll->SetScrollbar(h.pos,1,events.size()+1,events.size()/10,true);
}

void mxConsole::SetTime (int t) {
	if (t>=int(events.size())) {
		t=-1; parent->SetIsPresent(true);
	} else {
		parent->SetIsPresent(false);
	}
	cur_event=t;
	RebuildBuffer();
}

void mxConsole::RebuildBuffer ( ) {
	selection_start=-1;
	ClearBuffer(); cur_x=cur_y=0;
	Process(history, false);
	Refresh();
}

void mxConsole::PlayFromCurrentEvent ( ) {
	if (!events.size()) return;
	if (cur_event!=-1) {
		input_history.erase(input_history.begin()+events[cur_event].input_count,input_history.end());
	}
	Reload();
}

void mxConsole::Dimm ( ) {
	dimmed=1;
	Refresh();
}

inline int auxGetPosition(const wxMouseEvent &evt, int margin, int char_w, int char_h, int buffer_w, int buffer_h) {
	int x=(evt.GetX()-margin)/char_w, y=(evt.GetY()-margin)/char_h;
	int r=x+buffer_w*y;	
	if (r<0) r=0; else if (r>=buffer_w*buffer_h) r=buffer_w*buffer_h-1;
	return r;
}
void mxConsole::OnMouseLeftDown (wxMouseEvent & evt) {
	selecting=true; 
	selection_start=auxGetPosition(evt,margin,char_w,char_h,buffer_w,buffer_h);
	selection_end=selection_start;
	evt.Skip();
}

void mxConsole::OnMouseLeftUp (wxMouseEvent & evt) {
	selecting=false;
	evt.Skip();
}

void mxConsole::OnMouseMotion (wxMouseEvent & evt) {
	if (selecting) {
		cerr<<selection_start<<","<<selection_end<<"         \r";
		selection_end=auxGetPosition(evt,margin,char_w,char_h,buffer_w,buffer_h);
		Refresh();
	}
	evt.Skip();
}

inline wxString GetClipboardText() {
	if (!wxTheClipboard->Open()) return "";
	wxTextDataObject data;
	if (!wxTheClipboard->GetData(data)) {
		wxTheClipboard->Close();
		return "";
	}
	wxTheClipboard->Close();
	return data.GetText();
}

void mxConsole::OnMouseRightDown (wxMouseEvent & evt) {
	wxMenu menu;
	wxMenuItem *mcopy=menu.Append(CONSOLE_ID_COPY,"&Copiar");
	if (selection_start==-1) mcopy->Enable(false);
	wxMenuItem *mpaste=menu.Append(CONSOLE_ID_PASTE,"&Pegar");
	if (!GetClipboardText().Len()) mpaste->Enable(false);
	PopupMenu(&menu);
}


void mxConsole::OnPaste(wxCommandEvent &evt) {
	wxString str=GetClipboardText();
	str.Replace("\r","");
	wxKeyEvent k(wxEVT_CHAR); 
	for(int i=0;i<str.Len();i++) { 
		k.m_keyCode=str[i];
		OnChar(k);
		if (str[i]=='\n') {
			str=str.AfterFirst('\n');
			str<<"\n";
			while (str.Len()) {
				input_history.push_back((str.BeforeFirst('\n')+"\n").c_str());
				str=str.AfterFirst('\n');
			}
			return;
		}
	}
}

void mxConsole::OnCopy (wxCommandEvent & evt) {
	if (selection_end<selection_start) { int aux=selection_end; selection_end=selection_start; selection_start=aux; }
	int i0=selection_start%buffer_w, i1=selection_end%buffer_w;
	int j0=selection_start/buffer_w, j1=selection_end/buffer_w;
	wxString res;
	for(int j=j0;j<=j1;j++) {
		wxString aux;
		int k0=j==j0?i0:0, k1=j==j1?i1:buffer_w;
		for (int k=k0;k<k1;k++) {
			aux<<_buffer(j,k).the_char;
		}
		while (aux.Len() && aux.Last()==' ') aux.RemoveLast();
		if (j!=j1) aux<<"\n"; res<<aux; 
	}
	wxTheClipboard->Open();
	wxTheClipboard->SetData(new wxTextDataObject(res));
	wxTheClipboard->Close();
}

