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
#include "mxStatusBar.h"
#include "version.h"
#include "Logger.h"
#include <wx/stattext.h>
#include <wx/textdlg.h>
#include "string_conversions.h"

BEGIN_EVENT_TABLE(mxUpdatesChecker, wxDialog)
	EVT_BUTTON(wxID_CANCEL,mxUpdatesChecker::OnCloseButton)
	EVT_BUTTON(wxID_OK,mxUpdatesChecker::OnChangesButton)
	EVT_BUTTON(wxID_FIND,mxUpdatesChecker::OnProxyButton)
	EVT_CLOSE(mxUpdatesChecker::OnClose)
	EVT_END_PROCESS(wxID_ANY,mxUpdatesChecker::OnProcessEnds)
END_EVENT_TABLE()

wxProcess *mxUpdatesChecker::process;
	
mxUpdatesChecker::mxUpdatesChecker(bool show) : wxDialog(main_window, wxID_ANY, "Buscar Actualizaciones", wxDefaultPosition, wxSize(450,150) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	_LOG("mxUpdatesChecker::mxUpdatesChecker show="<<(show?"true":"false"));
	
//	wxBoxSizer *iSizer = new wxBoxSizer(wxHORIZONTAL);
	
	done=false; shown=show;
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	check = new wxCheckBox(this,wxID_ANY,"Verificar siempre al iniciar");
	check->SetValue(config->check_for_updates);
	
	text = new wxStaticText(this,wxID_ANY,"Consultando web...",wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
	mySizer->AddStretchSpacer();
	mySizer->Add(text,wxSizerFlags().Border(wxALL,5).Expand().Proportion(0));
	mySizer->AddStretchSpacer();
	mySizer->Add(check,wxSizerFlags().Border(wxALL,5));
//	mySizer->Add(pSizer,sizers->BA5);
	
	
	close_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,"&Cerrar");
	changes_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,"Ir al &sitio...");
	proxy_button = new mxBitmapButton (this,wxID_FIND,bitmaps->buttons.ok,"Conf. Proxy...");
	buttonSizer->Add(changes_button,wxSizerFlags().Border(wxALL,5));
	buttonSizer->Add(proxy_button,wxSizerFlags().Border(wxALL,5));
	buttonSizer->Add(close_button,wxSizerFlags().Border(wxALL,5));
	mySizer->Add(buttonSizer,wxSizerFlags().Right());
	
//	iSizer->Add(new wxStaticBitmap(this,wxID_ANY, wxBitmap("upgrade.png", wxBITMAP_TYPE_PNG)),sizers->BA10);
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
	
	text->SetLabel(_Z("Consultando web..."));

#ifdef __WIN32__
	wxString command("updatem.exe ");
#else
	wxString command("./updatem.bin ");
#endif
	
	if (config->proxy.Len())
		command<<"--proxy "<<config->proxy<<" ";
	
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,"updatem.res"));
	command<<"--child \""<<temp_file<<"\"";
	command<<" pseint ";
	
	if (process) process->Detach();
	process = new wxProcess(this->GetEventHandler(),wxID_ANY);
#ifdef __APPLE__
	process->Redirect(); // no necesito la salida, pero sin esto en mac y con wx8 execute no funciona
#endif
	_LOG("mxUpdatesChecker::CheckNow "<<command);
	if (wxExecute(command,wxEXEC_ASYNC,process)<=0) {
		if (shown) {
			text->SetLabel(_Z("Error al conectarse al servidor."));
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
	wxString res = wxGetTextFromUser(_Z("Ingrese la direccion del proxy ( ej: 192.168.0.120:3128 ):"),_Z("Buscar Actualizaciones"), config->proxy, this);
	config->proxy=res;
	CheckNow();
}

void mxUpdatesChecker::OnCloseButton(wxCommandEvent &evt) {
	Close();
}

void mxUpdatesChecker::OnChangesButton(wxCommandEvent &evt) {
#ifdef __WIN3__
	bool do_exit = wxMessageBox(_Z("Si decide actualizar PSeInt ahora, deberá cerrar \n"
								   "esta instancia antes de ejecutar el nuevo instalador.\n\n"
								   "¿Desea cerrar PSeInt ahora?"),_Z("Actualización"),wxYES_NO|wxICON_QUESTION,this)==wxYES;
#else
	bool do_exit = false;
#endif
	wxLaunchDefaultBrowser("http://pseint.sourceforge.net?page=actualizacion.php&os=" ARCHITECTURE);
	Close();
	if (do_exit) main_window->OnFileClose(evt);
}

void mxUpdatesChecker::BackgroundCheck() {
	
	wxString cur_date(wxDateTime::Now().Format("%Y%m%d"));
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,"updatem.ts"));
	wxTextFile fil(temp_file);
	if (fil.Exists() && fil.Open() && fil.GetLineCount() && fil.GetFirstLine()==cur_date) {
		fil.Close();
		return;
	}
	fil.Close(); fil.Create(); fil.Clear();
	fil.AddLine(cur_date);
	fil.Write();
	fil.Close();
	
	status_bar->SetStatus(STATUS_UPDATE_CHECKING);
	
	new mxUpdatesChecker(false);
}

void mxUpdatesChecker::OnProcessEnds(wxProcessEvent &evt) {
	delete process; process=NULL;
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,"updatem.res"));
	wxTextFile fil(temp_file);
	if (!fil.Exists() || !fil.Open() || !fil.GetLineCount()) {
		status_bar->SetStatus(STATUS_UPDATE_ERROR);
		text->SetLabel(_Z("Error al conectarse al servidor."));
		GetSizer()->Layout();
		if (!shown) Destroy();
		fil.Close();
		return;
	}
	wxString res=fil.GetFirstLine();
	if (res=="nonews") {
		status_bar->SetStatus(STATUS_UPDATE_NONEWS);
		text->SetLabel(_Z("No hay nuevas versiones disponibles."));
		GetSizer()->Layout();
		if (!shown) Destroy();
	} else if (res=="update") {
		status_bar->SetStatus(STATUS_UPDATE_FOUND);
		wxString str;
		str<<_Z("Hay una nueva version disponible en\nhttp://pseint.sourceforge.net (")<<fil.GetNextLine()<<_Z(")");
		text->SetLabel(str);
		proxy_button->Hide();
		changes_button->Show();
		GetSizer()->Layout();
		if (!shown) Show();
		changes_button->SetFocus();
	} else {
		text->SetLabel(_Z("Error al conectarse al servidor."));
	}
	fil.Close();
}
