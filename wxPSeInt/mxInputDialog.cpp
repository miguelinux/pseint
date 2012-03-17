#include "mxInputDialog.h"
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include "mxBitmapButton.h"
#include "mxArt.h"

BEGIN_EVENT_TABLE(mxInputDialog,wxDialog)
	EVT_CHECKBOX(wxID_FIND,mxInputDialog::OnCheckUse)
	EVT_BUTTON(wxID_OK,mxInputDialog::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxInputDialog::OnButtonCancel)
END_EVENT_TABLE()

mxInputDialog::mxInputDialog(wxWindow *parent):wxDialog(parent,wxID_ANY,"Predefinir entrada",wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	wxStaticText *text=new wxStaticText(this,wxID_ANY,
		"Puede utilizar esta ventana para ingresar los valores que quiere\n"
		"que reciba su algoritmo cuando realice acciones de lectura al\n"
		"ejecutarse, para evitar así tener que ingresarlos en cada ejecución.\n"
		"Para ello, debe escribir en el cuadro de texto los valores que\n"
		"ingresaría al algoritmo por teclado, uno por linea. Es decir, cada\n"
		"linea se corresponderá con la lectura de una variable o de una\n"
		"posición de un arreglo."
		);
	sizer->Add(text,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	check_use=new wxCheckBox(this,wxID_FIND,"Utilizar una entrada predefinida para este algoritmo");
	sizer->Add(check_use,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	values = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxSize(200,200),wxTE_MULTILINE);
	sizer->Add(values,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
	check_partial=new wxCheckBox(this,wxID_ANY,"Si faltan valores, continuar leyendo desde teclado normalmente.");
	sizer->Add(check_partial,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	check_partial->SetValue(true);
	check_partial->Enable(false);
	values->Enable(false);
	
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, _T("Aceptar"));
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, _T("Cancelar"));
	button_sizer->Add(cancel_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(ok_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	sizer->Add(button_sizer,wxSizerFlags().Right());
	
	SetSizerAndFit(sizer);
}


void mxInputDialog::OnCheckUse (wxCommandEvent & evt) {
	evt.Skip();
	check_partial->Enable(check_use->GetValue());
	values->Enable(check_use->GetValue());
}

void mxInputDialog::OnButtonOk (wxCommandEvent & evt) {
	Hide();
}

void mxInputDialog::OnButtonCancel (wxCommandEvent & evt) {
	values->SetValue(old_values);
	Hide();
}

void mxInputDialog::Show() {
	old_values=values->GetValue();
	if (check_use->GetValue())
		values->SetFocus();
	else
		check_use->SetFocus();
	ShowModal();
}

bool mxInputDialog::HasInput ( ) {
	return check_use->GetValue();
}

bool mxInputDialog::IsPartial ( ) {
	return check_partial->GetValue();
}

wxString mxInputDialog::GetInput ( ) {
	return values->GetValue();
}

