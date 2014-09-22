#ifndef MXVARWINDOW_H
#define MXVARWINDOW_H
#include <wx/panel.h>
#include <wx/treectrl.h>
#include "../pseint/new_memoria_inter.h"

class mxVarWindow : public wxPanel {
private:
	struct range:public wxTreeItemData { long from, to; };
	wxTreeCtrl *tree;
	wxTreeItemId tree_root,tree_current;
	wxString last_sel, last_parent;
protected:
public:
	void BeginInput();
	void AddProc(wxString vname, bool main_process); ///< cuando el RTSyntaxManager recibe el nombre de un proceso o subproceso, lo registra en la lista con este método
	void AddVar(wxString vname, wxChar type); ///<  cuando el RTSyntaxManager recibe el nombre de una variable
	void EndInput();
	void OnTreeClick2(wxTreeEvent &evt);
	void OnTreeClick(wxTreeEvent &evt);
	void OnTreeTooltip(wxTreeEvent &evt);
	void OnDefinir(wxCommandEvent &evt);
	wxTreeItemId GetSelection();
	mxVarWindow(wxWindow *parent);
	~mxVarWindow();
	
	int GetVarType(int &line, wxString var_name);
	int GetVarType(const wxTreeItemId &it);
	
	DECLARE_EVENT_TABLE();
};

extern mxVarWindow *vars_window;

#endif

