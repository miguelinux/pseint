#ifndef MXFRAME_H
#define MXFRAME_H
#include <wx/frame.h>
#include <wx/socket.h>

class mxConsole;

class mxFrame : public wxFrame {
private:
	mxConsole *console;
	wxSocketClient *socket;
	int src_id;
	wxString buffer;
protected:
public:
	mxFrame(wxString command, int port, int id);
//	void OnButtonReload(wxCommandEvent &evt);
	void InitSocket(int port);
	void OnSocketEvent(wxSocketEvent &event);
	void ProcessCommand();
	void OnProcessTerminated();
	DECLARE_EVENT_TABLE();
};

#endif

