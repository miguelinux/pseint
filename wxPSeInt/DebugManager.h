#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H
#include <wx/string.h>

class mxSource;
class wxSocketBase;
class wxSocketServer;
class wxSocketEvent;
class mxProcess;
class mxEvaluateDialog;
class wxObject;

#define _calc_delay(speed) 25+(100-speed)*20

class DebugManager {
private:
	bool step_in;
	bool do_desktop_test;
	mxProcess *process;
	wxString current_proc_name;
	wxString sbuffer;
	mxSource *source;
	wxSocketServer *server;
	wxSocketBase *socket;
	friend class mxProcess;
	friend class mxDebugWindow;
	friend class mxDesktopTest;
	bool should_pause;
	int port;
public:
	bool debugging, paused;
	DebugManager();
	void Start(mxProcess *process, mxSource *src);
	void Close(mxSource *src);
	void ProcData(wxString data);
	void SocketEvent(wxSocketEvent *event);
	void SetSpeed(int speed);
	int GetSpeed(int delay) const;
	bool Pause();
	void Step();
	void Stop();
	void SetDoDesktopTest(bool val);
	void SendEvaluation(wxString exp);
	bool HasSocket(wxObject *s);
	void SetStepIn(bool b);
	int GetPort();
};

extern DebugManager *debug;
#endif

