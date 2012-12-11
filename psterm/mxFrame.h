#ifndef MXFRAME_H
#define MXFRAME_H
#include <wx/frame.h>
#include <wx/socket.h>
#include <wx/scrolbar.h>
#include <wx/button.h>

class mxConsole;

class mxFrame : public wxFrame {
private:
	wxButton *play_from_here;
	wxScrollBar *scroll;
	mxConsole *console;
	wxSocketClient *socket;
	int src_id;
	wxString buffer;
	bool is_present; ///< para saber si mostrar o no el boton "continuar desde aqui"
protected:
public:
	mxFrame(wxString command, int port, int id);
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
	DECLARE_EVENT_TABLE();
};

#endif

