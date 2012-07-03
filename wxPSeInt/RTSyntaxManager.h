#ifndef RTSYNTAXMANAGER_H
#define RTSYNTAXMANAGER_H
#include <wx/process.h>

class mxSource;
class RTSyntaxManager:public wxProcess {
	static RTSyntaxManager *the_one;
	static int lid;
	int id, pid;
	bool restart,running,processing;
	RTSyntaxManager();
public:
	void OnTerminate(int pid, int status);
	static void Start();
	static void Stop();
	static bool IsLoaded();
	static void Restart();
	static bool Process(mxSource *src);
};

#endif

