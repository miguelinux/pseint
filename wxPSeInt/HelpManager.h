#ifndef HELPMANAGER_H
#define HELPMANAGER_H
#include <wx/string.h>
#include <wx/hashset.h>
#include "mxUtils.h"

#define MAX_ERRORS_TXT 512

class HelpManager {
private:
	wxString errors[MAX_ERRORS_TXT];
	mxHashStringString commands;
public:
	HelpManager();
	void LoadErrors();
	void LoadCommands();
	wxString GetErrorText(wxString text, int num);
	wxString GetCommandText(wxString command);
	wxString GetQuickHelp(wxString key);
	~HelpManager();
};


extern HelpManager *help;

#endif

