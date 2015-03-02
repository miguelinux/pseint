#include "Application.h"
#include "mxMainWindow.h"
#include "version.h"
#include <iostream>
#include <cstdlib>

IMPLEMENT_APP(mxApplication)
	
Package pack;

bool mxApplication::OnInit() {
	
	srand(time(0));
	_handle_version_query("psEval");
	
	if (argc<4) {
		std::cerr<<"Use "<<argv[0]<<" <archivo_de_ejercicio> <clave> <comando pseint...>"<<std::endl;
		return false;
	}
	wxString args; for(int i=3;i<argc;i++) { args+=argv[i]; args+=" "; } args+="--foreval";
	return (new mxMainWindow())->Start(argv[1],argv[2],args);
}


