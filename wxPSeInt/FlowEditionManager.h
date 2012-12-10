#ifndef FLOWEDITIONMANAGER_H
#define FLOWEDITIONMANAGER_H
#include <wx/string.h>

class wxSocketBase;
class wxSocketServer;
class wxSocketEvent;
class wxObject;

class FlowEditionManager {
private:
	wxSocketServer *server;
	wxSocketBase *socket;
	wxString sbuffer;
	int port;
public:
	FlowEditionManager();
	void SocketEvent(wxSocketEvent *event);
	void ProcData(wxSocketBase *s, wxString data);
	bool HasSocket(wxObject *s);
	int GetPort();
	~FlowEditionManager();
};

extern FlowEditionManager *flow_editor;

#endif

