#include "mxVarWindow.h"
#include <wx/listbox.h>
#include "mxMainWindow.h"
#include <wx/imaglist.h>
#include "ConfigManager.h"
#include "mxUtils.h"

mxVarWindow *var_window=NULL;

BEGIN_EVENT_TABLE(mxVarWindow,wxPanel) 
	EVT_LIST_ITEM_SELECTED(wxID_ANY,mxVarWindow::OnListClick)
END_EVENT_TABLE()

mxVarWindow::mxVarWindow(wxWindow *parent):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(150,100)) {
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	list =new wxListCtrl(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLC_SINGLE_SEL|wxLC_REPORT|wxLC_NO_HEADER|wxLC_HRULES);
	wxImageList* imglist = new wxImageList(16, 16,true,7);
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_desc.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_log.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_num.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_log_num.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_car.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_log_car.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(utils->JoinDirAndFile(config->images_path,_T("vt_car_num.png")),wxBITMAP_TYPE_PNG));
	list->AssignImageList(imglist,wxIMAGE_LIST_SMALL);
	list->InsertColumn(0,"Variables");
	list->SetColumnWidth(0,1000);
	list->SetToolTip(_T("En esta sección se listan las variables que utiliza un algoritmo. El ícono a la izquierda del nombre indica los potenciales tipos de datos que determina el intérprete en caso de que el tipo de variable pueda deducirse antes de ejecutar el algoritmo. Puede seleccionar una para resaltarla en el pseudocódigo."));
	sizer->Add(list,wxSizerFlags().Proportion(1).Expand());
	SetSizer(sizer);
}

mxVarWindow::~mxVarWindow() {
	
}

void mxVarWindow::BeginInput ( ) {
	int s=GetSelection(); 
	if (s==-1) last_sel=""; else last_sel=list->GetItemText(s).BeforeFirst('[');
	list->DeleteAllItems();
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
	list->InsertItem(list->GetItemCount(),vname/*+stype*/,icon);
	if (vname.BeforeFirst('[')==last_sel) 
		list->SetItemState(list->GetItemCount()-1,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
}

void mxVarWindow::EndInput ( ) {
	
}

void mxVarWindow::OnListClick (wxListEvent & evt) {
	evt.Skip();	
	int it=GetSelection();
	if (it==-1) return;
	mxSource *src=main_window->GetCurrentSource();
	if (!src) return;
	src->HighLight(list->GetItemText(it).BeforeFirst('['));
}

int mxVarWindow::GetSelection ( ) {
	if (list->GetSelectedItemCount()==0) return -1;
	return list->GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
}

