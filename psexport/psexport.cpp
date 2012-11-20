#include <fstream>
#include "../pseint/new_funciones.h"
#include "../pseint/new_programa.h"
#include "../pseint/utils.h"
#include "defines.h"
#include "version.h"
#include "export_chooser.h"
using namespace std;

// otras funciones
t_algoritmo cargar(string archivo){
	char buffer[1024];
	t_algoritmo p;
	ifstream f(archivo.c_str());
	t_instruccion i;
	string s;
	int x;
	p.insert(p.begin(),i);
	if (!f.is_open()) {
		cerr<<"No se pudo abrir el archivo\n"; 
		(*(p.begin())).nombre="ERROR ERROR ERROR!";
		return p;
	}
	while (!f.eof()) {
		f.getline(buffer,256); s=buffer;
		programa.PushBack(buffer);
		for (unsigned int ii=0;ii<s.size();ii++) if(s[ii]=='\t') s[ii]=' ';
		if (s=="FINPROCESO"){ 
			break;
		} else if (s=="FINSI") {
			i.nombre="FINSI"; p.insert(p.end(),i);
		} else if (s=="ENTONCES") {
			i.nombre="ENTONCES"; p.insert(p.end(),i);
		} else if (s=="SINO") {
			i.nombre="SINO"; p.insert(p.end(),i);
		} else if (s=="FINSEGUN") {
			i.nombre="FINSEGUN"; p.insert(p.end(),i);
		} else if (s=="FINPARA") {
			i.nombre="FINPARA"; p.insert(p.end(),i);
		} else if (s=="FINMIENTRAS") {
			i.nombre="FINMIENTRAS"; p.insert(p.end(),i);
		} else if (s=="REPETIR") {
			i.nombre="REPETIR"; p.insert(p.end(),i);
		} else if (LeftCompare(s,"PROCESO ")) {
			(*(p.begin())).nombre=CutString(s,8);
		} else if (LeftCompare(s,"SEGUN ")) {
			i.nombre="SEGUN"; i.par1=CutString(s,6,6);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"PARA ")) {
			i.nombre="PARA"; s=CutString(s,5,6);
			x=s.find("CON PASO ",0);
			if (x>0 && x<(int)s.size()) {
				int p=8;
				while (s[x+p]==' ') p++;
				i.par4=CutString(s,x+p);
				s=CutString(s,0,s.size()-x+1);
			} else i.par4="1";
			x=s.find(" ",0);
			i.par3=CutString(s,x+7);
			s=CutString(s,0,s.size()-x);
			x=s.find("<-",0);
			i.par2=CutString(s,x+3,1);
			i.par1=CutString(s,0,s.size()-x);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"PARACADA ")) {
			i.nombre="PARACADA"; s=CutString(s,9,6);
			x=s.find(" DE ",0);
			i.par1=s.substr(0,x);
			i.par2=s.substr(x+4);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"MIENTRAS ")) {
			i.nombre="MIENTRAS"; i.par1=CutString(s,9,6);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"SI ")) {
			i.nombre="SI"; i.par1=CutString(s,3);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"BORRARPANTALLA")) {
			i.nombre="BORRARPANTALLA"; i.par1="";
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"ESPERARTECLA")) {
			i.nombre="ESPERARTECLA"; i.par1="";
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"HASTA QUE ")) {
			i.nombre="HASTAQUE"; i.par1=CutString(s,10);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"ESCRIBIR ")) {
			i.nombre="ESCRIBIR"; i.par1=CutString(s,9,1);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"ESCRIBNL ")) {
			i.nombre="ESCRIBIR"; i.par1=string("**SINSALTAR**,")+CutString(s,9,1);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"DIMENSION ")) {
			i.nombre="DIMENSION"; i.par1=CutString(s,10,1);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"DEFINIR ")) {
			i.nombre="DEFINIR"; i.par1=CutString(s,8,1);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"LEER ")) {
			i.nombre="LEER"; i.par1=CutString(s,5,1);
			p.insert(p.end(),i);
		} else if (s[s.size()-1]==':') {
			i.nombre="OPCION"; i.par1=CutString(s,0,1);
			p.insert(p.end(),i);
		} else if (s!="") {
			x=s.find("<-",0);
			if (x>0 && x<(int)s.size()) {
				i.nombre="ASIGNACION"; 
				i.par1=CutString(s,0,s.size()-x);
				i.par2=CutString(s,x+3,2);
			} else {
				i.nombre="SECUENCIAL"; i.par1=s;
			}
			p.insert(p.end(),i);
		}
	}
	f.close();
	
	return p;
}



int main(int argc, char *argv[]){

	string fname_in,fname_out;
	base_zero_arrays=false;
	force_integer_indexes=true;
	
	for (int i=1;i<argc;i++) {
		if (string(argv[i])=="--install-test") {
			cout<<"OK";
			return 0;
		} else if (string(argv[i])=="--version") {
			cout<<"PSExport "<<VERSION<<"-"<<ARCHITECTURE<<"\n by Pablo Novara - zaskar_84@yahoo.com.ar\n"; 
			return 0;
		} else if (string(argv[i])=="--help") {
			cerr<<"Use: "<<argv[0]<<" [--basezeroarrays] <in_file.drw> <out_file.cpp>\n";
			return 1;
		} else if (string(argv[i])=="--basezeroarrays") {
			base_zero_arrays=true;
		} else if (fname_in.size() && fname_out.size()) {
			cerr<<"Argumentos incorrectos"<<endl;
			return 1;
		} else if (fname_in.size()) {
			fname_out=argv[i];
		} else {
			fname_in=argv[i];
		}
	}
	if (!fname_out.size()) {
		cerr<<"Argumentos incorrectos"<<endl;
		return 1;
	}

	memoria=new Memoria(NULL);
	LoadFunciones(false);
	
	//cargar programa
	t_algoritmo alg=cargar(argv[1]);
	// convertir
	t_programa prog;
	translate_main(prog,alg);
	
#ifdef _USE_COUT
#define fout cout
#else
	ofstream fout(argv[2]);
#endif
	t_programa::iterator it=prog.begin();
	while (it!=prog.end()) 
		fout<<*(it++)<<endl;
#ifndef _USE_COUT
	fout.close();
#endif
	return 0;
}
	
