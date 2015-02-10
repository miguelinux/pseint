#include "mxSingleCaseWindow.h"
#include "Package.h"
#include <wx/sizer.h>
#include <wx/stc/stc.h>
#include <wx/stattext.h>
#include <wx/settings.h>

mxSingleCaseWindow::mxSingleCaseWindow (wxWindow *parent, const wxString &test_name, const TestCase & t, bool show_solution) : wxFrame(parent,wxID_ANY,test_name,wxDefaultPosition,wxDefaultSize,wxDEFAULT_FRAME_STYLE) {
	
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxSizer *input_sizer = new wxBoxSizer(wxVERTICAL);
	wxStyledTextCtrl *input = new wxStyledTextCtrl(this);
	input_sizer->Add(new wxStaticText(this,wxID_ANY,"Datos de entrada:"),wxSizerFlags().Border(wxALL,5));
	input_sizer->Add(input,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
	sizer->Add(input_sizer,wxSizerFlags().Proportion(1).Expand()/*.Border(wxALL,5)*/);
	input->SetText(t.input);
	
	if (show_solution) {
		wxSizer *solution_sizer = new wxBoxSizer(wxVERTICAL);
		wxStyledTextCtrl *solution = new wxStyledTextCtrl(this);
		solution_sizer->Add(new wxStaticText(this,wxID_ANY,"Solución correcta:"),wxSizerFlags().Border(wxALL,5));
		solution_sizer->Add(solution,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
		sizer->Add(solution_sizer,wxSizerFlags().Proportion(1).Expand()/*.Border(wxALL,5)*/);
		solution->SetText(t.solution);
	}
	
	wxSizer *output_sizer = new wxBoxSizer(wxVERTICAL);
	wxStyledTextCtrl *output = new wxStyledTextCtrl(this);
	output_sizer->Add(new wxStaticText(this,wxID_ANY,"Tu solución:"),wxSizerFlags().Border(wxALL,5));
	output_sizer->Add(output,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
	sizer->Add(output_sizer,wxSizerFlags().Proportion(1).Expand()/*.Border(wxALL,5)*/);
	output->SetText(t.output);
	
	SetSizerAndFit(sizer);
	CentreOnParent();
}

