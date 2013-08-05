#include "mxVarWindow.h"
#include <wx/listbox.h>
#include "mxMainWindow.h"
#include <wx/imaglist.h>
#include "ConfigManager.h"
#include "ids.h"
#include "mxUtils.h"
#include "DebugManager.h"
#include "mxDebugWindow.h"

mxVarWindow *vars_window=NULL;

BEGIN_EVENT_TABLE(mxVarWindow,wxPanel) 
	EVT_TREE_SEL_CHANGED(wxID_ANY,mxVarWindow::OnTreeClick)
	EVT_TREE_ITEM_MIDDLE_CLICK(wxID_ANY,mxVarWindow::OnTreeClick2)
	EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY,mxVarWindow::OnTreeClick2)
	EVT_TREE_ITEM_GETTOOLTIP(wxID_ANY,mxVarWindow::OnTreeTooltip)
	EVT_MENU(mxID_VARS_DEFINIR,mxVarWindow::OnDefinir)
END_EVENT_TABLE()
	
static wxString tooltip;

mxVarWindow::mxVarWindow(wxWindow *parent):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(150,100)) {
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	tree = new wxTreeCtrl(this,wxID_ANY,wxDefaultPosition,wxSize(150,50),wxTR_FULL_ROW_HIGHLIGHT|wxTR_NO_LINES|wxTR_HIDE_ROOT|wxTR_SINGLE|wxTR_NO_BUTTONS/*|wxTR_ROW_LINES*/);
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
	tree_root=tree_current=tree->AddRoot("");
	tree->SetToolTip(tooltip=utils->FixTooltip("En esta sección se listan las variables que utiliza un algoritmo. El ícono a la izquierda del nombre indica los potenciales tipos de datos que determina el intérprete en caso de que el tipo de variable pueda deducirse antes de ejecutar el algoritmo. Puede seleccionar una para resaltarla en el pseudocódigo."));
	sizer->Add(tree,wxSizerFlags().Proportion(1).Expand());
	SetSizer(sizer);
}

mxVarWindow::~mxVarWindow() {
	
}

void mxVarWindow::BeginInput ( ) {
	wxTreeItemId s=GetSelection(); 
	if (s.IsOk()) {
		last_sel=tree->GetItemText(s).BeforeFirst('['); 
		wxTreeItemId p=tree->GetItemParent(s);
		last_parent=tree->GetItemText(p).BeforeFirst('('); 
	} else last_sel="";
	tree->DeleteChildren(tree_root);
}

void mxVarWindow::Add (wxString vname, bool main_process) {
	range *r=NULL;
	if (vname.Contains(":")) {
		r=new range;
		vname.AfterFirst(':').BeforeFirst(':').ToLong(&r->from);
		vname.AfterLast(':').ToLong(&r->to);
		vname=vname.BeforeFirst(':');
	}
	tree_current=tree->AppendItem(tree_root,vname,main_process?8:9,-1,r);
	if (vname==last_sel && !last_parent.Len()) tree->SelectItem(tree_current);
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
	if (vname.BeforeFirst('[')==last_sel && last_parent==tree->GetItemText(tree_current)) tree->SelectItem(id);
}

void mxVarWindow::EndInput ( ) {
	tree->ExpandAll();
}

void mxVarWindow::OnTreeClick (wxTreeEvent & evt) {
	evt.StopPropagation(); // para que no le llegue a main_window y crea que es el arbol de errores
	evt.Skip(); // para que se seleccione en el arbol
	wxTreeItemId it=GetSelection();
	if (it==-1) return;
	mxSource *src=main_window->GetCurrentSource();
	if (!src) return;
	int from=-1, to=-1;
	wxTreeItemId parent=tree->GetItemParent(it);
	if (parent!=tree->GetRootItem()) {
		range *r=(range*)tree->GetItemData(parent);
		if (r) { from=r->from; to=r->to; }
	}
	src->HighLight(tree->GetItemText(it).BeforeFirst('['),from,to);
}

void mxVarWindow::OnTreeClick2 (wxTreeEvent & evt) {
	if (debug->debugging) { // cuando se depura, sirve para inspeccionar
		if (!debug->paused) return;
		wxTreeItemId it=evt.GetItem();
		if (it.IsOk() && tree->GetItemParent(it)!=tree_root) {
			tree->SelectItem(it);
			debug_panel->ShowInEvaluateDialog(tree->GetItemText(it).BeforeFirst('['));
		}
	} else { // cuando no se depura muestra un menu contextual
		if (tree->GetItemParent(evt.GetItem())==tree_root) return;
		wxMenu menu;
		menu.Append(mxID_VARS_DEFINIR,"Definir variable..."); // para mostrar en el dialogo
		PopupMenu(&menu);
	}
}

void mxVarWindow::OnTreeTooltip(wxTreeEvent &evt) {
	tree->SetToolTip(NULL); // si no pongo esto al mover el mouse por varios items y mostrar tooltip "sobre" (no se ve asi) tooltip despues no se cierra
	evt.SetToolTip(tooltip); // for windows only
}

wxTreeItemId mxVarWindow::GetSelection ( ) {
	return tree->GetSelection();
}

static bool IsEmpty(const wxString &s, bool comments) {
	int l=s.Len(), i=0;
	while (i<l && (s[i]==' '||s[i]=='\t'||s[i]=='\n'||s[i]=='\r')) {
		if (comments && s[i]=='/' && i+1<l && s[i+1]=='/') return true;
		i++;
	}
	return i==l;
}

static bool IsDimOrDef(const wxString &s) {
	int l=s.Len(), i=0;
	while (i<l && (s[i]==' '||s[i]=='\t')) i++;
	if (i+7<l && s.Mid(i,7).Upper()=="DEFINIR") return true;
	if (i+9<l && s.Mid(i,9).Upper()=="DIMENSION") return true;
	return false;
}

void mxVarWindow::OnDefinir (wxCommandEvent & evt) {
	wxTreeItemId it=GetSelection();
	if (!it.IsOk()) return; // ver que se seleccione un item del arbol
	mxSource *src=main_window->GetCurrentSource();
	if (!src) return; // ver que tengamos un fuente abierto
	wxTreeItemId parent=tree->GetItemParent(it);
	if (parent==tree_root) return; // ver que el item no sea el nombre de un proceso o subproceso
	range *r=(range*)tree->GetItemData(parent);
	if (!r) return; // ver que tengamos la informacion del scope del item
	int n=src->GetLineCount(), where=r->from-1, empty_lines=0;
	while (where+1<n && IsEmpty(src->GetLine(where+1),false)) // ver si hay lineas en blanco al principio del proceso
			{ where++; empty_lines++; } 
	bool add_line=empty_lines && where+1<n && !IsDimOrDef(src->GetLine(where+1));
	
	wxString var_name=tree->GetItemText(it); // obtener el nombre de la variable desde el arbol
	if (var_name.Contains("[")) var_name=var_name.BeforeFirst('['); // cortar las dimensiones si fuera un arreglo
	
	wxString var_type;
	int img=tree->GetItemImage(it); // ver el tipo segun el icono que tiene
	if (img==1) var_type="Logica";
	else if (img==2) var_type="Numerica";
	else if (img==4) var_type="Caracter";
	else { // si el tipo es ambiguo o desconocido, preguntar
		wxArrayString types;
		if (img==0 || (img&1)) types.Add("Logica");
		if (img==0 || (img&2)) types.Add("Numerica");
		if (img==0 || (img&4)) types.Add("Caracter");
		var_type=wxGetSingleChoice("Tipo de variable:",var_name,types);
		if (!var_type.Len()) return;
	}
	// agregar la definicion
	int x=src->GetLineEndPosition(where);
	src->SetTargetStart(x); src->SetTargetEnd(x);
	src->ReplaceTarget(wxString("\n")<<"\tDefinir "<<var_name<<" Como "<<var_type<<(config->lang.force_semicolon?";":"")<<(add_line?"\n\t":""));
}

