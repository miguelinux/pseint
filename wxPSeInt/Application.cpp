#include <wx/image.h>
#include <wx/socket.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <iostream>
#include "Logger.h"
using namespace std;
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
using namespace std;

#ifdef __WXMAC__
// esto es para evitar el problema de no poder hacerle foco a la ventana en Mac sin tener que hacer un application bundle (ver OnInit)
#include <ApplicationServices/ApplicationServices.h>
#endif

IMPLEMENT_APP(mxApplication)
	
wxSplashScreen *splash;

bool mxApplication::OnInit() {
	
	if (argc==2 && wxString(argv[1])=="--version") {
		_print_version_info("wxPSeInt");
		return false;
	} else if (argc==3 && wxString(argv[1])=="--version") {
		_write_version_info("wxPSeInt",argv[2]);
		return false;
	}
	
#ifdef __WXMAC__
	// esto es para evitar el problema de no poder hacerle foco a la ventana en Mac sin tener que hacer un application bundle
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication); // este es para que pueda recibir el foco
	SetFrontProcess( &PSN ); // este es para que no aparezca en segundo plano
#endif
	
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
		wxMessageBox(_T("PSeInt no pudo determinar el directorio donde fue instalado. Compruebe que el directorio de trabajo actual sea el correcto."),_T("Error"));
	
	srand(time(0));
	
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxXPMHandler);
	
	config = new ConfigManager(zpath);
	
	wxSocketBase::Initialize();
	
	bitmaps = new mxArt(config->images_path);
	utils = new mxUtils;
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
	
#ifdef __APPLE__
	wxMessageBox(_T("Bienvenido a PSeInt para Mac OS X. Esta una de las primeras versiones beta en este sistema operativo y por lo tanto algunas funcionalidades todavia no se encuentran disponibles: La ejecucion paso a paso no funcionara correctamente y la visualizacion de diagramas de flujo solo sera posible si cuenta con las bibliotecas del servidor X."),_T("Advertencia"));
#endif
	
	if (config->profile==NO_PROFILE) {
		wxMessageBox(_T(
			"Bienvenido a PSeInt. Antes de comenzar debes seleccionar un perfil "
			"para ajustar el pseudolenguaje  tus necesidades. Si tu universidad "
			"o institucion no aparece en la lista, notifica a tu profesor para "
			"que envie sus datos a traves del sitio web. "
			),_T("Bienvenido a PSeInt"),wxID_OK,main_window);
		config->profile=_T("Flexible");
		new mxProfile(main_window);
	} else {
		if (config->check_for_updates) 
			mxUpdatesChecker::BackgroundCheck();
	}
	
	comm_manager=new CommunicationsManager();
	
	return true;
	
}
