#include "mxVarWindow.h"
#include <wx/listbox.h>
#include "mxMainWindow.h"
#include <wx/imaglist.h>
#include "ConfigManager.h"
#include "mxUtils.h"

mxVarWindow *var_window=NULL;

BEGIN_EVENT_TABLE(mxVarWindow,wxPanel) 
	EVT_TREE_SEL_CHANGED(wxID_ANY,mxVarWindow::OnTreeClick)
	EVT_TREE_ITEM_GETTOOLTIP(wxID_ANY,mxVarWindow::OnTreeTooltip)
END_EVENT_TABLE()
	
static wxString tooltip;

mxVarWindow::mxVarWindow(wxWindow *parent):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(150,100)) {
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	tree = new wxTreeCtrl(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTR_FULL_ROW_HIGHLIGHT|wxTR_NO_LINES|wxTR_HIDE_ROOT|wxTR_SINGLE|wxTR_NO_BUTTONS/*|wxTR_ROW_LINES*/);
	wxImageList* imglist = new wxImageList(16, 16,true,7);
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_desc.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_log.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_num.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_log_num.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_car.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_log_car.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_car_num.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_desc.png")),wxBITMAP_TYPE_PNG)); // solo para completar las posibles combinaciones de bits en los tipos de variables, esta no se debería usar nunca
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_proc.png")),wxBITMAP_TYPE_PNG)); 
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_sub.png")),wxBITMAP_TYPE_PNG)); 
	tree->AssignImageList(imglist);
	tree_root=tree_current=tree->AddRoot("Variables");
	tree->SetToolTip(tooltip=utils->FixTooltip("En esta sección se listan las variables que utiliza un algoritmo. El ícono a la izquierda del nombre indica los potenciales tipos de datos que determina el intérprete en caso de que el tipo de variable pueda deducirse antes de ejecutar el algoritmo. Puede seleccionar una para resaltarla en el pseudocódigo."));
	sizer->Add(tree,wxSizerFlags().Proportion(1).Expand());
	SetSizer(sizer);
}

mxVarWindow::~mxVarWindow() {
	
}

void mxVarWindow::BeginInput ( ) {
	wxTreeItemId s=GetSelection(); 
	if (s.IsOk()) last_sel=tree->GetItemText(s).BeforeFirst('['); else last_sel="";
	tree->DeleteChildren(tree_root);
}

void mxVarWindow::Add (wxString vname, bool main_process) {
	tree_current=tree->AppendItem(tree_root,vname,main_process?8:9);
	if (vname.BeforeFirst('[')==last_sel) tree->SelectItem(tree_current);
}

void mxVarWindow::Add (wxString vname, char type) {
	wxString stype; int icon=0;
	switch (type) {
	case '1': /*stype=" (log)"; */icon=1; break;
	case '2': /*stype=" (num)"; */icon=2; break;
	case '3': /*stype=" (log|num)"; */icon=3; break;
	case '4': /*stype=" (car)"; */icon=4; break;
	case '5': /*stype=" (log|car)"; */icon=5; break;
	case '6': /*stype=" (num|car)"; */icon=6; break;
	default: stype=" "; break;
	}
	vname.Replace("-1","??",true);
	wxTreeItemId id=tree->AppendItem(tree_current,vname/*+stype*/,icon);
	if (vname.BeforeFirst('[')==last_sel) tree->SelectItem(id);
}

void mxVarWindow::EndInput ( ) {
	tree->ExpandAll();
}

void mxVarWindow::OnTreeClick (wxTreeEvent & evt) {
	evt.Skip();	
	wxTreeItemId it=GetSelection();
	if (it==-1) return;
	mxSource *src=main_window->GetCurrentSource();
	if (!src) return;
	src->HighLight(tree->GetItemText(it).BeforeFirst('['));
}

void mxVarWindow::OnTreeTooltip(wxTreeEvent &evt) {
	evt.SetToolTip(tooltip); // for windows only
}

wxTreeItemId mxVarWindow::GetSelection ( ) {
	return tree->GetSelection();
}

