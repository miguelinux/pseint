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
	bool subtitles_on;
	bool do_desktop_test;
	mxProcess *process;
	wxString current_proc_name;
	wxString sbuffer;
	mxSource *source;
	wxSocketBase *socket;
	friend class mxProcess;
	friend class mxDebugWindow;
	friend class mxDesktopTestGrid;
	bool should_pause;
	int current_line, current_inst; // linea e instruccion actual, como viene del interprete (base 1), para usar en el subtitulo
public:
	bool debugging, paused;
	DebugManager();
	void Start(mxProcess *process, mxSource *src);
	void Close(mxSource *src);
	void SetSocket(wxSocketBase *s);
	void ProcSocketData(wxString data);
	void ProcessSocketLost();
	void SetSpeed(int speed);
	int GetSpeed(int delay) const;
	bool Pause();
	void Step();
	void Stop();
	void SendEvaluation(wxString exp);
	void SetStepIn(bool b);
	void SetSubtitles(bool b);
};

extern DebugManager *debug;
#endif

