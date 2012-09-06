#ifndef MXVARWINDOW_H
#define MXVARWINDOW_H
#include <wx/panel.h>
#include <wx/treectrl.h>

class mxVarWindow : public wxPanel {
private:
	wxTreeCtrl *tree;
	wxTreeItemId tree_root,tree_current;
	wxString last_sel;
protected:
public:
	void BeginInput();
	void Add(wxString vname, char type);
	void EndInput();
	void OnTreeClick(wxTreeEvent &evt);
	wxTreeItemId GetSelection();
	mxVarWindow(wxWindow *parent);
	~mxVarWindow();
	DECLARE_EVENT_TABLE();
};

extern mxVarWindow *var_window;

#endif

