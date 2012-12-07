#ifndef MXFRAME_H
#define MXFRAME_H
#include <wx/frame.h>

class mxConsole;

class mxFrame : public wxFrame {
private:
	mxConsole *console;
protected:
public:
	mxFrame(wxString command);
	void OnButtonReload(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

