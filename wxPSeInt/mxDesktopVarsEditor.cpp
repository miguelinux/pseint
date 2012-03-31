#include "mxDesktopVarsEditor.h"
#include "ids.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxDesktopVarsEditor, wxDialog)
	
	EVT_BUTTON(wxID_OK,mxDesktopVarsEditor::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxDesktopVarsEditor::OnCancelButton)
	
	EVT_BUTTON(mxID_DESKTOP_LIST_ADD,mxDesktopVarsEditor::OnAdd)
	EVT_BUTTON(mxID_DESKTOP_LIST_EDIT,mxDesktopVarsEditor::OnEdit)
	EVT_BUTTON(mxID_DESKTOP_LIST_DELETE,mxDesktopVarsEditor::OnDelete)
	EVT_BUTTON(mxID_DESKTOP_LIST_DELETE_ALL,mxDesktopVarsEditor::OnDeleteAll)
	EVT_BUTTON(mxID_DESKTOP_LIST_UP,mxDesktopVarsEditor::OnUp)
	EVT_BUTTON(mxID_DESKTOP_LIST_DOWN,mxDesktopVarsEditor::OnDown)
	EVT_BUTTON(mxID_HELP_BUTTON,mxDesktopVarsEditor::OnHelpButton)
	
	EVT_CLOSE(mxDesktopVarsEditor::OnClose)
	
END_EVENT_TABLE()

mxDesktopVarsEditor::mxDesktopVarsEditor(wxWindow *parent) : wxDialog(parent, wxID_ANY, _T("Prueba de Escritorio"), wxDefaultPosition, wxSize(400,100) ,wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	wxBoxSizer *mid_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *right_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	
	list = new wxListBox(this,wxID_ANY,wxDefaultPosition,wxSize(300,200));
	mid_sizer->Add(list,wxSizerFlags().Border(wxALL,5).Proportion(1).Expand());
	wxButton *button_up = new wxButton(this,mxID_DESKTOP_LIST_UP,_T("Mover Arriba"));
	wxButton *button_down = new wxButton(this,mxID_DESKTOP_LIST_DOWN,_T("Mover Abajo"));
	wxButton *button_edit = new wxButton(this,mxID_DESKTOP_LIST_EDIT,_T("Editar..."));
	wxButton *button_add = new wxButton(this,mxID_DESKTOP_LIST_ADD,_T("Agregar..."));
	wxButton *button_delete = new wxButton(this,mxID_DESKTOP_LIST_DELETE,_T("Quitar"));
	wxButton *button_delete_all = new wxButton(this,mxID_DESKTOP_LIST_DELETE_ALL,_T("Borrar Todo"));
	
	wxButton *button_ok = new wxButton(this,wxID_OK,_T("Aceptar"));
	wxButton *button_cancel = new wxButton(this,wxID_CANCEL,_T("Cancelar"));
	right_sizer->Add(button_add,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	right_sizer->Add(button_edit,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	right_sizer->Add(button_delete,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	right_sizer->Add(button_delete_all,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand().DoubleBorder(wxTOP|wxBOTTOM));
	right_sizer->Add(button_up,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	right_sizer->Add(button_down,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	mid_sizer->Add(right_sizer,wxSizerFlags().Proportion(0).Expand().Right());
	bottom_sizer->Add(button_cancel,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	bottom_sizer->Add(button_ok,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	
	main_sizer->Add(new wxStaticText(this,wxID_ANY,_T("Expresiones a evaluar:")),wxSizerFlags().Left().Border(wxALL,5));
	main_sizer->Add(mid_sizer,wxSizerFlags().Proportion(1).Expand());
	check_doit = new wxCheckBox(this,wxID_ANY,_T("Realizar seguimiento."));
	check_doit->SetValue(true);
	main_sizer->Add(check_doit,wxSizerFlags().Left().Border(wxALL,5).DoubleBorder(wxTOP|wxBOTTOM));
	main_sizer->Add(bottom_sizer,wxSizerFlags().Center());
	
	list->InsertItems(main_window->GetDesktopVars(),0);
	list->SetFocus();
	SetSizerAndFit(main_sizer);
	CenterOnParent();
	ShowModal();
	
}


mxDesktopVarsEditor::~mxDesktopVarsEditor() {
	
}

void mxDesktopVarsEditor::OnOkButton(wxCommandEvent &evt) {
	wxArrayString vars;
	for (unsigned int i=0;i<list->GetCount();i++)
		vars.Add(list->GetString(i));
	main_window->SetDesktopVars(check_doit->GetValue(),vars);
	Close();
}

void mxDesktopVarsEditor::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxDesktopVarsEditor::OnHelpButton(wxCommandEvent &evt) {
	
}

void mxDesktopVarsEditor::OnDeleteAll(wxCommandEvent &evt) {
	list->Clear();
}

void mxDesktopVarsEditor::OnDelete(wxCommandEvent &evt) {
	wxArrayInt ai;
	list->GetSelections(ai);
	if (ai.GetCount())
		for (int i=(int)ai.GetCount()-1;i>=0;i--)
			list->Delete(ai[i]);
}

void mxDesktopVarsEditor::OnAdd(wxCommandEvent &evt) {
	wxString var = wxGetTextFromUser(_T("Expresion"),_T("Ingrese la expresion:"),list->GetString(list->GetSelection()),this);
	if (var.Len())
		list->Append(var);
}

void mxDesktopVarsEditor::OnEdit(wxCommandEvent &evt) {
	if (list->GetSelection()!=wxNOT_FOUND) {
		wxString var = wxGetTextFromUser(_T("Expresion"),_T("Ingrese la expresion:"),list->GetString(list->GetSelection()),this);
		if (var.Len())
			list->SetString(list->GetSelection(),var);
	}
}

void mxDesktopVarsEditor::OnUp(wxCommandEvent &evt) {
	int n=list->GetSelection();
	if (n!=wxNOT_FOUND && n>0) {
		wxString aux = list->GetString(n);		
		list->SetString(n,list->GetString(n-1));
		list->SetString(n-1,aux);
		list->SetSelection(n-1);
	}
}

void mxDesktopVarsEditor::OnDown(wxCommandEvent &evt) {
	int n=list->GetSelection();
	if (n!=wxNOT_FOUND && n+1<(int)list->GetCount()) {
		wxString aux = list->GetString(n);		
		list->SetString(n,list->GetString(n+1));
		list->SetString(n+1,aux);
		list->SetSelection(n+1);
	}
}

void mxDesktopVarsEditor::OnClose(wxCloseEvent &evt) {
	Destroy();
}
