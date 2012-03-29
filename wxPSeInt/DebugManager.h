#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H
#include <wx/string.h>

class mxSource;
class wxSocketBase;
class wxSocketServer;
class wxSocketEvent;
class mxProcess;
class mxDesktopTest;
class mxEvaluateDialog;
class wxObject;

#define _calc_delay(speed) 25+(100-speed)*20

class DebugManager {
private:
	mxEvaluateDialog *evaluate_dialog;
	bool do_desktop_test;
	mxDesktopTest *desktop_test;
	mxProcess *process;
	wxString sbuffer;
	mxSource *source;
	wxSocketServer *server;
	wxSocketBase *socket;
	friend class mxProcess;
	friend class mxMainWindow;
	friend class mxDesktopTest;
	bool should_pause;
	int port;
public:
	bool debugging, paused;
	DebugManager(mxDesktopTest *dt, mxEvaluateDialog *ed);
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
	~DebugManager();
	int GetPort();
};

extern DebugManager *debug;
#endif

