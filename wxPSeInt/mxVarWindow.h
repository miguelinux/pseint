#ifndef MXVARWINDOW_H
#define MXVARWINDOW_H
#include <wx/panel.h>

class wxListBox;

class mxVarWindow : public wxPanel {
private:
	wxListBox *list;
	wxString last_sel;
protected:
public:
	void BeginInput();
	void Add(wxString vname, char type);
	void EndInput();
	void OnListClick(wxCommandEvent &evt);
	mxVarWindow(wxWindow *parent);
	~mxVarWindow();
	DECLARE_EVENT_TABLE();
};

extern mxVarWindow *var_window;

#endif

