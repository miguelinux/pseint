#include "mxMainWindow.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/statline.h>
#include <wx/listbox.h>
#include <wx/app.h>
#include <wx/msgdlg.h>
#include "Package.h"
#include <wx/utils.h>
#include <wx/process.h>
#include <wx/txtstrm.h>
#include "mxSingleCaseWindow.h"

BEGIN_EVENT_TABLE(mxMainWindow,wxFrame)
	EVT_END_PROCESS(wxID_ANY,mxMainWindow::OnProcessTerminate)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxMainWindow::OnList)
END_EVENT_TABLE()

mxMainWindow::mxMainWindow ( ) 
	: wxFrame(NULL,wxID_ANY,"PSeInt - Ejercicio",wxDefaultPosition,wxDefaultSize,wxDEFAULT_FRAME_STYLE) 
{
	
	wxSizerFlags szfc = wxSizerFlags().Center().Border(wxALL,5);
	wxSizerFlags szf = wxSizerFlags().Proportion(0).Expand().Border(wxALL,5);
	wxSizerFlags szfe = szf; szfe.Proportion(1);
	
	sizer = new wxBoxSizer(wxVERTICAL);
	
	main_label = new wxStaticText(this,wxID_ANY,"Cargando ejercicio...");
	sizer->Add(main_label,szf);
	progress_bar = new wxGauge(this,wxID_ANY,10);
	sizer->Add(progress_bar,szf);
	
	sizer->Add(new wxStaticLine(this,wxID_ANY),szf);
	
	wxStaticText *results_title = new wxStaticText(this,wxID_ANY,"Resultados:");
	sizer->Add(results_title,szf);
	results_text = new wxStaticText(this,wxID_ANY,"0 correctos / 0 incorrectos");
	sizer->Add(results_text,szfc);
	results_bar = new wxGauge(this,wxID_ANY,10);
	sizer->Add(results_bar,szf);
	
	sizer->Add(new wxStaticLine(this,wxID_ANY),szf);
	
	wxStaticText *errors_title = new wxStaticText(this,wxID_ANY,"Casos fallidos:");
	sizer->Add(errors_title,szf);
	error_list = new wxListBox(this,wxID_ANY,wxDefaultPosition,wxSize(150,150));
	sizer->Add(error_list,szfe);
	wxStaticText *help = new wxStaticText(this,wxID_ANY,"Haga doble click sobre un caso de la\nlista para obtener más detalles.");
	sizer->Add(help,szfc);
	
	SetSizerAndFit(sizer);
}

void mxMainWindow::Start (const wxString &fname, const wxString &passkey, const wxString &cmdline) {
	Show();
	if (!pack.Load(fname)) {
		wxMessageBox("Error al cargar el ejercicio","PSeInt",wxOK|wxICON_ERROR,this);
		Destroy();
	} else {
		wxArrayString tests;
		int number = pack.GetNames(tests);
		progress_bar->SetRange(number);
		results_bar->SetRange(number);
		int results_ok=0, results_wrong=0;
		for(int i=0;i<number;i++) { 
			main_label->SetLabel(wxString("Probando ")+tests[i]);
			progress_bar->SetValue(i);
			Refresh(); wxYield();
			bool ok = RunTest(cmdline,pack.GetTest(tests[i]));
			if (ok) results_ok++; else results_wrong++;
			results_text->SetLabel( wxString()
				<<results_ok<<" correcto"<<(results_ok>1?"s":"") <<" / "
				<<results_wrong<<" incorrecto"<<(results_wrong>1?"s":"") );
			results_bar->SetValue(results_ok);
			if (!ok) error_list->Append(tests[i]);
		}
		progress_bar->SetValue(number);
		main_label->SetLabel("Finalizado");
	}
}

static bool process_finished;

bool mxMainWindow::RunTest(wxString command, TestCase &test) {
	
	wxProcess *the_process  = new wxProcess(this->GetEventHandler());
	the_process->Redirect();
	int pid = wxExecute(command,wxEXEC_ASYNC,the_process);
	if (!pid) return false;
	wxString &input=test.input, &output=test.output, &solution=test.solution;
	wxTextOutputStream in(*the_process->GetOutputStream());
	wxTextInputStream out(*the_process->GetInputStream());
	
	in<<input;
	process_finished=false;
	while(!process_finished) {
		while (the_process->IsInputAvailable()) 
			{ char aux; aux=out.GetChar(); if (aux) output<<aux; }
		wxYield();
	}
	while (the_process->IsInputAvailable())
		{ char aux; aux=out.GetChar(); if (aux) output<<aux; }
	delete the_process;
	
	output.Replace("\r","");
	solution.Replace("\r","");
	
	return output==solution;
	
}

void mxMainWindow::OnProcessTerminate (wxProcessEvent & event) {
	process_finished=true;
}

void mxMainWindow::OnList (wxCommandEvent & event) {
	wxString test_name = error_list->GetStringSelection();
	(new mxSingleCaseWindow(this,test_name,pack.GetTest(test_name)))->Show();
}

