#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
using namespace std;

#include "Global.h"
#include "Version.h"
#include "Events.h"
#include "Load.h"
#include "Comm.h"

int main(int argc, char **argv) {
	for (int i=0;i<circle_steps;i++) {
		cosx[i]=cos((i*2*M_PI)/circle_steps);
		sinx[i]=sin((i*2*M_PI)/circle_steps);
	}
	cosx[circle_steps]=cosx[0];
	sinx[circle_steps]=sinx[0];
	int id=-1, port=-1;
	string fname;
	for(int i=1;i<argc;i++) { 
		string a(argv[i]);
		if (a=="--version") {
			cerr<<"psedraw v2 "<<VERSION<<endl;
			return 0;
		}
		else if (a=="--noedit") edit_on=false;
		else if (a.size()>=5 && a.substr(0,5)=="--id=") {
			id=atoi(a.substr(5).c_str());
		} else if (a.size()>=7 && a.substr(0,7)=="--port=") {
			port=atoi(a.substr(7).c_str());
		} else fname=a;
	}
	if (port!=-1 && id!=-1) {
		if (!Connect(port,id)) edit_on=false;
	}
	if (fname.length()) Load(fname.c_str());
	else Load();
	glutInit (&argc, argv);
	initialize();
	glutMainLoop();
	return 0;
}
