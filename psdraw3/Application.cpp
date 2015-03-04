#include "GLstuff.h"
#include "Application.h"
#include "MainWindow.h"
#include "Version.h"
#include "Global.h"
#include "Entity.h"
#include "Comm.h"
#include "Load.h"
#include "../pseint/LangSettings.h"
using namespace std;

bool mxApplication::OnInit() {
	
	_handle_version_query("psDraw3");
	
	GlobalInit();
	LangSettings lang;
	int id=-1, port=-1;
	string fname;
	for(int i=1;i<argc;i++) { 
		string a(argv[i]);
		if (a=="--shapecolors") Entity::shape_colors=true;
		else if (a=="--noedit") edit_on=false;
		else if (a.substr(0,2)=="--" && !lang.ProcessConfigLine(a.substr(2))) {
			; // done in lang.ProcessConfigLine
		}
		else if (a.size()>=5 && a.substr(0,5)=="--id=") {
			id=atoi(a.substr(5).c_str());
		} else if (a.size()>=7 && a.substr(0,7)=="--port=") {
			port=atoi(a.substr(7).c_str());
		} else if (a.substr(0,2)=="--" && lang.ProcessConfigLine(a.substr(2))) {
			; // procesado en lang.ProcessConfigLine
		} else fname=a;
	}
	lang.Fix();
	Entity::nassi_schneiderman=lang[LS_USE_NASSI_SCHNEIDERMAN];
	Entity::alternative_io=lang[LS_USE_ALTERNATIVE_IO_SHAPES];
	if (port!=-1 && id!=-1) {
		if (!::Connect(port,id)) edit_on=false;
	}
	if (fname.length()) Load(fname.c_str());
	else Load();
	glutInit (&argc, argv);
	new MainWindow(wxString("PSDraw v2 - ")<<start->label.c_str());
	return true;
	
}

IMPLEMENT_APP(mxApplication)
