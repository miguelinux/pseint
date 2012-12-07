#ifndef MXCONSOLE_H
#define MXCONSOLE_H
#include <wx/panel.h>
#include <wx/timer.h>
#include <wx/process.h>

class mxConsole : public wxPanel {
private:
	
	wxProcess *the_process;
	int the_process_pid;
	void GetProcessOutput();
	
	struct console_char {
		wxChar the_char;
		int fg;
		console_char():the_char(' '),fg(0){}
	} *buffer;
	int buffer_w, buffer_h;
	
	int bg, cur_fg;
	
	int cur_x,cur_y;
	
	int margin;
	
	wxString history;
	int last_clear;
	
	wxFont font;
	int char_w, char_h;
	wxColour fgColour;
	wxColour bgColour;
	wxTimer *timer_size;
	wxTimer *timer_caret;
	
	wxTimer *timer_process;
	
	bool caret_visible;
	bool blinking_caret_aux;
	
	// event handlers
	void OnPaint( wxPaintEvent &event );
	void OnSize( wxSizeEvent &event );
	void OnChar( wxKeyEvent &event );
	void OnTimerCaret( wxTimerEvent &event );
	void OnTimerSize( wxTimerEvent &event );
	void OnTimerProcess( wxTimerEvent &event );
	void OnProcessTerminate( wxProcessEvent &event );
	DECLARE_EVENT_TABLE();
	
	
public:
	mxConsole(wxWindow *parent);
	void Run(wxString command);
	void SetFontSize(int size);
	void Process(wxString input, bool record=true);
	void Print(wxString text, bool record=true);
	void ShowCaret(bool show, bool record=true);
	void Clear(bool record=true);
	void GotoXY(int x, int y, bool record=true);
	wxString GetInput();
	void WaitForKey();
	void Reset();
};

#endif

