#include <wx/image.h>
#include "Application.h"
#include "GLstuff.h" // poner este depues de Application, sino genera errores de enlazado en un metodo de wxapp por alguna macro de algun #include
#include "MainWindow.h"
#include "Version.h"
#include "Global.h"
#include "Entity.h"
#include "Comm.h"
#include "Load.h"
#include "../pseint/LangSettings.h"
#include "../wxPSeInt/osdep.h"
#include "ShapesBar.h"
#include "ToolBar.h"
#include "Trash.h"
#include "ProcessSelector.h"

LangSettings lang(LS_DO_NOT_INIT);

bool mxApplication::OnInit() {
	
	_handle_version_query("psDraw3");
	
	OSDep::AppInit();
	
	lang.Reset();
	GlobalInitPre();
	
	int id=-1, port=-1;
	string fname;
	for(int i=1;i<argc;i++) { 
		string a(argv[i]);
		if (a=="--shapecolors") Entity::shape_colors=true;
		else if (a=="--noedit") edit_on=false;
		else if (a=="--nocroplabels") Entity::enable_partial_text=false;
		else if (a=="--bigicons") big_icons=true;
		else if (a.size()>=5 && a.substr(0,5)=="--id=") {
			id=atoi(a.substr(5).c_str());
		} else if (a.size()>=7 && a.substr(0,7)=="--port=") {
			port=atoi(a.substr(7).c_str());
		} else if (a.substr(0,2)=="--" && lang.ProcessConfigLine(a.substr(2))) {
			; // done in lang.ProcessConfigLine
		} else fname=a;
	}
	lang.Fix();
	Entity::nassi_shneiderman=lang[LS_USE_NASSI_SHNEIDERMAN];
	Entity::alternative_io=lang[LS_USE_ALTERNATIVE_IO_SHAPES];
	GlobalInitPost();

	glutInit (&argc, argv);
	wxImage::AddHandler(new wxPNGHandler);
	Trash::Initialize();
	ToolBar::Initialize();
	ShapesBar::Initialize();
	ProcessSelector::Initialize();
	
	if (port!=-1 && id!=-1) {
		if (!::Connect(port,id)) edit_on=false;
	}
	if (fname.length()) Load(fname.c_str());
	else Load();
	
	new MainWindow(wxString("PSDraw - ")<<start->label.c_str());
	return true;
	
}

IMPLEMENT_APP(mxApplication)
