#include "mxEvaluateDialog.h"
#include <wx/sizer.h>
#include <wx/combobox.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include "DebugManager.h"
#include <wx/msgdlg.h>
using namespace std;

BEGIN_EVENT_TABLE(mxEvaluateDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxEvaluateDialog::OnEvaluateButton)
	EVT_BUTTON(wxID_CANCEL,mxEvaluateDialog::OnCloseButton)
	EVT_CHAR_HOOK(mxEvaluateDialog::OnCharHook)
//	EVT_CLOSE(mxEvaluateDialog::OnClose)	
END_EVENT_TABLE()

	
mxEvaluateDialog::mxEvaluateDialog(wxWindow *parent) : wxDialog(parent,wxID_ANY,_T("Evaluar expresiones"),wxDefaultPosition,wxDefaultSize,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *comboSizer= new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	mySizer->Add(new wxStaticText(this, wxID_ANY, _T("Introduzca una expresion:"), wxDefaultPosition, wxDefaultSize, 0), wxSizerFlags().Border(wxALL,5));
	
	combo = new wxComboBox(this, wxID_ANY);
	comboSizer->Add(combo, wxSizerFlags().Border(wxALL,5).Proportion(1).Expand());
	wxButton *button_evaluate = new wxButton(this,wxID_OK, _T("&Evaluar"));
	comboSizer->Add( button_evaluate , wxSizerFlags().Border(wxALL,5));
	
	mySizer->Add(comboSizer, wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	
	mySizer->Add(new wxStaticText(this, wxID_ANY, _T("Valor actual:"), wxDefaultPosition, wxDefaultSize, 0), wxSizerFlags().Border(wxALL,5));
	result = new wxTextCtrl(this, wxID_ANY, _T("<<Presione Evaluar para ver aqui el resultado>>"), wxDefaultPosition, wxSize(300,70), wxTE_MULTILINE|wxTE_READONLY);
	mySizer->Add(result, wxSizerFlags().Border(wxALL,5).DoubleBorder(wxBOTTOM|wxLEFT).Proportion(1).Expand());
	
	wxButton *button_close = new wxButton(this,wxID_CANCEL, _T("&Cerrar"));
	bottomSizer->Add( button_close , wxSizerFlags().Border(wxALL,5));
	SetAffirmativeId(wxID_OK);
	SetEscapeId(wxID_CANCEL);
	mySizer->Add(bottomSizer,wxSizerFlags().Border(wxALL,5).Right());
	
	SetMinSize(GetSize());
	SetSizerAndFit(mySizer);
	SetFocusFromKbd();
	
}

mxEvaluateDialog::~mxEvaluateDialog() {
	
}

void mxEvaluateDialog::OnEvaluateButton(wxCommandEvent &evt) {
	if (!debug->debugging || !debug->paused) 
		result->SetValue(_T("<<La ejecucion paso a paso debe estar iniciada y pausada para poder evaluar expresiones>>"));
	else if (combo->GetValue().Len()) {
		result->SetValue(_T("<<evaluando...>>"));
		act_expression=combo->GetValue();
		if ( !last_expression.Len() || (act_expression.Len() && last_expression!=act_expression) ) {
			combo->Append(act_expression);
			
		}
		debug->SendEvaluation(act_expression);
		combo->SetSelection(0,act_expression.Len());
		last_expression=act_expression;
	} else
		result->SetValue(_T("<<Debe ingresar una expresion>>"));
}

void mxEvaluateDialog::OnCloseButton(wxCommandEvent &evt) {
	Hide();
}

void mxEvaluateDialog::SetEvaluationValue(wxString val) {
	result->SetValue(val);
}

//void mxEvaluateDialog::OnClose(wxCloseEvent &evt) {
//	Destroy();
//}

	
void mxEvaluateDialog::OnCharHook(wxKeyEvent &evt)	{
	if (evt.GetKeyCode()==WXK_RETURN || evt.GetKeyCode()==WXK_NUMPAD_ENTER) {
		wxCommandEvent evt;
		OnEvaluateButton(evt);
	} else evt.Skip();
}
	
void mxEvaluateDialog::Show() {
	result->SetValue(_T("<<Presione Evaluar para ver aqui el resultado>>"));
	wxDialog::Show();
}
