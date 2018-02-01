#include <wx/clipbrd.h>
#include <wx/sizer.h>
#include <wx/stc/stc.h>
#include <wx/stattext.h>
#include <wx/settings.h>
#include <wx/statline.h>
#include <wx/app.h>
#include "Application.h"
#include "mxSingleCaseWindow.h"
#include "Package.h"
#include "../wxPSeInt/string_conversions.h"

BEGIN_EVENT_TABLE(mxSingleCaseWindow,wxFrame)
	EVT_LIST_ITEM_SELECTED(wxID_ANY,mxSingleCaseWindow::OnList)
	EVT_BUTTON(wxID_OK,mxSingleCaseWindow::OnCopy)
	EVT_BUTTON(wxID_CANCEL,mxSingleCaseWindow::OnCancel)
	EVT_CLOSE(mxSingleCaseWindow::OnClose)
END_EVENT_TABLE()

mxSingleCaseWindow::mxSingleCaseWindow (wxWindow *parent, bool only_first, bool show_solution) : wxFrame(parent,wxID_ANY,_Z("Resultados"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_FRAME_STYLE) {
	
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	
	input=output=solution=NULL;
	
	wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	
	if (show_solution) {
		wxSizer *solution_sizer = new wxBoxSizer(wxVERTICAL);
		solution = new wxStyledTextCtrl(this);
		solution_sizer->Add(new wxStaticText(this,wxID_ANY,_Z("Solución correcta:")),wxSizerFlags().Border(wxALL,5));
		solution_sizer->Add(solution,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
		sizer->Add(solution_sizer,wxSizerFlags().Proportion(1).Expand()/*.Border(wxALL,5)*/);
	} else {
		wxSizer *input_sizer = new wxBoxSizer(wxVERTICAL);
		input = new wxStyledTextCtrl(this);
		input_sizer->Add(new wxStaticText(this,wxID_ANY,_Z("Datos de entrada:")),wxSizerFlags().Border(wxALL,5));
		input_sizer->Add(input,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
		sizer->Add(input_sizer,wxSizerFlags().Proportion(1).Expand()/*.Border(wxALL,5)*/);
	}
	
	wxSizer *output_sizer = new wxBoxSizer(wxVERTICAL);
	output = new wxStyledTextCtrl(this);
	output_sizer->Add(new wxStaticText(this,wxID_ANY,_Z("Tu solución:")),wxSizerFlags().Border(wxALL,5));
	output_sizer->Add(output,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
	sizer->Add(output_sizer,wxSizerFlags().Proportion(1).Expand()/*.Border(wxALL,5)*/);
	
	
	list = new wxListCtrl(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLC_LIST|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
	list->InsertColumn(0,_Z("Casos"));
	
	wxSizer *but_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *but_copy = new wxButton(this,wxID_OK,_Z("Copiar Entrada"));
	wxButton *but_close  = new wxButton(this,wxID_CANCEL,_Z("Cerrar"));
	but_sizer->Add(but_copy,wxSizerFlags().Left());
	but_sizer->AddStretchSpacer();
	but_sizer->Add(but_close,wxSizerFlags().Left());
	
	if (!only_first) 
		main_sizer->Add(new wxStaticText(this,wxID_ANY,_Z("Seleccione un caso de prueba:")),wxSizerFlags().Border(wxALL,5));
	main_sizer->Add(list,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	if (only_first) list->Hide(); 
	else main_sizer->Add(new wxStaticLine(this,wxID_ANY),wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	main_sizer->Add(sizer,wxSizerFlags().Proportion(1).Expand());
	main_sizer->Add(but_sizer,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	SetSizerAndFit(main_sizer);
	CentreOnParent();
	
}

void mxSingleCaseWindow::AddCaso (wxString & nombre) {
	list->InsertItem(list->GetItemCount(),nombre);
}

void mxSingleCaseWindow::OnList (wxListEvent & event) {
	long item = -1;
	item = list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	current_test = list->GetItemText(item);
	TestCase &t = pack.GetTest(current_test);
	if (solution) solution->SetText(t.solution);
	if (output) output->SetText(t.output);
	if (input) input->SetText(t.input);
}

void mxSingleCaseWindow::OnClose (wxCloseEvent & event) {
	Destroy();
	GetParent()->Destroy();
}

void mxSingleCaseWindow::OnCancel (wxCommandEvent & event) {
	Close();
}

void mxSingleCaseWindow::Show ( ) {
	wxFrame::Show();
	if (list->GetItemCount()==1) { 
		list->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		wxListEvent event; OnList(event);
	}
}

void mxSingleCaseWindow::OnCopy (wxCommandEvent & event) {
	TestCase &t = pack.GetTest(current_test);
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(t.input));
		wxTheClipboard->Close();
	}
}

