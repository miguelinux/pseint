#include <wx/image.h>
#include <wx/socket.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <iostream>
#include "Logger.h"
#include "string_conversions.h"
#include "Application.h"
#include "mxMainWindow.h"
#include "version.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "HelpManager.h"
#include "DebugManager.h"
#include "mxProfile.h"
#include "mxArt.h"
#include "mxUpdatesChecker.h"
#include "mxIconInstaller.h"
#include "CommunicationsManager.h"
#include "mac-stuff.h"
#include "error_recovery.h"
#include <wx/textfile.h>
using namespace std;



IMPLEMENT_APP(mxApplication)
	
wxSplashScreen *splash;

bool mxApplication::OnInit() {
	
	_handle_version_query("wxPSeInt");
	
	fix_mac_focus_problem();
	
	utils = new mxUtils;
	if (argc==3 && wxString(argv[1])=="--logger") new Logger(argv[2]);
	
	wxFileName f_path = wxGetCwd(); 
	f_path.MakeAbsolute();
	wxString cmd_path = f_path.GetFullPath();
	wxFileName f_cmd(argv[0]);
	
	wxFileName f_zpath = f_cmd.GetPathWithSep();
	f_zpath.MakeAbsolute();
	wxString zpath(f_zpath.GetPathWithSep());
	bool flag=false;
	if (f_zpath!=f_path) {
		if ( (flag=(wxFileName::FileExists(DIR_PLUS_FILE(zpath,_T("pseint.dir"))) || wxFileName::FileExists(DIR_PLUS_FILE(zpath,_T("PSeInt.dir")))) ) )
			wxSetWorkingDirectory(zpath);
#ifdef __APPLE__
		else if ( (flag=(wxFileName::FileExists(DIR_PLUS_FILE(zpath,_T("../Resources/pseint.dir"))) ||wxFileName::FileExists(DIR_PLUS_FILE(zpath,_T("../Resources/PSeInt.dir")))) ) ) {
			zpath = DIR_PLUS_FILE(zpath,_T("../Resources"));
			wxSetWorkingDirectory(zpath);
		}
#endif
		else 
			zpath = cmd_path;
	}
	
	if (!flag && !wxFileName::FileExists(_T("pseint.dir")) && !wxFileName::FileExists(_T("PSeInt.dir")))
		wxMessageBox(_Z("PSeInt no pudo determinar el directorio donde fue instalado. Compruebe que el directorio de trabajo actual sea el correcto."),_T("Error"));
	
	srand(time(0));
	
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxXPMHandler);
	
	config = new ConfigManager(zpath);
	if (logger) logger->DumpVersions();
	
	wxSocketBase::Initialize();
	
	bitmaps = new mxArt(config->images_path);
	help = new HelpManager;
	
	// create main window
	if (config->size_x<=0 || config->size_y<=0)
		main_window = new mxMainWindow(wxDefaultPosition, wxSize(800, 600));
	else
		main_window = new mxMainWindow(wxPoint(config->pos_x,config->pos_y), wxSize(config->size_x,config->size_y));
	main_window->Maximize(config->maximized);
	main_window->Show(true);
	if (logger || argc==1)
		main_window->NewProgram();
	else
		for (int i=1;i<argc;i++)
			main_window->OpenProgram(DIR_PLUS_FILE(cmd_path,argv[i]));
	SetTopWindow(main_window);
	wxYield();	
	main_window->Refresh();

#if !defined(__WIN32__) && !defined(__APPLE__)	
	if (config->version<20120626) 
		new mxIconInstaller(true);
#endif
	
//#ifdef __APPLE__
//	wxMessageBox(_T("Bienvenido a PSeInt para Mac OS X. Esta una de las primeras versiones beta en este sistema operativo y por lo tanto algunas funcionalidades todavia no se encuentran disponibles: La ejecucion paso a paso no funcionara correctamente y la visualizacion de diagramas de flujo solo sera posible si cuenta con las bibliotecas del servidor X."),_T("Advertencia"));
//#endif
	
	if (config->profile==NO_PROFILE) {
		_LOG("mxApplication::OnInit NO_PROFILE");
		wxMessageBox(_Z(
			"Bienvenido a PSeInt. Antes de comenzar debes seleccionar un perfil "
			"para ajustar el pseudolenguaje a tus necesidades. Si tu universidad "
			"o institución no aparece en la lista, notifica a tu profesor para "
			"que envíe sus datos a través del sitio web. "
			),_Z("Bienvenido a PSeInt"),wxOK,main_window);
		config->profile=_Z("Flexible");
		new mxProfile(main_window);
	} else {
		if (config->check_for_updates) 
			mxUpdatesChecker::BackgroundCheck();
	}
	
	comm_manager=new CommunicationsManager();
	
	RecoverFromError();
	
	return true;
	
}

void mxApplication::RecoverFromError ( ) {
	
	wxTextFile fil(er_get_recovery_fname());	
	if (!fil.Exists()) return;
	
	wxArrayString rec_names, rec_files;
	fil.Open();
	fil.GetFirstLine(); // hora de explosion
	wxString str;
	while (!fil.Eof()) {
		str = fil.GetNextLine(); // nombre de la pestaña
		if (str.Len() && !fil.Eof()) {
			fil.GetNextLine(); // nombre real del archivo antes de la explosion
			rec_names.Add(str);
			rec_files.Add(str=fil.GetNextLine()); // nombre del archivo de segurida generado en la explosion
		}
	}
	fil.Close();
	
	if ( !rec_names.GetCount() ) return;
	int res =wxMessageBox("PSeInt no se cerró correctamente durante su última ejecución.\n"
							"Algunos algoritmos en los que trabajaba fueron guardados,\n"
							"automaticamente y ahora puede recuperarlos. ¿Desea recuperarlos?","PSeInt - Recuperación ante errores",wxYES_NO|wxICON_WARNING);
	if (res==wxYES) {
		for (unsigned int i=0;i<rec_files.GetCount();i++) {
			mxSource *src =	main_window->OpenProgram(rec_files[i],false);
			src->SetPageText(rec_names[i]);
			src->SetModify(true);
			src->sin_titulo = true;
		}
	}	
	
	wxRemoveFile(er_get_recovery_fname());
}

