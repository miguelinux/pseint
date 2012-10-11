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
	void Add(wxString vname, bool main_process); ///< cuando el RTSyntaxManager recibe el nombre de un proceso o subproceso, lo registra en la lista con este método
	void Add(wxString vname, char type); //  cuando el RTSyntaxManager recibe el nombre de una variable
	void EndInput();
	void OnTreeClick2(wxTreeEvent &evt);
	void OnTreeClick(wxTreeEvent &evt);
	void OnTreeTooltip(wxTreeEvent &evt);
	wxTreeItemId GetSelection();
	mxVarWindow(wxWindow *parent);
	~mxVarWindow();
	DECLARE_EVENT_TABLE();
};

extern mxVarWindow *var_window;

#endif

