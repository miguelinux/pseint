#include "GLstuff.h"
#include "Application.h"
#include "MainWindow.h"
#include "Version.h"
#include "Global.h"
#include "Entity.h"
#include "Comm.h"
#include "Load.h"
using namespace std;

bool mxApplication::OnInit() {
	
	if (argc==2 && string(argv[1])=="--version") {
		_print_version_info("psDraw3");
		return 0;
	} else if (argc==3 && string(argv[1])=="--version") {
		_write_version_info("psDraw2",argv[2]);
		return 0;
	}
	
	GlobalInit();
	
	int id=-1, port=-1;
	string fname;
	for(int i=1;i<argc;i++) { 
		string a(argv[i]);
		if (a=="--forcesemicolons") force_semicolons=true;
		else if (a=="--nassischneiderman") Entity::nassi_schneiderman=true;
		else if (a=="--alternativeio") Entity::alternative_io=true;
		else if (a=="--shapecolors") Entity::shape_colors=true;
		else if (a=="--noedit") edit_on=false;
		else if (a=="--nowordoperators") word_operators=false;
		else if (a.size()>=5 && a.substr(0,5)=="--id=") {
			id=atoi(a.substr(5).c_str());
		} else if (a.size()>=7 && a.substr(0,7)=="--port=") {
			port=atoi(a.substr(7).c_str());
		} else fname=a;
	}
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
