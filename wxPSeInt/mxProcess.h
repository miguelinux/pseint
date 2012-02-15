#ifndef MXPROCESS_H
#define MXPROCESS_H
#include <wx/process.h>
#include "mxSource.h"


class wxOutputStream;
class wxInputStream;

class mxProcess : public wxProcess {
private:
	static int cont;
	int command_on_death_time;
	mxSource *source;
	bool was_readonly;
	wxString temp;
	int what;
	long pid;
	wxString file_code, file_log, file_other;
	wxOutputStream *output;
	wxInputStream *input;
	wxString filename;
	friend class mxMainWindow;
	friend class DebugManager;
public:
	mxProcess *next, *prev;
	mxProcess(mxSource *src,wxString filename);
	bool CheckSyntax(wxString file, wxString parsed=wxEmptyString);
	bool Run(wxString file, bool check_first=false);
	bool Debug(wxString file, bool check_first=false);
	bool Draw(wxString file, bool check_first=false);
	bool SaveDraw(wxString file, bool check_first=false);
	bool ExportCpp(wxString file, bool check_first=false);
	void ReadOut();
	wxString GetProfileArgs();
	~mxProcess();
	void OnTerminate(int pid, int status);
};

extern mxProcess *proc_list;
extern mxProcess *proc_for_killing;
#endif

