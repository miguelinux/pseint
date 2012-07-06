#include "mxVarWindow.h"
#include <wx/listbox.h>
#include "mxMainWindow.h"

mxVarWindow *var_window=NULL;

BEGIN_EVENT_TABLE(mxVarWindow,wxPanel) 
	EVT_LISTBOX(wxID_ANY,mxVarWindow::OnListClick)
END_EVENT_TABLE()

mxVarWindow::mxVarWindow(wxWindow *parent):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(100,100)) {
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	list=new wxListBox(this,wxID_ANY,wxDefaultPosition,wxDefaultSize);
	list->SetToolTip(_T("En esta sección se listan las variables que utiliza un algoritmo. Entre paréntesis aparecen los potenciales tipos de datos que determina el intérprete en caso de que el tipo de variable pueda deducirse antes de ejecutar el algoritmo. Puede seleccionar una para resaltarla en el pseudocódigo."));
	sizer->Add(list,wxSizerFlags().Proportion(1).Expand());
	SetSizer(sizer);
}

mxVarWindow::~mxVarWindow() {
	
}

void mxVarWindow::BeginInput ( ) {
	int s=list->GetSelection(), n=list->GetCount();
	if (s<0||s>=n) last_sel=""; else last_sel=list->GetString(s).BeforeFirst(' ');
	list->Clear();
}

void mxVarWindow::Add (wxString vname, char type) {
	wxString stype;
	switch (type) {
	case '1': stype=" (log)"; break;
	case '2': stype=" (num)"; break;
	case '3': stype=" (log|num)"; break;
	case '4': stype=" (car)"; break;
	case '5': stype=" (log|car)"; break;
	case '6': stype=" (num|car)"; break;
	default: stype=" "; break;
	}
	list->Append(vname+stype);
	if (vname==last_sel) list->SetSelection(list->GetCount()-1);
}

void mxVarWindow::EndInput ( ) {
	
}

void mxVarWindow::OnListClick (wxCommandEvent & evt) {
	int s=list->GetSelection(), n=list->GetCount();
	if (s<0||s>=n) return;
	mxSource *src=main_window->GetCurrentSource();
	if (!src) return;
	src->HighLight(list->GetString(s).BeforeFirst(' '));
}

