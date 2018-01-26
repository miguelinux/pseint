#include "export_php.h"
#include <sstream>
#include <cstdlib>
#include "../pseint/new_evaluar.h"
#include "../pseint/utils.h"
#include "version.h"
#include "new_memoria.h"
#include "exportexp.h"
#include "new_funciones.h"
#include "export_common.h"
#include "export_tipos.h"
using namespace std;

PhpExporter::PhpExporter():CppExporter() {
	use_stdin=false;
	read_strings=false;
	output_base_zero_arrays=false;
}

void PhpExporter::borrar_pantalla(t_output &prog, string param, string tabs){
	if (for_test)
		insertar(prog,tabs+"echo PHP_EOL;");
	else
		insertar(prog,tabs+"echo PHP_EOL; // no hay forma directa de borrar la pantalla con php");
}

void PhpExporter::esperar_tecla(t_output &prog, string param, string tabs){
	use_stdin=true;
	if (for_test)
		insertar(prog,tabs+"fgetc($stdin);");
	else
		insertar(prog,tabs+"fgetc($stdin); // a diferencia del pseudocódigo, espera un Enter, no cualquier tecla");
}

void PhpExporter::esperar_tiempo(t_output &prog, string tiempo, bool mili, string tabs) {
	tipo_var t; tiempo=expresion(tiempo,t); // para que arregle los nombres de las variables
	stringstream inst;
	if (mili) inst<<"usleep("<<colocarParentesis(tiempo)<<"*1000);";
	else inst<<"sleep("<<tiempo<<");";
	insertar(prog,tabs+inst.str());
}

void PhpExporter::escribir(t_output &prog, t_arglist args, bool saltar, string tabs){
	t_arglist_it it=args.begin();
	string linea="";
	while (it!=args.end()) {
		if (linea.size()) linea+=",";
		linea+=expresion(*it);
		++it;
	}
	insertar(prog,tabs+"echo "+linea+(saltar?",PHP_EOL;":";"));
}

void PhpExporter::leer(t_output &prog, t_arglist args, string tabs) {
	use_stdin=true;
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		tipo_var t;
		string varname=expresion(*it,t);
		if (t==vt_numerica && t.rounded) insertar(prog,tabs+"fscanf($stdin,\"%d\","+varname+");");
		else if (t==vt_numerica) insertar(prog,tabs+"fscanf($stdin,\"%f\","+varname+");");
		else if (t==vt_logica) insertar(prog,tabs+"fscanf($stdin,\"%d\","+varname+");");
		else { read_strings=true; insertar(prog,tabs+varname+"=rtrim(fgets($stdin),PHP_EOL);"); }
		++it;
	}
}


void PhpExporter::paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs) {
	string var=ToLower((*r).par2), aux=ToLower((*r).par1);
	const int *dims=memoria->LeerDims(var);
	if (!dims) { insertar(prog,string("Error: ")+var+" no es un arreglo"); return; }
	int n=dims[0];
	
	string *auxvars=new string[n+1];
	auxvars[0]=var;
	for(int i=1;i<n;i++) auxvars[i]=get_aux_varname("aux_var_");
	auxvars[n]=aux;
	
	for(int i=0;i<n;i++) { 
		string aux_2=auxvars[i+1], var_2=auxvars[i];
		insertar(prog,tabs+"foreach ($"+var_2+" as $"+aux_2+") {");
		tabs+="\t";
	}
	for(int i=n-1;i>0;i--) release_aux_varname(auxvars[i]);
	delete []auxvars;
	
	bloque(prog,++r,q,tabs);
	for(int i=0;i<n;i++) { 
		tabs.erase(tabs.size()-1);
		insertar(prog,tabs+"}");
	}
}



string PhpExporter::function(string name, string args) {
	if (name=="SEN") {
		return string("sin")+args;
	} else if (name=="TAN") {
		return string("tan")+args;
	} else if (name=="ASEN") {
		return string("asin")+args;
	} else if (name=="ACOS") {
		return string("acos")+args;
	} else if (name=="COS") {
		return string("cos")+args;
	} else if (name=="RAIZ") {
		return string("sqrt")+args;
	} else if (name=="RC") {
		return string("sqrt")+args;
	} else if (name=="ABS") {
		return string("abs")+args;
	} else if (name=="LN") {
		return string("log")+args;
	} else if (name=="EXP") {
		return string("exp")+args;
	} else if (name=="AZAR") {
		return string("rand(0,")+sumarOrestarUno(get_arg(args,1),false)+")";
	} else if (name=="ATAN") {
		return string("atan")+args;
	} else if (name=="TRUNC") {
		return string("floor")+args;
	} else if (name=="REDON") {
		return string("round")+args;
	} else if (name=="CONCATENAR") {
		return get_arg(args,1)+"."+get_arg(args,2);
	} else if (name=="LONGITUD") {
		return string("strlen")+args;
	} else if (name=="MAYUSCULAS") {
		return string("strtoupper")+args;
	} else if (name=="MINUSCULAS") {
		return string("strtolower")+args;
	} else if (name=="SUBCADENA") {
		string desde=get_arg(args,2);
		string cuantos=sumarOrestarUno(get_arg(args,3)+"-"+get_arg(args,2),true);
		if (!input_base_zero_arrays) desde=sumarOrestarUno(desde,false);
		return string("substr(")+get_arg(args,1)+","+desde+","+cuantos+")";
	} else if (name=="CONVERTIRANUMERO") {
		return string("(")+colocarParentesis(get_arg(args,1))+"+0)";
	} else if (name=="CONVERTIRATEXTO") {
		return string("((string)")+colocarParentesis(get_arg(args,1))+")";
	} else 
		return ToLower(name)+args; // no debería pasar nunca esto
}

void PhpExporter::header(t_output &out) {
	out.push_back("<?php");
	init_header(out,"\t// ");
	if (use_stdin) out.push_back("\t$stdin = fopen('php://stdin','r');");
	if (read_strings) {
		if (!for_test) {
			out.push_back("\t// Para leer variables de texto se utiliza una $x=rtrim(fgets($stdin) porque el string");
			out.push_back("\t// que lee fgets incluye el caracter de fin de linea (entonces se usa rtrim para");
			out.push_back("\t// quitarlo); y la alternativa fscanf($stdin,\"%s\",$x) solo lee una palabra (lo cual");
			out.push_back("\t// no sería equivalente a la instrucción Leer del pseudocódigo).");
			out.push_back("\t");
		}
	}
}

void PhpExporter::translate_single_proc(t_output &out, Funcion *f, t_proceso &proc) {
	
	t_output out_proc;
	if (!f) {
		bloque(out,++proc.begin(),proc.end(),"\t");
		return;
	} 
	
	bloque(out_proc,++proc.begin(),proc.end(),"\t\t");
	string dec="\tfunction "; dec+=ToLower(f->id)+"(";
	for(int i=1;i<=f->cant_arg;i++) {
		if (i!=1) dec+=", ";
		if (f->pasajes[i]==PP_REFERENCIA) dec+="&";
		dec+="$"; dec+=ToLower(f->nombres[i]);
	}
	out.push_back(dec+") {");
	
	t_output_it ito=out_proc.begin();
	while (ito!=out_proc.end()) {
		if ( (*ito).find("($stdin,")!=string::npos || (*ito).find("($stdin)")!=string::npos) {
			insertar(out,"\t\tglobal $stdin;"); break;
		} else ++ito;
	}
	insertar_out(out,out_proc);
	if (f && f->nombres[0]!="") 
		insertar(out,string("\t\treturn ")+make_varname(f->nombres[0])+";");
	insertar(out,"\t}");
	if (!for_test) out.push_back("");
}

void PhpExporter::translate(t_output &out, t_programa &prog) {
	
	// cppcheck-suppress unusedScopedObject
	TiposExporter(prog,false);
	
	t_output aux;
	translate_all_procs(aux,prog,"\t");
	header(out);
	insertar_out(out,aux);
	insertar(out,"?>");
}

string PhpExporter::get_operator(string op, bool for_string) {
	if (op=="+" && for_string) return ".";
	else return CppExporter::get_operator(op,false);
}

string PhpExporter::make_string (string cont) {
	return string("\'")+cont+"\'";
}

void PhpExporter::definir(t_output &prog, t_arglist &arglist, string tipo, string tabs) {
	if (tipo=="ENTERO") tipo="integer";
	else if (tipo=="REAL") tipo="float";
	else if (tipo=="LOGICO") tipo="boolean";
	else tipo="string";
	t_arglist_it it=arglist.begin();
	while (it!=arglist.end()) {
		insertar(prog,tabs+"settype("+expresion(*it)+",'"+tipo+"');");
		++it;
	}
}

string PhpExporter::make_varname(string varname) {
	return string("$")+ToLower(varname);
}

void PhpExporter::asignacion(t_output &prog, string param1, string param2, string tabs){
	insertar(prog,tabs+expresion(param1)+" = "+expresion(param2)+";");
}

void PhpExporter::dimension(t_output &prog, t_arglist &args, string tabs) {
	ExporterBase::dimension(prog,args,tabs);
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		string name=*it;
		name.erase(name.find("("));
		insertar(prog,tabs+"$"+ToLower(name)+" = array();");
		++it;
	}
}

