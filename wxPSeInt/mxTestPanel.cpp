#include <wx/button.h>
#include <wx/sizer.h>
#include "mxTestPanel.h"
#include <wx/stattext.h>
#include "string_conversions.h"
#include "ids.h"
#include "mxSource.h"
#include "ConfigManager.h"
#include "mxProcess.h"
#include "Logger.h"
#include "mxMainWindow.h"
#include "version.h"
#include <wx/fs_mem.h>
#include <wx/msgdlg.h>

BEGIN_EVENT_TABLE(mxTestPanel,wxPanel)
	EVT_BUTTON(mxID_TESTPACK_RUN,mxTestPanel::OnRun)
	EVT_BUTTON(wxID_HELP,mxTestPanel::OnHelp)
END_EVENT_TABLE()
	
bool mxTestPanel::first_time = true;

mxTestPanel::mxTestPanel(wxWindow *parent) : wxPanel(parent,wxID_ANY) {
	
	if (first_time) wxFileSystem::AddHandler(new wxMemoryFSHandler); first_time=false;
	
	src = NULL;
	sizer = new wxBoxSizer(wxHORIZONTAL);
	eval_button = new wxButton(this,mxID_TESTPACK_RUN,_Z("Evaluar..."));
	label = new wxStaticText(this,wxID_ANY,_Z("Cargando ejercicio..."),wxDefaultPosition,wxDefaultSize);
	sizer->Add(eval_button,wxSizerFlags().Border(wxALL,5));
	sizer->Add(label,wxSizerFlags().Center().Centre().Border(wxALL,5));
	sizer->AddStretchSpacer();
	help_button = new wxButton(this,wxID_HELP,_Z(" ? "),wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
	sizer->Add(help_button,wxSizerFlags().Border(wxALL,5));
	SetSizerAndFit(sizer);
}

bool mxTestPanel::Load (const wxString & path, const wxString &key, mxSource *src) {
	this->path=path; this->key=key; this->src=src;
	if (!pack.Load(path,key)) return false;
	if (pack.GetConfigInt("version requerida")>PACKAGE_VERSION) {
		wxMessageBox(_Z("Debe actualizar PSeInt para poder abrir este ejercicio"),_Z("Error"),wxID_OK|wxICON_ERROR,this);
		return false;
	}
	if (pack.GetConfigBool("creator")) {
		Run("--create_new_test_package=1");
		return false;
	}
	label->SetLabel(_Z(" <- click aquí para evaluar su respuesta"));
	sizer->Layout();
	src->SetText(pack.GetBaseSrc());
	return true;
}

void mxTestPanel::OnRun (wxCommandEvent & event) {
	main_window->SelectSource(src); src->SaveTemp();
	Run(src->GetTempFilenamePSC());
}

void mxTestPanel::OnHelp (wxCommandEvent & event) {
	main_window->ShowQuickHelp(true,GetHelp(),false);
}


void mxTestPanel::Run (const wxString & source_fname) {
	wxString cmd = config->pseval_command +" \""+path+"\" \""+(key.Len()?key:"--nokey")+"\" "+ config->pseint_command+" "+mxProcess::GetProfileArgs() + " \""+source_fname+"\"";
	_LOG("mxTestPanel::Run");
	_LOG("    "<<cmd);
	wxExecute(cmd,wxEXEC_ASYNC);
}

bool mxTestPanel::Destroy ( ) {
	pack.UnloadImages();
	return wxPanel::Destroy();
}

