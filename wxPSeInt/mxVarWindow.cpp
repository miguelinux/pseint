#include "mxVarWindow.h"
#include <wx/listbox.h>
#include "mxMainWindow.h"
#include <wx/imaglist.h>
#include "ConfigManager.h"
#include "ids.h"
#include "mxUtils.h"
#include "DebugManager.h"
#include "mxDebugWindow.h"
#include <wx/choicdlg.h>
#include <wx/menu.h>
#include "string_conversions.h"
#include "mxDesktopTestGrid.h"

mxVarWindow *vars_window=NULL;

BEGIN_EVENT_TABLE(mxVarWindow,wxPanel) 
	EVT_TREE_SEL_CHANGED(wxID_ANY,mxVarWindow::OnTreeClick)
	EVT_TREE_ITEM_MIDDLE_CLICK(wxID_ANY,mxVarWindow::OnTreeClick2)
	EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY,mxVarWindow::OnTreeClick2)
	EVT_TREE_ITEM_GETTOOLTIP(wxID_ANY,mxVarWindow::OnTreeTooltip)
	EVT_MENU(mxID_VARS_DEFINIR,mxVarWindow::OnDefinir)
	EVT_MENU(mxID_VARS_ADD_TO_DESKTOP_TEST,mxVarWindow::OnAgregarAPruebaDeEscritorio)
END_EVENT_TABLE()
	
static wxString tooltip;

mxVarWindow::mxVarWindow(wxWindow *parent):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(150,100)) {
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	tree = new wxTreeCtrl(this,wxID_ANY,wxDefaultPosition,wxSize(150,50),wxTR_FULL_ROW_HIGHLIGHT|wxTR_NO_LINES|wxTR_HIDE_ROOT|wxTR_SINGLE|wxTR_NO_BUTTONS/*|wxTR_ROW_LINES*/);
	
	wxArrayString img_files(LV_MAX+2,""); // +2 por proc y sub
	img_files[LV_DEFINIDA                                   ] = img_files[0                                ] = "vt_desc.png";
	img_files[LV_DEFINIDA| LV_LOGICA                        ] = img_files[LV_LOGICA                        ] = "vt_log.png";
	img_files[LV_DEFINIDA|           LV_NUMERICA            ] = img_files[          LV_NUMERICA            ] = "vt_num.png";
	img_files[LV_DEFINIDA| LV_LOGICA|LV_NUMERICA            ] = img_files[LV_LOGICA|LV_NUMERICA            ] = "vt_log_num.png";
	img_files[LV_DEFINIDA|                       LV_CARACTER] = img_files[                      LV_CARACTER] = "vt_car.png";
	img_files[LV_DEFINIDA| LV_LOGICA            |LV_CARACTER] = img_files[LV_LOGICA|            LV_CARACTER] = "vt_log_car.png";
	img_files[LV_DEFINIDA|           LV_NUMERICA|LV_CARACTER] = img_files[          LV_NUMERICA|LV_CARACTER] = "vt_car_num.png";
	img_files[LV_DEFINIDA| LV_LOGICA|LV_NUMERICA|LV_CARACTER] = img_files[LV_LOGICA|LV_NUMERICA|LV_CARACTER] = "vt_desc.png";
	img_files[LV_MAX] = "vt_proc.png";
	img_files[LV_MAX+1] = "vt_sub.png";
	
	wxImageList* imglist = new wxImageList(16, 16,true,img_files.GetCount());
	for(unsigned int i=0;i<img_files.GetCount();i++)
		imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,img_files[i]),wxBITMAP_TYPE_PNG));
	tree->AssignImageList(imglist);
	tree_root=tree_current=tree->AddRoot("");
	tree->SetToolTip(tooltip=utils->FixTooltip(_Z("En esta sección se listan las variables que utiliza un algoritmo. El ícono a la izquierda del nombre indica los potenciales tipos de datos que determina el intérprete en caso de que el tipo de variable pueda deducirse antes de ejecutar el algoritmo. Puede seleccionar una para resaltarla en el pseudocódigo.")));
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
		if (p.IsOk()) last_parent=tree->GetItemText(p).BeforeFirst('('); 
		else last_parent="";
	} else last_sel="";
	tree->DeleteChildren(tree_root);
}

void mxVarWindow::AddProc(wxString vname, bool main_process) {
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

void mxVarWindow::AddVar(wxString vname, wxChar type) {
	int icon = type-LV_BASE_CHAR;
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
	if (!it.IsOk()) return;
	mxSource *src=main_window->GetCurrentSource();
	if (!src) return;
	int from=-1, to=-1;
	wxTreeItemId parent=tree->GetItemParent(it);
	if (parent.IsOk() && parent!=tree->GetRootItem()) {
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
		menu.Append(mxID_VARS_DEFINIR,_Z("Definir variable...")); // para mostrar en el dialogo
		menu.Append(mxID_VARS_ADD_TO_DESKTOP_TEST,_Z("Agregar la variable a la Prueba de Escritorio...")); // para mostrar en el dialogo
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


void mxVarWindow::OnDefinir (wxCommandEvent & evt) {
	wxTreeItemId it=GetSelection();
	if (!it.IsOk()) return; // ver que se seleccione un item del arbol
	mxSource *src=main_window->GetCurrentSource();
	if (!src) return; // ver que tengamos un fuente abierto
	wxTreeItemId parent=tree->GetItemParent(it);
	if (parent==tree_root) return; // ver que el item no sea el nombre de un proceso o subproceso
	range *r=(range*)tree->GetItemData(parent);
	if (!r) return; // ver que tengamos la informacion del scope del item
	src->DefineVar(r->from-1,tree->GetItemText(it),GetVarType(it));
}

int mxVarWindow::GetVarType (int &line, wxString var_name) {

	if (var_name.Contains("[")) var_name=var_name.BeforeFirst('[');
	var_name.MakeUpper();
	
	wxTreeItemIdValue cookie;
	wxTreeItemId it = tree->GetFirstChild(tree->GetRootItem(),cookie);
	while (it.IsOk()) {
		range *r=(range*)tree->GetItemData(it);
		if (line>=r->from && line<=r->to) {
			line = r->from;
			break;
		}
		it = tree->GetNextChild(it,cookie);
	}
	if (!it.IsOk()) return -1;
	it = tree->GetFirstChild(it,cookie);
	while (it.IsOk()) {
		wxString it_text = tree->GetItemText(it);
		if (it_text.Contains("[")) {
			if (it_text.BeforeFirst('[').Upper()==var_name) 
				return GetVarType(it);
		} else {
			if (it_text.Upper()==var_name) 
				return GetVarType(it);
		}
		it = tree->GetNextChild(it,cookie);
	}
	return -1;
	
}

int mxVarWindow::GetVarType (const wxTreeItemId & it) {
	return tree->GetItemImage(it);
}

void mxVarWindow::OnAgregarAPruebaDeEscritorio (wxCommandEvent & evt) {
	wxTreeItemId it=GetSelection();
	if (!it.IsOk()) return; // ver que se seleccione un item del arbol
	wxTreeItemId parent=tree->GetItemParent(it);
	if (parent==tree_root) return; // ver que el item no sea el nombre de un proceso o subproceso
	desktop_test->AddDesktopVar(tree->GetItemText(it));
}

