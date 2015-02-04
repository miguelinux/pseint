#ifndef MXTESTPANEL_H
#define MXTESTPANEL_H
#include <wx/panel.h>
#include "../pseval/Package.h"

class wxButton;
class wxStaticText;
class wxSizer;
class mxSource;

class mxTestPanel : public wxPanel {
private:
	Package pack;
	wxSizer *sizer;
	wxButton *eval_button;
	wxStaticText *label;
	wxString path,key;
	mxSource *src;
public:
	mxTestPanel(wxWindow *parent);
	bool Load(const wxString &path, const wxString &key, mxSource *src);
private:
	void OnRun(wxCommandEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

