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
	int last_id;
public:
	FlowEditionManager(int port);
	void SocketEvent(wxSocketEvent *event);
	void ProcData(wxSocketBase *s, wxString data);
	bool HasSocket(wxObject *s);
	int GetNextId();
	~FlowEditionManager();
};

extern FlowEditionManager *flow_editor;

#endif

