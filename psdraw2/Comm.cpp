#include "Comm.h"
#include <sstream>
#include "../pseint/zockets.h"
#include "Global.h"
#include "Entity.h"
#include <fstream>
#include "Events.h"
#include "Load.h"
using namespace std;

ZOCKET zocket=ZOCKET_ERROR; // para comunicarse con wxPSeInt


bool Connect(int port, int id) {
	zocket = zocket_llamar(port,"127.0.0.1");
	if (zocket==ZOCKET_ERROR) return false;
	stringstream ss;
	ss<<"hello-flow "<<id<<"\n";
	string s=ss.str();
	zocket_escribir(zocket,s.c_str(),s.size());
	if (zocket==ZOCKET_ERROR) return false;
	return true;
}

bool SendUpdate(bool run, bool exp) {
	if (!Save()) return false;
	if (zocket==ZOCKET_ERROR) return false;
	if (exp)
		zocket_escribir(zocket,"export\n",7);
	else if (run)
		zocket_escribir(zocket,"run\n",4);
	else
		zocket_escribir(zocket,"reload\n",7);
	if (zocket==ZOCKET_ERROR) return false;
	return true;
}

bool SendHelp() {
	if (zocket==ZOCKET_ERROR) return false;
	zocket_escribir(zocket,"help\n",5);
	if (zocket==ZOCKET_ERROR) return false;
	return true;
}

void CloseComm( ) {
	if (zocket!=ZOCKET_ERROR) {
		zocket_cerrar(zocket);
	}
}

void ReadComm( ) {
	static char *rec=new char[256]; int cant=256;
	if (zocket!=ZOCKET_ERROR && zocket_leer(zocket,rec,cant)) {
		rec[cant]=0;
		if (string(rec)=="edit") { edit_on=true; Raise(); }
		else if (string(rec)=="noedit") { edit_on=false; Raise(); }
		else if (string(rec)=="raise") Raise();
		else if (string(rec)=="quit") Salir();
	}
}

