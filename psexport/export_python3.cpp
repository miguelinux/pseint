#include "export_python3.h"
#include "../pseint/utils.h"
#include "version.h"
#include "new_memoria.h"
#include "exportexp.h"
#include "new_funciones.h"
#include "export_common.h"
#include "export_tipos.h"
#include <stack>
using namespace std;

Python3Exporter::Python3Exporter(int version) {
	this->version=version;
	import_pi=false;
	import_sleep=false;
	import_sqrt=false;
	import_log=false;
	import_exp=false;
	import_sin=false;
	import_cos=false;
	import_tan=false;
	import_asin=false;
	import_acos=false;
	import_atan=false;
	import_randint=false;
	use_subprocesos=false;
	output_base_zero_arrays=true;
}

void Python3Exporter::dimension(t_output &prog, t_arglist &args, string tabs) {
	ExporterBase::dimension(prog,args,tabs); // creo que esto no es necesario, pero lo dejo por las dudas (del copy/paste de java)
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		// obtener nombre y dimensiones
		string name=*it, dims=*it;
		name.erase(name.find("("));
		dims.erase(0,dims.find("(")+1);
		dims.erase(dims.size()-1,1);
		t_arglist dimlist;
		sep_args(dims,dimlist);
		// obtener un elemento del tipo adecuado (para replicar por todo el arreglo)	
		string stipo="str()";
		tipo_var tipo=memoria->LeerTipo(name);
		if (tipo==vt_numerica) stipo=tipo.rounded?"int()":"float()";
		if (tipo==vt_logica) stipo="bool()";
		// armar la expresion que genera el arreglo
		dims=stipo;
		dimlist.reverse();
		t_arglist_it it2=dimlist.begin();
		stack<string> auxvars;
		while (it2!=dimlist.end()) {
			auxvars.push(get_aux_varname("ind"));
			dims=string("[")+dims+" for "+auxvars.top()+" in range("+expresion(*it2)+")]";
			++it2;
		}
		while(!auxvars.empty()) { release_aux_varname(auxvars.top()); auxvars.pop(); }
		// asignar
		insertar(prog,tabs+ToLower(name)+" = "+dims);
		++it;
	}
}

void Python3Exporter::borrar_pantalla(t_output &out, string param, string tabs){
	string linea=tabs;
	if (version==3) linea+="print(\"\")";
	else linea+="print \"\"";
	if (!for_test) linea+=" # no hay forma directa de borrar la pantalla con Python estandar";
	insertar(out,linea);
}

void Python3Exporter::esperar_tecla(t_output &prog, string param, string tabs){
	string input=version==3?"input":"raw_input";
	if (for_test)
		insertar(prog,tabs+input+"()");
	else
		insertar(prog,tabs+input+"() # a diferencia del pseudocódigo, espera un Enter, no cualquier tecla");
}

void Python3Exporter::esperar_tiempo(t_output &prog, string tiempo, bool mili, string tabs) {
	tipo_var t; tiempo=expresion(tiempo,t); // para que arregle los nombres de las variables
	import_sleep=true;
	if (mili) 
		insertar(prog,tabs+"sleep("+colocarParentesis(tiempo)+"/1000.0)");
	else
		insertar(prog,tabs+"sleep("+tiempo+")");
}

void Python3Exporter::invocar(t_output &prog, string param, string tabs){
	string linea=expresion(param);
	if (linea[linea.size()-1]!=')') 
		linea+="()";
		insertar(prog,tabs+linea);
}

void Python3Exporter::escribir(t_output &prog, t_arglist args, bool saltar, string tabs){
	t_arglist_it it=args.begin();
	string linea="print",sep=version==3?"(":" ";
	while (it!=args.end()) {
		linea+=sep; sep=",";
		linea+=expresion(*it);
		++it;
	}
	if (version==3) {
		linea+=(saltar?")":", end=\"\")");
	} else if (!saltar) linea+=",";
	insertar(prog,tabs+linea);
}

void Python3Exporter::definir(t_output &prog, t_arglist &arglist, string tipo, string tabs) {
	if (tipo=="ENTERO") tipo="int()";
	else if (tipo=="REAL") tipo="float()";
	else if (tipo=="LOGICO") tipo="bool()";
	else tipo="str()";
	t_arglist_it it=arglist.begin();
	while (it!=arglist.end()) {
		insertar(prog,tabs+expresion(*it)+" = "+tipo);
		++it;
	}
}


void Python3Exporter::leer(t_output &prog, t_arglist args, string tabs) {
	string input=version==3?"input":"raw_input";
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		tipo_var t;
		string varname=expresion(*it,t);
		if (t==vt_numerica && t.rounded) insertar(prog,tabs+varname+" = int("+input+"())");
		else if (t==vt_numerica) insertar(prog,tabs+varname+" = float("+input+"())");
		else if (t==vt_logica) insertar(prog,tabs+varname+" = bool("+input+"())");
		else  insertar(prog,tabs+varname+" = "+input+"()");
		++it;
	}
}

void Python3Exporter::asignacion(t_output &prog, string param1, string param2, string tabs){
	insertar(prog,tabs+expresion(param1)+" = "+expresion(param2));
}

void Python3Exporter::si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs){
	insertar(prog,tabs+"if "+expresion((*r).par1)+":");
	bloque(prog,++r,q,tabs+"\t");
	if (q!=s) {
		insertar(prog,tabs+"else:");
		bloque(prog,++q,s,tabs+"\t");
	}
}

void Python3Exporter::mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"while "+expresion((*r).par1)+":");
	bloque(prog,++r,q,tabs+"\t");
}

void Python3Exporter::segun(t_output &prog, list<t_proceso_it> its, string tabs){
	list<t_proceso_it>::iterator p,q,r;
	q=p=its.begin();r=its.end();
	t_proceso_it i=*q;
	string cond=expresion((*i).par1)+"==";
	++q;++p; bool first=true;
	while (++p!=r) {
		i=*q;
		bool dom=(*i).par1=="DE OTRO MODO";
		if (dom) {
			insertar(prog,tabs+"else:");
		} else {
			string line=first?"if ":"elif "; first=false;
			string e=expresion((*i).par1);
			bool comillas=false; int parentesis=0, j=0,l=e.size();
			while(j<l) {
				if (e[j]=='\''||e[j]=='\"') comillas=!comillas;
				else if (!comillas) {
					if (e[j]=='['||e[j]=='(') parentesis++;
					else if (e[j]==']'||e[j]==')') parentesis--;
					else if (parentesis==0 && e[j]==',') {
						e.replace(j,1,string(" or "+cond)); l+=4+cond.size();
					}
				}
				j++;
			}
			line+=cond+e+":";
			insertar(prog,tabs+line);
		}
		bloque(prog,++i,*p,tabs+"\t");
		++q;
	}
}

void Python3Exporter::repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"while True:"+(for_test?"":"# no hay 'repetir' en python"));
	bloque(prog,++r,q,tabs+"\t");
	if ((*q).nombre=="HASTAQUE")
		insertar(prog,tabs+"\tif "+expresion((*q).par1)+": break");
	else
		insertar(prog,tabs+"\tif "+expresion(invert_expresion((*q).par1))+": break");
}

void Python3Exporter::para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string var=expresion((*r).par1), ini=expresion((*r).par2), fin=expresion((*r).par3), paso=(*r).par4;
	string line=string("for ")+var+" in range(";
	if (ini!="0"||paso!="1") line+=ini+",";
	line+=sumarOrestarUno(fin,(*r).par4[0]!='-');
	if (paso!="1") line+=string(",")+paso;
	line+="):";
	insertar(prog,tabs+line);
	bloque(prog,++r,q,tabs+"\t");
}

void Python3Exporter::paracada(t_output &out, t_proceso_it r, t_proceso_it q, string tabs) {
	// el "for x in a" de python sirve para solo-lectura (modificar x no modifica a)
	string var=ToLower((*r).par2), aux=ToLower((*r).par1);
	const int *dims=memoria->LeerDims(var);
	int n=dims[0];
		
	string *auxvars=new string[n];
	for(int i=0;i<n;i++) auxvars[i]=get_aux_varname("aux_index_");
	
	string vname=var;
	for(int i=0;i<n;i++) { 
		string idx=auxvars[i];
		insertar(out,tabs+"for "+idx+" in range("+IntToStr(dims[i+1])+"):");
		vname+="["+idx+"]";
		tabs+="\t";
	}
	
	for(int i=n-1;i>=0;i--) release_aux_varname(auxvars[i]);
	delete []auxvars;
	
	t_output aux_out;
	bloque(aux_out,++r,q,tabs);
	replace_var(aux_out,aux,vname);
	insertar_out(out,aux_out);
}

string Python3Exporter::function(string name, string args) {
	if (name=="SEN") {
		import_sin=true;
		return string("sin")+args;
	} else if (name=="TAN") {
		import_tan=true;
		return string("tan")+args;
	} else if (name=="ASEN") {
		import_asin=true;
		return string("asin")+args;
	} else if (name=="ACOS") {
		import_acos=true;
		return string("acos")+args;
	} else if (name=="COS") {
		import_cos=true;
		return string("cos")+args;
	} else if (name=="RAIZ") {
		import_sqrt=true;
		return string("sqrt")+args;
	} else if (name=="RC") {
		import_sqrt=true;
		return string("sqrt")+args;
	} else if (name=="ABS") {
		return string("abs")+args;
	} else if (name=="LN") {
		import_log=true;
		return string("log")+args;
	} else if (name=="EXP") {
		import_exp=true;
		return string("exp")+args;
	} else if (name=="AZAR") {
		import_randint=true;
		return string("randint(0,")+sumarOrestarUno(get_arg(args,1),false)+")";
	} else if (name=="ATAN") {
		import_atan=true;
		return string("atan")+args;
	} else if (name=="TRUNC") {
		return string("int")+args;
	} else if (name=="REDON") {
		return string("round")+args;
	} else if (name=="CONCATENAR") {
		return get_arg(args,1)+"+"+get_arg(args,2);
	} else if (name=="LONGITUD") {
		return string("len")+args;
	} else if (name=="SUBCADENA") {
		string desde=get_arg(args,2);
		if (!input_base_zero_arrays) desde=sumarOrestarUno(desde,false);
		string hasta=get_arg(args,3);
		if (input_base_zero_arrays) hasta=sumarOrestarUno(hasta,true);
		return get_arg(args,1)+"["+desde+":"+hasta+"]";
	} else if (name=="CONVERTIRANUMERO") {
		return string("float")+args;
	} else if (name=="MAYUSCULAS") {
		return string("str.upper")+args;
	} else if (name=="MINUSCULAS") {
		return string("str.lower")+args;
	} else if (name=="CONVERTIRATEXTO") {
		return string("str")+args;
	} else {
		return name+args; // no debería pasar nunca
	}
}

void Python3Exporter::header(t_output &out) {
	// cabecera
	if (version==2) insertar(out,"# -*- coding: iso-8859-15 -*-");
	init_header(out,"# ");
	string math_stuff;
	if (import_pi) math_stuff+=", pi";
	if (import_sqrt) math_stuff+=", sqrt";
	if (import_log) math_stuff+=", log";
	if (import_exp) math_stuff+=", exp";
	if (import_sin) math_stuff+=", sin";
	if (import_cos) math_stuff+=", cos";
	if (import_tan) math_stuff+=", tan";
	if (import_asin) math_stuff+=", asin";
	if (import_acos) math_stuff+=", acos";
	if (import_atan) math_stuff+=", atan";
	if (math_stuff.size()) out.push_back(string("from math import")+math_stuff.substr(1));
	if (import_randint) out.push_back("from random import randint");
	if (import_sleep) out.push_back("from time import sleep");
	if (!for_test) out.push_back("");
	if (!for_test && use_subprocesos) {
		out.push_back("# En python no hay forma de elegir como pasar una variable a una");
		out.push_back("# funcion (por referencia o por valor). Las variables \"inmutables\"");
		out.push_back("# (str, int, float, bool) se pasan siempre por copia mientras que");
		out.push_back("# las demas (listas, objetos, etc.) se pasan siempre por referencia.");
		out.push_back("# Esto coincide con el comportamiento por defecto en pseint, pero");
		out.push_back("# cuando utiliza las palabras clave \"Por Referencia\" para");
		out.push_back("# alterarlo la traducción no es correcta.");
		out.push_back("");
	}
}

void Python3Exporter::translate_single_proc(t_output &out, Funcion *f, t_proceso &proc) {
	
	if (f) use_subprocesos=true;
	
	//cuerpo del proceso
	t_output out_proc;
	bloque(out_proc,++proc.begin(),proc.end(),"\t");
	
	// cabecera del proceso
	string ret; // sentencia "return ..." de la funcion
	if (!f) {
		out.push_back("if __name__ == '__main__':");
	} else {
		string dec="def ";
		dec+=ToLower(f->id)+"(";
		for(int i=1;i<=f->cant_arg;i++) {
			if (i!=1) dec+=", ";
			dec+=ToLower(f->nombres[i]);
		}
		dec+="):";
		if (f->nombres[0]!="") {
			ret=string("return ")+ToLower(f->nombres[0]);
		}
		out.push_back(dec);
	}
	
	//cuerpo del proceso
	insertar_out(out,out_proc);
	
	// cola del proceso
	if (ret.size()) out.push_back(string("\t")+ret);
	if (!for_test) out.push_back("");
}

void Python3Exporter::translate(t_output &out, t_programa &prog) {
	// cppcheck-suppress unusedScopedObject
	TiposExporter(prog,false); // para que se cargue el mapa_memorias con memorias que tengan ya definidos los tipos de variables que correspondan
	
	t_output aux_defs,aux_main;
	translate_all_procs(aux_main,aux_defs,prog);
	
	header(out);
	insertar_out(out,aux_defs);
	insertar_out(out,aux_main);
}

string Python3Exporter::get_constante(string name) {
	if (name=="PI") { import_pi=true; return "pi"; }
	if (name=="VERDADERO") return "True";
	if (name=="FALSO") return "False";
	return name;
}

string Python3Exporter::get_operator(string op, bool for_string) {
	// para agrupar operaciones y alterar la jerarquia
	if (op=="(") return "("; 
	if (op==")") return ")";
	// para llamadas a funciones
	if (op=="{") return "("; 
	if (op=="}") return ")";
	// para indices de arreglos
	if (op=="[") return "[";
	if (op==",") return "][";
	if (op=="]") return "]";
	// algebraicos, logicos, relaciones
	if (op=="+") return "+"; 
	if (op=="-") return "-"; 
	if (op=="/") return "/"; 
	if (op=="*") return "*";
	if (op=="^") return "**";
	if (op=="%") return "%";
	if (op=="=") return "=="; 
	if (op=="<>") return "!="; 
	if (op=="<") return "<"; 
	if (op==">") return ">"; 
	if (op=="<=") return "<="; 
	if (op==">=") return ">="; 
	if (op=="&") return " and "; 
	if (op=="|") return " or "; 
	if (op=="~") return "not "; 
	return op; // no deberia pasar nunca
}

string Python3Exporter::make_string (string cont) {
	for(unsigned int i=0;i<cont.size();i++)
		if (cont[i]=='\\') cont.insert(i++,"\\");
	return string("\"")+cont+"\"";
}

void Python3Exporter::comentar (t_output & prog, string text, string tabs) {
	if (!for_test) insertar(prog,tabs+"# "+text);
}

