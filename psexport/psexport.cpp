#include <fstream>
#include "../pseint/new_funciones.h"
#include "../pseint/new_programa.h"
#include "../pseint/utils.h"
#include "defines.h"
#include "version.h"
#include "export_common.h"
#include "export_cpp.h"
#include "export_vb.h"
#include "export_c.h"
#include "export_php.h"
#include "export_java.h"
#include "export_python3.h"
#include "export_pascal.h"
#include "exportexp.h"
#include "export_javascript.h"
#include "export_html.h"
#include "export_matlab.h"
#include "export_cs.h"
#include "export_qbasic.h"
using namespace std;

// otras funciones
bool cargar(list<t_proceso> &algs, string archivo){
	t_proceso p;
	ifstream f(archivo.c_str());
	string s;
	int x;
	if (!f.is_open()) return false;
	while (!f.eof()) {
		t_instruccion i;
		getline(f,s);
		programa.PushBack(s);
		for (unsigned int ii=0;ii<s.size();ii++) if(s[ii]=='\t') s[ii]=' ';
		if (LeftCompare(s,"#comment ")) { 
			i.nombre="#comment";
			i.par1 = s.substr(9);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"#comment-inline ")) { 
			i.nombre="#comment-inline";
			i.par1 = s.substr(16);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"PROCESO ")) { 
			i.nombre="PROCESO";
			main_process_name=s.substr(8);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"SUBPROCESO")) { 
			i.nombre="SUBPROCESO";
			i.par1=CutString(s,11);
			p.insert(p.end(),i);
			int x;
			Funcion *f=ParsearCabeceraDeSubProceso(i.par1,false,x);
			subprocesos[f->id]=f;
		} else if (s=="FINPROCESO" || s=="FINSUBPROCESO"){ 
			algs.push_back(p); p.clear();
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
		} else if (LeftCompare(s,"HASTA QUE ")) {
			i.nombre="HASTAQUE"; i.par1=CutString(s,10);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"MIENTRAS QUE ")) {
			i.nombre="MIENTRASQUE"; i.par1=CutString(s,13);
			p.insert(p.end(),i);
		} else if (s=="REPETIR") {
			i.nombre="REPETIR"; p.insert(p.end(),i);
//		} else if (LeftCompare(s,"PROCESO ")) {
//			(*(p.begin())).nombre=CutString(s,8);
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
		} else if (LeftCompare(s,"ESPERAR ")) {
			i.nombre="ESPERAR"; 
			i.par1=s.substr(s.find(" ",0)+1);
			i.par2=i.par1.substr(i.par1.find(" ",0)+1);
			i.par1=i.par1.substr(0,i.par1.find(" ",0));
			if (i.par2[i.par2.size()-1]==';') i.par2=i.par2.substr(0,i.par2.size()-1);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"ESPERARTECLA")) {
			i.nombre="ESPERARTECLA"; i.par1="";
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"ESCRIBIR ")) {
			i.nombre="ESCRIBIR"; i.par1=CutString(s,9,1);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"INVOCAR ")) {
			i.nombre="INVOCAR"; i.par1=CutString(s,8,1);
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
		} else if (!s.empty() && s[s.size()-1]==':') {
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
	if (!p.empty()) algs.push_back(p); 
	return true;
}

LangSettings lang(LS_DO_NOT_INIT);

int main(int argc, char *argv[]){

	string fname_in,fname_out;

	_handle_version_query("psExport");
	
	lang.Reset();
	
	for (int i=1;i<argc;i++) {
		string s=argv[i];
		if (s=="--install-test") {
			cout<<"OK";
			return 0;
		} else if (s=="--help") {
			cerr<<"Use: "<<argv[0]<<" [--base_zero_arrays=1] [--lang=<lenguaje>] <in_file.drw> <out_file.cpp>\n";
			return 1;
		} else if (s.substr(0,7)=="--lang=") {
			s.erase(0,7); 
			if (s=="c" || s=="c99") exporter=new CExporter();
			else if (s=="c++" || s=="cpp" || s=="c++98" || s=="cpp98" || s=="c++03" || s=="cpp03") exporter=new CppExporter();
			else if (s=="c#" || s=="cs" || s=="csharp") exporter=new CSharpExporter();
			else if (s=="htm" || s=="html") exporter=new HTMLExporter();
			else if (s=="java") exporter=new JavaExporter();
			else if (s=="js" || s=="javascript") exporter=new JavaScriptExporter();
			else if (s=="m" || s=="matlab") exporter=new MatLabExporter();
			else if (s=="pas" || s=="pascal") exporter=new PascalExporter();
			else if (s=="php") exporter=new PhpExporter();
			else if (s=="py" || s=="pyton" || s=="py3" || s=="python3") exporter=new Python3Exporter(3);
			else if (s=="py2" || s=="python2") exporter=new Python3Exporter(2);
			else if (s=="vb" || s=="visualbasic") exporter=new VbExporter();
			else if (s=="bas" || s=="qb" || s=="qbasic" || s=="quickbasic") exporter=new QBasicExporter();
			else {
				if (s=="ook") { 
					char s[]="Uû!op!fsft!vo!psbohvuâo-!qfsp!upnb!vob!cbobob;"; int i=0; while(s[i]!='\0') s[i++]--; cout<<s<<endl;
					cout<<"\t    _\n\t   | |\n\t  /  /\n\t /  /|\n\t | | |\n\t | | |\n\t | | |\n\t \\ | |\n\t  \\\\ |\n\t   \\__\\"<<endl;
					return 0;
				} else {
					cerr<<"El lenguaje no es válido. Los lenguajes disponibles son: bas, c, cpp, html, java, js, pas, php, py2, py3, vb"<<endl;
				}
			}
		} else if (s=="--for-testing") {
			for_test=true;
		} else if (s.substr(0,2)=="--" && lang.ProcessConfigLine(s.substr(2))) {
			; // procesado en lang.ProcessConfigLine
		} else if (fname_in.size() && fname_out.size()) {
			cerr<<"Argumentos incorrectos"<<endl;
			return 1;
		} else if (fname_in.size()) {
			fname_out=s;
		} else {
			fname_in=s;
		}
	}
	if (!fname_out.size()) {
		cerr<<"Argumentos incorrectos: falta archivo de salida."<<endl;
		return 1;
	}
	if (!exporter) {
		cerr<<"Argumentos incorrectos: no se especificó lenguaje."<<endl;
		return 1;
	}
	lang.Fix();
	input_base_zero_arrays=lang[LS_BASE_ZERO_ARRAYS];
	lang[LS_BASE_ZERO_ARRAYS]=output_base_zero_arrays; // lo va a consultar el evaluador de expresiones
	LoadFunciones();
	
	//cargar programa
	t_programa prog;
	if (!cargar(prog,fname_in)) {
		cerr<<"No se pudo abrir el archivo "<<fname_in<<".\n"; 
		return 1;
	}
	// convertir
	t_output out;
	exporter->translate(out,prog);
	
#ifdef _USE_COUT
#define fout cout
#else
	ofstream fout(fname_out.c_str());
#endif
	t_output::iterator it=out.begin();
	while (it!=out.end())
		fout<<*(it++)<<endl;
#ifndef _USE_COUT
	fout.close();
#endif
	
	UnloadFunciones();
	return 0;
}
	
