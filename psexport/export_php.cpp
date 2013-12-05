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
	base_zero_arrays=false;
}

void PhpExporter::borrar_pantalla(t_output &prog, string param, string tabs){
	if (for_testing)
		insertar(prog,tabs+"echo \"\\n\";");
	else
		insertar(prog,tabs+"echo \"\\n\"; // no hay forma directa de borrar la pantalla con php");
}

void PhpExporter::esperar_tecla(t_output &prog, string param, string tabs){
	use_stdin=true;
	if (for_testing)
		insertar(prog,tabs+"fgetc($stdin);");
	else
		insertar(prog,tabs+"fgetc($stdin); // a diferencia del pseudocódigo, espera un Enter, no cualquier tecla");
}

void PhpExporter::esperar_tiempo(t_output &prog, string tiempo, bool mili, string tabs) {
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
		it++;
	}
	insertar(prog,tabs+"echo "+linea+(saltar?",\"\\n\";":";"));
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
		else { read_strings=true; insertar(prog,tabs+varname+"=rtrim(fgets($stdin),\"\\n\");"); }
		it++;
	}
}

static string make_aux_var(int n) { stringstream ss; ss<<"$aux_var_"<<n; return ss.str(); }

void PhpExporter::paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string var=ToLower((*r).par2), aux=ToLower((*r).par1);
	const int *dims=memoria->LeerDims(var);
	if (!dims) { insertar(prog,string("Error: ")+var+" no es un arreglo"); return; }
	int n=dims[0];
	for(int i=0;i<n;i++) { 
		string aux_2=aux, var_2=var;
		if (i!=0) var_2=make_aux_var(n);
		if (i+1!=n) aux_2=make_aux_var(n+1);
		insertar(prog,tabs+"foreach ("+var_2+" as "+aux_2+") {");
		tabs+="\t";
	}
	bloque(prog,++r,q,tabs+"\t");
	for(int i=0;i<n;i++) { 
		insertar(prog,tabs+"}");
		tabs.erase(tabs.size()-1);
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
		return string("sqrtf")+args;
	} else if (name=="RC") {
		return string("sqrtf")+args;
	} else if (name=="ABS") {
		return string("abs")+args;
	} else if (name=="LN") {
		return string("log")+args;
	} else if (name=="EXP") {
		return string("exp")+args;
	} else if (name=="AZAR") {
		return string("(rand()%")+colocarParentesis(get_arg(args,1))+")";
	} else if (name=="ATAN") {
		return string("atan")+args;
	} else if (name=="TRUNC") {
		return string("floor")+args;
	} else if (name=="REDON") {
		return string("floor(")+colocarParentesis(get_arg(args,1))+"+.5)";
	} else if (name=="CONCATENAR") {
		return string("(")+get_arg(args,1)+"."+get_arg(args,2)+")";
	} else if (name=="LONGITUD") {
		return convertirAString(get_arg(args,1))+".size()";
	} else if (name=="SUBCADENA") {
		return convertirAString(get_arg(args,1))+".substr("+get_arg(args,2)+","+get_arg(args,3)+"-"+get_arg(args,2)+"+1)";
	} else if (name=="CONVERTIRANUMERO") {
		string s=get_arg(args,1);
		if (es_cadena_constante(s)) return string("atof(")+s+")";
		else return string("atof(")+colocarParentesis(s)+".c_str())";
	} else {
		if (name=="MINUSCULAS") use_func_minusculas=true;
		if (name=="MAYUSCULAS") use_func_mayusculas=true;
		if (name=="CONVERTIRATEXTO") use_func_convertiratexto=true;
		return ToLower(name)+args; // no deberia pasar esto
	}
}

void PhpExporter::header(t_output &out) {
	out.push_back("<?php");
	init_header(out,"/* "," */");
	if (use_stdin) out.push_back("\t$stdin = fopen('php://stdin','r');");
	if (use_arreglo_max) {
		if (!for_testing) {
			out.push_back("// En C++ no se puede dimensionar un arreglo estático con una dimensión no constante.");
			out.push_back("// PSeInt sobredimensionará el arreglo utilizando un valor simbólico ARREGLO_MAX.");
			out.push_back("// Sería posible crear un arreglo dinámicamente con los operadores new y delete, pero");
			out.push_back("// este mecanismo aún no está soportado en las traducciones automáticas de PSeInt.");
		}
		out.push_back("#define ARREGLO_MAX 100");
		if (!for_testing) out.push_back("");
	}
	if (read_strings) {
		if (!for_testing) {
			out.push_back("// Para leer variables de texto se utiliza una $x=rtrim(fgets($stdin) porque el string");
			out.push_back("// que lee fgets incluye el caracter de fin de linea (entonces se usa rtrim para");
			out.push_back("// quitarlo); y la alternativa fscanf($stdin,\"%s\",$x) solo lee una palabra (lo cual");
			out.push_back("// no sería equivalente a la instrucción Leer del pseudocódigo).");
			out.push_back("");
		}
	}
}

void PhpExporter::translate_single(t_output &out, t_proceso &proc) {
	
	t_proceso_it it=proc.begin(); Funcion *f;
	if (it->nombre=="PROCESO") { f=NULL; set_memoria(main_process_name); }
	else { int x; f=ParsearCabeceraDeSubProceso(it->par1,false,x); set_memoria(f->id); }
	
	t_output out_proc;
	if (!f) {
		bloque(out,++proc.begin(),proc.end(),"\t");
		return;
	} else {
		bloque(out_proc,++proc.begin(),proc.end(),"\t\t");
		string dec="\tfunction "; dec+=ToLower(f->id)+"(";
		for(int i=1;i<=f->cant_arg;i++) {
			if (i!=1) dec+=", ";
			if (f->pasajes[i]==PP_REFERENCIA) dec+="&";
			dec+="$"; dec+=ToLower(f->nombres[i]);
		}
		out.push_back(dec+") {");
		delete f;
	}
	t_output_it ito=out_proc.begin();
	while (ito!=out_proc.end()) {
		if ( (*ito).find("($stdin,")!=string::npos || (*ito).find("($stdin)")!=string::npos) {
			insertar(out,"\t\tglobal $stdin;"); break;
		} else ito++;
	}
	insertar_out(out,out_proc);
	insertar(out,"\t}");
	if (!for_testing) out.push_back("");
	delete memoria;
}

void PhpExporter::translate(t_output &out, t_programa &prog) {
	
	TiposExporter(prog,false);
	
	t_output aux;
	for (t_programa_it it=prog.begin();it!=prog.end();++it)
		translate_single(aux,*it);	
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
		insertar(prog,tabs+"settype("+*it+",'"+tipo+"');");
		it++;
	}
}

string PhpExporter::make_varname(string varname) {
	return string("$")+ToLower(varname);
}

void PhpExporter::asignacion(t_output &prog, string param1, string param2, string tabs){
	insertar(prog,tabs+param1+" = "+param2+";");
}

void PhpExporter::dimension(t_output &prog, t_arglist &args, string tabs) {
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		string name=*it;
		name.erase(name.find("("));
		insertar(prog,tabs+"$"+ToLower(name)+" = array();");
		it++;
	}
}
