#ifndef MXCONSOLE_H
#define MXCONSOLE_H
#include <wx/panel.h>
#include <wx/timer.h>
#include <wx/process.h>
#include <wx/scrolbar.h>
#include <vector>
using namespace std;

class mxFrame;

class mxConsole : public wxPanel {
	
public:
	
	mxFrame *parent; ///< para avisarle de algunos eventos
	wxScrollBar *scroll; ///< barra para moverse en el tiempo (es de la ventana padre, pero la ajusta la consola)
	
	wxString command; ///< the last command runned in this console
	
	wxString history; ///< todo el texto procesado desde el inicio del proceso
	
	struct history_event {
		int last_clear; ///< valor de last_clear para el evento
		int input_count; ///< valor input_history.size()
		int pos; ///< posicion en history del evento
	};
	vector<history_event> events; ///< eventos a registrar en la scrollbar para viajes en el tiempo
	void MarkEvent();
	int cur_event; ///< ultimo evento que se esta mostrando actualmente, o -1 si va en tiempo real
	void SetTime(int t); ///< elige un evento para que la consola muestre solo hasta ese evento
	
	wxProcess *the_process; ///< running process, or NULL if process have ended/not started
	int the_process_pid; ///< [last] running process id
	void KillProcess(); ///< kills current process if any
	void OnProcessTerminate( wxProcessEvent &event ); ///< si el proceso hijo termina, actualiza el estado de la clase
	
	wxTimer *timer_process; ///< timer to extract and process output from current process' output stream
	void OnTimerProcess( wxTimerEvent &event ); ///< llama a GetProcessOutput regularmente, para actualizar la salida
	void GetProcessOutput(); ///< extract unprocessed text from output stream for current process
	
	wxString current_input; ///< the text entered for next/current input (since last time Enter key was presses, or since start), for storing in input_history on next Enter
	vector<wxString> input_history; ///< array with all inputs since process started, for reapeting input when reloading process
	int input_history_position; ///< auxiliar para saber en un Reload cuales ya se usaron y cuales no de las entradas en input_history
	bool wait_one_key; ///< indica que se espera una tecla cualquiera (que no se debe mostrar en pantalla)
	void OnChar( wxKeyEvent &event ); ///< process console input, and sends it to child process
	void RecordInput(wxString input); ///< guarda une lectura en el historial de entradas (input_history)
	
	int margin; ///< margin in pixels for text inside the console
	int char_w; ///< width in pixels for a char with current fontsize
	int char_h; ///< height in pixels for a char with current fontsize
	int bg; ///< index for current console background color (the same for the whole console)
	struct console_char { ///< data for each visible char in the console
		wxChar the_char; ///< the char to show
		int fg; ///< index for forground color
		console_char():the_char(' '),fg(0){}
	};
	console_char *buffer; ///< current content for the visible part of the console (size buffer_w*buffer_h), ordered by rows
	int buffer_w; ///< number of columns for current console size
	int buffer_h; ///< number of rows for current console size
	void OnPaint( wxPaintEvent &event ); ///< redraw the console based on buffers' content
	
	int cur_fg; ///< index for current text foreground color, will be used for printing int Print
	int cur_x; ///< current x caret position
	int cur_y; ///< current y caret position
	
	wxFont font; ///< current font
	void OnMouseWheel(wxMouseEvent &evt);
	
	wxTimer *timer_size; ///< timer to reprocess last text after a resize event (regenerates buffer content)
	int last_clear; ///< position in history where last cls even occured (to rewrite the text when resizing the console)
	void CalcResize(); ///< reprocess since last cls event to redraw after a resize event
	void OnTimerSize(wxTimerEvent &event); ///< calls resize when in timer_tize event
	void OnSize( wxSizeEvent &event ); ///< starts timer_size for regenerating buffer after a resize event
	void RebuildBuffer(); ///< recalcula buffer, se usa luego de un resize o de un settime
	
	
	bool caret_visible; ///< indicates if caret should be visible or not
	bool blinking_caret_aux; ///< aux flag for caret blinking animation
	wxTimer *timer_caret; ///< timer to blink caret if caret_visible
	void OnTimerCaret( wxTimerEvent &event ); ///< anima el parpadeo del cursor
		
	mxConsole(mxFrame *parent, wxScrollBar *scroll=NULL);
	
	void Run(wxString command);
	
	void Reload(int to=-1); ///< recarga el proceso, reejecutandolo completo y volviendo a darle las mismas entradas
	void PlayFromCurrentEvent(); ///< recarga el proceso, reejecutandolo hasta el evento actual, y siguiendo desde ahi en vivo (si historial)
	
	void SetFontSize(int size);
	void Process(wxString input, bool record=true/*, bool do_print=true*/);
	void Print(wxString text, bool record/*, bool do_print*/);
	void ClearBuffer();
	void GotoXY(int x, int y, bool record);
	void Reset(bool hard=false);
	
	DECLARE_EVENT_TABLE();
	
};

#endif

