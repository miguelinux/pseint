#ifndef MXMAINWINDOW_H
#define MXMAINWINDOW_H
#include <wx/frame.h>
#include <wx/process.h>
#include "Package.h"
#include <wx/listctrl.h>

class wxStaticText;
class wxGauge;
class wxListBox;
class wxSizer;
class wxProcess;
class TestCase;

class mxMainWindow : public wxFrame {
	wxStaticText *main_label;
	wxGauge *progress_bar;
	wxStaticText *results_text;
	wxGauge *results_bar;
	wxListBox *error_list;
	wxSizer *sizer;
	Package pack;
	
public:
	mxMainWindow();
	void Start(const wxString &fname, const wxString &passkey, const wxString &cmdline);
	bool RunTest(wxString command, TestCase &test);
	
private:
	void OnProcessTerminate(wxProcessEvent &event);
	void OnList(wxCommandEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

