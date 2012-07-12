#ifndef MXVARWINDOW_H
#define MXVARWINDOW_H
#include <wx/panel.h>
#include <wx/listctrl.h>

class mxVarWindow : public wxPanel {
private:
	wxListCtrl *list;
	wxString last_sel;
protected:
public:
	void BeginInput();
	void Add(wxString vname, char type);
	void EndInput();
	void OnListClick(wxListEvent &evt);
	int GetSelection();
	mxVarWindow(wxWindow *parent);
	~mxVarWindow();
	DECLARE_EVENT_TABLE();
};

extern mxVarWindow *var_window;

#endif

