#include "mxUpdatesChecker.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "mxBitmapButton.h"
#include "mxHelpWindow.h"
#include <wx/utils.h>
#include <wx/bitmap.h>
#include "mxArt.h"
#include <wx/textfile.h>

BEGIN_EVENT_TABLE(mxUpdatesChecker, wxDialog)
	EVT_BUTTON(wxID_CANCEL,mxUpdatesChecker::OnCloseButton)
	EVT_BUTTON(wxID_OK,mxUpdatesChecker::OnChangesButton)
	EVT_BUTTON(wxID_FIND,mxUpdatesChecker::OnProxyButton)
	EVT_CLOSE(mxUpdatesChecker::OnClose)
	EVT_END_PROCESS(wxID_ANY,mxUpdatesChecker::OnProcessEnds)
END_EVENT_TABLE()

wxProcess *mxUpdatesChecker::process;
	
mxUpdatesChecker::mxUpdatesChecker(bool show) : wxDialog(main_window, wxID_ANY, _T("Buscar Actualizaciones"), wxDefaultPosition, wxSize(450,150) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

//	wxBoxSizer *iSizer = new wxBoxSizer(wxHORIZONTAL);
	
	done=false; shown=show;
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	check = new wxCheckBox(this,wxID_ANY,_T("Verificar siempre al iniciar"));
	check->SetValue(config->check_for_updates);
	
	text = new wxStaticText(this,wxID_ANY,_T("Consultando web..."),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
	mySizer->AddStretchSpacer();
	mySizer->Add(text,wxSizerFlags().Border(wxALL,5).Expand().Proportion(0));
	mySizer->AddStretchSpacer();
	mySizer->Add(check,wxSizerFlags().Border(wxALL,5));
//	mySizer->Add(pSizer,sizers->BA5);
	
	
	close_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,_T("&Cerrar"));
	changes_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,_T("Ir al &sitio..."));
	proxy_button = new mxBitmapButton (this,wxID_FIND,bitmaps->buttons.ok,_T("Conf. Proxy..."));
	buttonSizer->Add(changes_button,wxSizerFlags().Border(wxALL,5).Right());
	buttonSizer->Add(proxy_button,wxSizerFlags().Border(wxALL,5).Right());
	buttonSizer->Add(close_button,wxSizerFlags().Border(wxALL,5).Right());
	mySizer->Add(buttonSizer,wxSizerFlags().Right());
	
//	iSizer->Add(new wxStaticBitmap(this,wxID_ANY, wxBitmap(_T("upgrade.png"), wxBITMAP_TYPE_PNG)),sizers->BA10);
//	iSizer->Add(mySizer,sizers->Exp1);
//	SetSizer(iSizer);
	SetSizer(mySizer);
	changes_button->Hide();
	if (show) { 
		Show();
		close_button->SetFocus();
		wxYield();
	}
	CheckNow();
}

void mxUpdatesChecker::CheckNow() {
	
	text->SetLabel(_T("Consultando web..."));

#ifdef __WIN32__
	wxString command("updatem.exe ");
#else
	wxString command(_T("./updatem.bin "));
#endif
	
	if (config->proxy.Len())
		command<<_T("--proxy ")<<config->proxy<<_T(" ");
	
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,_T("updatem.res")));
	command<<_T("--child \"")<<temp_file<<_T("\"");
	command<<_T(" pseint ");
	
	if (process) process->Detach();
	process = new wxProcess(this->GetEventHandler(),wxID_ANY);
	if (wxExecute(command,wxEXEC_ASYNC,process)<=0) {
		if (shown) {
			text->SetLabel(_T("Error al conectarse al servidor."));
		} else 
			Destroy();
	}
}

void mxUpdatesChecker::OnClose(wxCloseEvent &evt) {
	config->check_for_updates = check->GetValue();
	if (process) process->Detach();
	process=NULL;
	Destroy();
}

void mxUpdatesChecker::OnProxyButton(wxCommandEvent &evt) {
	wxString res = wxGetTextFromUser(_T("Ingrese la direccion del proxy ( ej: 192.168.0.120:3128 ):"), _T("Buscar Actualizaciones") , config->proxy, this);
	config->proxy=res;
	CheckNow();
}

void mxUpdatesChecker::OnCloseButton(wxCommandEvent &evt) {
	Close();
}

void mxUpdatesChecker::OnChangesButton(wxCommandEvent &evt) {
	wxLaunchDefaultBrowser(_T("http://pseint.sourceforge.net?page=descargas.php"));
	Close();
}

void mxUpdatesChecker::BackgroundCheck() {
	
	wxString cur_date(wxDateTime::Now().Format(_T("%Y%m%d")));
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,_T("updatem.ts")));
	wxTextFile fil(temp_file);
	if (fil.Exists() && fil.Open() && fil.GetLineCount() && fil.GetFirstLine()==cur_date) {
		fil.Close();
		return;
	}
	fil.Close(); fil.Create(); fil.Clear();
	fil.AddLine(cur_date);
	fil.Write();
	fil.Close();
	
	main_window->GetStatusBar()->SetStatusText(_T("Buscar actualizaciones: Consultando..."));
	
	new mxUpdatesChecker(false);
}

void mxUpdatesChecker::OnProcessEnds(wxProcessEvent &evt) {
	delete process;
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,_T("updatem.res")));
	wxTextFile fil(temp_file);
	if (!fil.Exists() || !fil.Open() || !fil.GetLineCount()) {
		main_window->GetStatusBar()->SetStatusText(_T("Buscar actualizaciones: Error al conectarse al servidor."));
		text->SetLabel(_T("Error al conectarse al servidor."));
		GetSizer()->Layout();
		if (!shown) Destroy();
		fil.Close();
		return;
	}
	wxString res=fil.GetFirstLine();
	if (res==_T("nonews")) {
		main_window->GetStatusBar()->SetStatusText(_T("Buscar actualizaciones: No hay versiones nuevas disponibles."));
		text->SetLabel(_T("No hay nuevas versiones disponibles."));
		GetSizer()->Layout();
		if (!shown) Destroy();
	} else if (res==_T("update")) {
		wxString str;
		str<<_T("Hay una nueva version disponible en\nhttp://pseint.sourceforge.net (")<<fil.GetNextLine()<<_T(")");
		text->SetLabel(str);
		proxy_button->Hide();
		changes_button->Show();
		GetSizer()->Layout();
		if (!shown) Show();
		changes_button->SetFocus();
	} else {
		text->SetLabel(_T("Error al conectarse al servidor."));
	}
	fil.Close();
}
