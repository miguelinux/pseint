#include "export_tipos.h"
#include <sstream>
#include <cstdlib>
#include "../pseint/new_evaluar.h"
#include "../pseint/utils.h"
#include "version.h"
#include "new_memoria.h"
#include "exportexp.h"
#include "new_funciones.h"
#include "export_common.h"
using namespace std;

static void AplicarTipo(const string &s, tipo_var t) {
	int i=0, j=s.size()-1;
	AplicarTipo(s,i,j,t);
}

TiposExporter::TiposExporter(t_programa &prog, bool switch_only_for_integers) {
	this->switch_only_for_integers=switch_only_for_integers;
	t_output out; translate(out,prog);
}

void TiposExporter::esperar_tiempo(t_output &prog, string tiempo, bool mili, string tabs) {
	AplicarTipo(tiempo,vt_numerica);
}

void TiposExporter::dimension(t_output &prog, t_arglist &args, string tabs) {
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		string args=*it;
		args.erase(0,args.find("(")+1);
		args.erase(args.size()-1,1);
		t_arglist dims; sep_args(args,dims);
		t_arglist_it it2=dims.begin();
		while (it2!=dims.end()) {
			AplicarTipo(*it2,vt_numerica_entera);
			++it2;
		}
		++it;
	}
}

void TiposExporter::esperar_tecla(t_output &prog, string param,string tabs) { }

void TiposExporter::borrar_pantalla(t_output &prog, string param,string tabs) { }

void TiposExporter::invocar(t_output &prog, string param, string tabs){ }

void TiposExporter::escribir(t_output &prog, t_arglist args, bool saltar, string tabs){ }

void TiposExporter::leer(t_output &prog, t_arglist args, string tabs) { }

void TiposExporter::asignacion(t_output &prog, string param1, string param2, string tabs) {
	tipo_var t;
	expresion(param2,t);
	AplicarTipo(param1,t);
}

void TiposExporter::si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs){
	AplicarTipo((*r).par1,vt_logica);
	bloque(prog,++r,q,tabs+"\t");
	if (q!=s) bloque(prog,++q,s,tabs+"\t");
}

void TiposExporter::mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	AplicarTipo((*r).par1,vt_logica);
	bloque(prog,++r,q,tabs+"\t");
}

void TiposExporter::segun(t_output &prog, list<t_proceso_it> its, string tabs) {
	t_arglist opts;
	list<t_proceso_it>::iterator p,q;
	q=p=its.begin(); t_proceso_it i=*q;
	insertar(opts,(*i).par1);
	++q;++p;
	while (++p!=its.end()) {
		t_proceso_it i=*q;
		if ((*i).par1!="DE OTRO MODO") {
			sep_args((*i).par1,opts);
		}
		bloque(prog,++i,*p,tabs+"\t");
		++q;
	}
	
	tipo_var t;
	if (switch_only_for_integers) {
		t=vt_numerica_entera;
	} else {
		t_arglist_it it=opts.begin();
		while (it!=opts.end()) {
			tipo_var aux;
			expresion(*it,aux);
			if (!t.set(aux)) return;
			++it;
		}
	}
	
	t_arglist_it it=opts.begin();
	while (it!=opts.end()) {
		AplicarTipo(*it,t);
		++it;
	}
	
}

void TiposExporter::repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	AplicarTipo((*q).par1,vt_logica);
	bloque(prog,++r,q,tabs+"\t");
}

void TiposExporter::para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string var=expresion((*r).par1), ini=expresion((*r).par2), fin=expresion((*r).par3), paso=(*r).par4;
	AplicarTipo(var,vt_numerica);
	AplicarTipo(ini,vt_numerica);
	AplicarTipo(fin,vt_numerica);
	AplicarTipo(paso,vt_numerica);
	bloque(prog,++r,q,tabs+"\t");
}

void TiposExporter::paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs) {
	bloque(prog,++r,q,tabs+"\t");
}

string TiposExporter::function(string name, string args) {
	return ToLower(name)+args; 
}

void TiposExporter::translate_single_proc(t_output &out, Funcion *f, t_proceso &proc) {
	t_output out_proc; 
	bloque(out_proc,++proc.begin(),proc.end(),"\t");
	memoria=NULL; // para que translate_all_procs no la elimine
}

void TiposExporter::translate(t_output &out, t_programa &prog) {
	ExporterBase *old=exporter;
	exporter=this;
	translate_all_procs(out,prog);
	exporter=old;
}

string TiposExporter::get_constante(string name) { return name; }

string TiposExporter::get_operator(string op, bool for_string) { return op; }

string TiposExporter::make_string(string cont) { return string("\'")+cont+"\'"; }

void TiposExporter::definir(t_output &prog, t_arglist &arglist, string tipo, string tabs) {
	tipo_var vt=vt_desconocido;
	if (tipo=="ENTERO") vt=vt_numerica_entera;
	else if (tipo=="REAL") vt=vt_numerica;
	else if (tipo=="LOGICO") vt=vt_logica;
	else vt=vt_caracter;
	t_arglist_it it=arglist.begin();
	while (it!=arglist.end()) {
		memoria->DefinirTipo(expresion(*it),vt,vt.rounded);
		++it;
	}
}
