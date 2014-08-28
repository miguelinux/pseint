#ifndef MXFRAME_H
#define MXFRAME_H
#include <wx/frame.h>
#include <wx/socket.h>
#include <wx/scrolbar.h>
#include <wx/button.h>

class mxConsole;

struct win_props {
	bool always_on_top;
	bool set_left; long left;
	bool set_right; long right;
	bool set_top; long top;
	bool set_bottom; long bottom;
	long width, height;
	bool dark_theme;
	win_props() {
		dark_theme=false;
		always_on_top=false;
		set_left=set_right=set_top=set_bottom=false;
		width=550; height=350;
	}
};

class mxFrame : public wxFrame {
private:
	wxButton *play_from_here;
	wxScrollBar *scroll;
	mxConsole *console;
	wxSocketClient *socket;
	bool already_connected; ///< false si esta esperando establecer la conexion, para no enviar nada antes del hello
	wxString tosend_after_hello; ///< si quiere enviar algo por el socket antes del hello, en lugar de enviarlo lo guarda aca
	int src_id;
	wxString buffer;
	bool is_present; ///< para saber si mostrar o no el boton "continuar desde aqui"
	bool debug_mode; ///< si estamos utilizando la consola para ejecucion paso a paso, entonces no se puede volver en el tiempo ni se espera tecla adicional luego de que finaliza el proceso hijo
protected:
public:
	mxFrame(wxString command, int port, int id, bool debug, win_props props);
	void OnFocus(wxActivateEvent &evt);
	void OnButtonReload(wxCommandEvent &evt);
	void OnButtonPlay(wxCommandEvent &evt);
	void InitSocket(int port);
	void OnSocketEvent(wxSocketEvent &event);
	void OnScroll(wxScrollEvent &event);
	void OnClose(wxCloseEvent &event);
	void ProcessCommand();
	// eventos que recibe desde console
	void OnProcessTerminated();
	void SetIsPresent(bool is);
	void SendLocation(int line, int inst); ///< envia un mensaje al editor para indicar que debe marcar una instruccion
	DECLARE_EVENT_TABLE();
};

#endif

