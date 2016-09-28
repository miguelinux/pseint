#include "mxCreatorWindow.h"
#include "Package.h"
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>
#include "mxMainWindow.h"
#include "Application.h"
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/settings.h>

enum { mxSTART=wxID_HIGHEST, mxID_INPUT_PATH_TEXT, mxID_INPUT_RELOAD, mxID_INPUT_PATH_BUTTON, mxID_OUTPUT_PATH_BUTTON, mxID_SHOW_ERRORS, mxID_CREATE_BUTTON, mxID_GENERATE_SOLUTIONS, mxID_USE_PASSWORD };

BEGIN_EVENT_TABLE(mxCreatorWindow,wxFrame)
	EVT_BUTTON(mxID_INPUT_PATH_BUTTON,mxCreatorWindow::OnInputPath)
//	EVT_BUTTON(mxID_OUTPUT_PATH_BUTTON,mxCreatorWindow::OnOutputPath)
	EVT_BUTTON(mxID_CREATE_BUTTON,mxCreatorWindow::OnCreate)
	EVT_BUTTON(mxID_GENERATE_SOLUTIONS,mxCreatorWindow::OnGenerateSolutions)
	EVT_TEXT(mxID_INPUT_PATH_TEXT,mxCreatorWindow::OnInputChange)
	EVT_BUTTON(mxID_INPUT_RELOAD,mxCreatorWindow::OnInputReload)
	EVT_TIMER(wxID_ANY,mxCreatorWindow::OnTimer)
	EVT_CHECKBOX(mxID_SHOW_ERRORS,mxCreatorWindow::OnShowErrors)
	EVT_CHECKBOX(mxID_USE_PASSWORD,mxCreatorWindow::OnCheckPass)
END_EVENT_TABLE()

mxCreatorWindow::mxCreatorWindow(const wxString &cmdline)
	: wxFrame(NULL,wxID_ANY,"PSeInt - Ejercicio",wxDefaultPosition,wxDefaultSize,wxDEFAULT_FRAME_STYLE)
	,timer(GetEventHandler())
{
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	pseint_command=cmdline;
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	
	main_sizer->Add(new wxStaticText(this,wxID_ANY,"Directorio de entrada:"),wxSizerFlags().Border(wxTOP|wxLEFT|wxRIGHT,5));
	wxBoxSizer *input_sizer = new wxBoxSizer(wxHORIZONTAL);
	input_path = new wxTextCtrl(this,mxID_INPUT_PATH_TEXT,"");
	input_sizer->Add(input_path,wxSizerFlags().Proportion(1).Center().Border(wxLEFT|wxTOP|wxBOTTOM,5));
	wxButton *input_button = new wxButton(this,mxID_INPUT_PATH_BUTTON," ... ",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
	input_sizer->Add(input_button,wxSizerFlags().Border(wxRIGHT|wxTOP|wxBOTTOM,5));
	wxButton *reload_button = new wxButton(this,mxID_INPUT_RELOAD," R ",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
	reload_button->SetToolTip("Recargar información desde el directorio de entrada actual");
	input_sizer->Add(reload_button,wxSizerFlags().Border(wxRIGHT|wxTOP|wxBOTTOM,5));
	main_sizer->Add(input_sizer,wxSizerFlags().Proportion(0).Expand());
	
	static1 = new wxStaticText(this,wxID_ANY,"Mensaje en caso de éxito:");
	main_sizer->Add(static1,wxSizerFlags().Border(wxTOP|wxLEFT|wxRIGHT,5));
	msg_ok = new wxTextCtrl(this,wxID_ANY,"");
	main_sizer->Add(msg_ok,wxSizerFlags().Expand().Border(wxALL,5));
	static2 = new wxStaticText(this,wxID_ANY,"Mensaje en caso de error:");
	main_sizer->Add(static2,wxSizerFlags().Border(wxTOP|wxLEFT|wxRIGHT,5));
	msg_bad = new wxTextCtrl(this,wxID_ANY,"");
	main_sizer->Add(msg_bad,wxSizerFlags().Expand().Border(wxALL,5));
	
	show_errors = new wxCheckBox(this,mxID_SHOW_ERRORS,"Mostrar casos fallidos");
	main_sizer->Add(show_errors,wxSizerFlags().Border(wxALL,5));
	stop_on_first_error = new wxCheckBox(this,wxID_ANY,"Solo el primero que falle");
	main_sizer->Add(stop_on_first_error,wxSizerFlags().Border(wxALL,5));
	show_solutions = new wxCheckBox(this,wxID_ANY,"Mostrar soluciones correctas");
	main_sizer->Add(show_solutions,wxSizerFlags().Border(wxALL,5));
	
	wxBoxSizer *tests_sizer = new wxBoxSizer(wxHORIZONTAL);
	num_tests = new wxTextCtrl(this,wxID_ANY,"");
	static3 = new wxStaticText(this,wxID_ANY,"Casos de prueba: ");
	tests_sizer->Add(static3,wxSizerFlags().Center().Border(wxLEFT,5));
	tests_sizer->Add(num_tests,wxSizerFlags().Proportion(1).Border(wxRIGHT|wxTOP|wxBOTTOM,5));
	main_sizer->Add(tests_sizer,wxSizerFlags().Proportion(0).Expand());
	
	shuffle_tests = new wxCheckBox(this,wxID_ANY,"Probar casos en orden aleatorio");
	main_sizer->Add(shuffle_tests,wxSizerFlags().Border(wxALL,5));
	
	base_psc = new wxCheckBox(this,wxID_ANY,"Incluir pseudocódigo base");
	main_sizer->Add(base_psc,wxSizerFlags().Border(wxALL,5));
	
	help_html = new wxCheckBox(this,wxID_ANY,"Incluir ayuda/enunciado html");
	main_sizer->Add(help_html,wxSizerFlags().Border(wxALL,5));
	
	solutions_button = new wxButton(this,mxID_GENERATE_SOLUTIONS," Generar soluciones... ");
	main_sizer->Add(solutions_button,wxSizerFlags().Border(wxALL,5));
	
	chk_pass = new wxCheckBox(this,mxID_USE_PASSWORD,"Contraseña: ");
	
	wxBoxSizer *pass1_sizer = new wxBoxSizer(wxHORIZONTAL);
	password1 = new wxTextCtrl(this,wxID_ANY,"");
	pass1_sizer->Add(chk_pass,wxSizerFlags().Center().Border(wxLEFT,5));
	pass1_sizer->Add(password1,wxSizerFlags().Proportion(1).Border(wxRIGHT|wxTOP|wxBOTTOM,5));
	main_sizer->Add(pass1_sizer,wxSizerFlags().Proportion(0).Expand());
	
	chk_new_cypher = new wxCheckBox(this,mxID_SHOW_ERRORS,"Cifrado mejorado (requiere PSeInt v20160401 o superior)");
	main_sizer->Add(chk_new_cypher,wxSizerFlags().Border(wxBOTTOM|wxLEFT|wxRIGHT,5));
	
//	main_sizer->Add(new wxStaticText(this,wxID_ANY,"Archivo de salida:"),wxSizerFlags().Border(wxTOP|wxLEFT|wxRIGHT,5));
//	wxBoxSizer *output_sizer = new wxBoxSizer(wxHORIZONTAL);
//	output_path = new wxTextCtrl(this,wxID_ANY,"");
//	output_sizer->Add(output_path,wxSizerFlags().Proportion(1).Center().Border(wxLEFT|wxTOP|wxBOTTOM,5));
//	wxButton *output_button = new wxButton(this,mxID_OUTPUT_PATH_BUTTON," ... ",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
//	output_sizer->Add(output_button,wxSizerFlags().Border(wxRIGHT|wxTOP|wxBOTTOM,5));
//	main_sizer->Add(output_sizer,wxSizerFlags().Proportion(0).Expand());
	
	ok_button = new wxButton(this,mxID_CREATE_BUTTON," Generar... ");
	main_sizer->Add(ok_button,wxSizerFlags().Right().Border(wxALL,5));
	EnableDisable();
	
	SetSizerAndFit(main_sizer);
	Show(); input_path->SetFocus();
}

void mxCreatorWindow::OnTimer (wxTimerEvent & event) {
	LoadDirectory();
}

void mxCreatorWindow::OnCreate (wxCommandEvent & event) {
	
	if (chk_pass->GetValue() && password1->GetValue().IsEmpty()) {
		wxMessageBox("Debe completar la constraseña","Error",wxOK|wxICON_ERROR,this);
		return;
	}
	
	wxArrayString tests;
	int tests_count = pack.GetNames(tests);
	if (tests_count==0) {
		wxMessageBox("No hay casos de prueba","Error",wxOK|wxICON_ERROR,this);
		return;
	}
	
	wxString all_output;
	for(unsigned int i=0;i<tests.GetCount();i++) { 
		all_output << pack.GetTest(tests[i]).solution;
	}
	
	if (all_output.IsEmpty()) {
		if (wxNO==wxMessageBox("Las soluciones están vacías. ¿Continuar igualmente?","Advertencia",wxYES_NO|wxICON_ERROR,this))
			return;
	}
	
	pack.SetConfigStr  ( "mensaje exito" , msg_ok->GetValue() );
	pack.SetConfigStr  ( "mensaje error" , msg_bad->GetValue() );
	pack.SetConfigBool ( "mostrar soluciones" , show_solutions->GetValue() );
	pack.SetConfigStr  ( "mostrar casos fallidos" , show_errors->GetValue()
												?(stop_on_first_error->GetValue()?"primero":"todos")
												:"ninguno" );
	pack.SetConfigBool ( "mezclar casos", shuffle_tests->GetValue() );
	pack.SetConfigInt  ( "version requerida" , PACKAGE_VERSION );
	
	static wxString fdir;
	static wxString fname;
	wxString fext = chk_pass->GetValue()?"psx":"psz";
	wxFileDialog dlg (this, "Guardar",fdir,fname+"."+fext,"Ejercicio|*.psz;*.PSZ;*.psx;*.PSX", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK) {
		wxFileName file(dlg.GetPath());
		fdir=file.GetPath(); fname=file.GetName();
		wxString ext = file.GetExt().Lower();
		if (ext=="") { file.SetExt(chk_pass->GetValue()?"psx":"psz"); }
		else if ( (ext=="psx") != chk_pass->GetValue() ) {
			wxMessageBox("Nombre de archivo incorrecto. La extensión debe ser \".psz\" para\n"
						"ejercicios sin contraseña, y \".psx\" para ejercicios con contraseña.",
						"Error",wxOK|wxICON_ERROR,this);
			return;
		}
		if (!pack.Save(file.GetFullPath(),chk_pass->GetValue()?password1->GetValue():"",!chk_new_cypher->GetValue())) {
			wxMessageBox(wxString("Error al guardar el archivo \"")+file.GetFullPath()+"\".","Error",wxOK|wxICON_ERROR,this);
		} else {
			wxMessageBox("Ejercicio generado correctamente","PSeInt",wxOK|wxICON_INFORMATION,this);
			pack.SaveConfig( GetFilePath("config.ini") );
		}
	}
}

void mxCreatorWindow::OnInputPath (wxCommandEvent & event) {
	wxDirDialog dlg(this,"Seleccione un directorio","");
	if (dlg.ShowModal()==wxID_OK) {
		input_path->SetValue(dlg.GetPath());
		timer.Start(10,true);
	}
}

void mxCreatorWindow::OnInputChange (wxCommandEvent & event) {
	timer.Start(500,true);
}

void mxCreatorWindow::OnShowErrors (wxCommandEvent & event) {
	EnableDisable();	
}

void mxCreatorWindow::OnGenerateSolutions (wxCommandEvent & event) {
	wxString cmd = pseint_command; 
	cmd.Replace("--create_new_test_package=1",wxString("\"")+GetFilePath("solution.psc")+"\"",false);
	(new mxMainWindow())->RunAllTests(cmd,true);
}

void mxCreatorWindow::OnCheckPass (wxCommandEvent & event) {
	chk_new_cypher->SetValue(chk_pass->GetValue());
	EnableDisable();
}

void mxCreatorWindow::EnableDisable ( ) {
	
	bool is_ok=pack.IsOk();
	
	msg_ok->Enable(is_ok);
	msg_bad->Enable(is_ok);
	chk_pass->Enable(is_ok);
	password1->Enable(is_ok&&chk_pass->GetValue());
	chk_new_cypher->Enable(is_ok&&chk_pass->GetValue());
	show_errors->Enable(is_ok);
	show_solutions->Enable(is_ok&&show_errors->GetValue());
	stop_on_first_error->Enable(is_ok&&show_errors->GetValue());
	shuffle_tests->Enable(is_ok);
	solutions_button->Enable(is_ok);
	ok_button->Enable(is_ok);
	static1->Enable(is_ok);
	static2->Enable(is_ok);
	ok_button->Enable(pack.IsOk());
	solutions_button->Enable(pack.IsOk()&&wxFileName::FileExists(GetFilePath("solution.psc")));
	static3->Enable(false);
	num_tests->Enable(false);
	help_html->Enable(false);
	base_psc->Enable(false);
}

wxString mxCreatorWindow::GetFilePath (const wxString &name) {
	wxString ipath = input_path->GetValue();
	if (ipath.EndsWith("\\")||ipath.EndsWith("/")) ipath.RemoveLast();
	return ipath+wxFileName::GetPathSeparator()+name;
}

void mxCreatorWindow::OnInputReload (wxCommandEvent & event) {
	LoadDirectory();
}

void mxCreatorWindow::LoadDirectory ( ) {
	timer.Stop();
	if (!input_path->GetValue().IsEmpty()) {
		if (pack.Load(input_path->GetValue())) {
			wxArrayString names;
			num_tests->SetValue(wxString()<<pack.GetNames(names));
			shuffle_tests->SetValue(pack.GetConfigBool("mezclar casos"));
			show_errors->SetValue(pack.GetConfigStr("mostrar casos fallidos")=="primero"||pack.GetConfigStr("mostrar casos fallidos")=="todos");
			stop_on_first_error->SetValue(pack.GetConfigStr("mostrar casos fallidos")=="primero");
			show_solutions->SetValue(pack.GetConfigBool("mostrar soluciones"));
			msg_bad->SetValue(pack.GetConfigStr("mensaje error"));
			msg_ok->SetValue(pack.GetConfigStr("mensaje exito"));
			base_psc->SetValue(!pack.GetBaseSrc().IsEmpty());
			help_html->SetValue(!pack.GetHelp().IsEmpty());
		}
	} else pack.Reset();
	EnableDisable();
}

