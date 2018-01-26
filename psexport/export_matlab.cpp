/**
* @file export_matlab.cpp
*
* Las traducciones están basadas en los ejemplos enviados por Jaime Meza
**/
#include "export_matlab.h"
#include "../pseint/new_funciones.h"
#include "../pseint/utils.h"
#include "export_common.h"
#include "version.h"
#include "exportexp.h"
#include "../pseint/new_evaluar.h"
#include "export_tipos.h"

MatLabExporter::MatLabExporter() {
	output_base_zero_arrays=false;
	use_comparar_cadenas=false;
	use_string_matrix=false;
}

void MatLabExporter::esperar_tecla(t_output &prog, string param, string tabs){
	insertar(prog,tabs+"pause;");
}

void MatLabExporter::esperar_tiempo(t_output &prog, string tiempo, bool milis, string tabs){
	tipo_var t; tiempo=expresion(tiempo,t); // para que arregle los nombres de las variables
	stringstream inst;
	inst<<"pause(";
	if (milis) inst<<colocarParentesis(tiempo)<<"/1000"; 
	else inst<<tiempo;
	inst<<");";
	insertar(prog,tabs+inst.str());
}

void MatLabExporter::borrar_pantalla(t_output &prog, string param, string tabs){
	insertar(prog,tabs+"clc;");
}

void MatLabExporter::invocar(t_output &prog, string param, string tabs){
	string linea=expresion(param);
	if (linea[linea.size()-1]!=')') linea+="()";
	insertar(prog,tabs+linea+";");
}

void MatLabExporter::escribir(t_output &prog, t_arglist args, bool saltar, string tabs) {
	string args_num, args_str;
	bool have_strings=false;
	int args_count=0;
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		tipo_var t;
		string varname=expresion(*it,t);
		args_num+=varname;
		if (t==vt_caracter) {
			args_str+=varname;
			have_strings=true;
		} else {
			args_str+=string("num2str(")+varname+")";
		}
		args_num+=","; args_str+=",";
		++it; args_count++;
	}
	string args_final=have_strings?args_str:args_num; args_final.erase(args_final.size()-1,1);
	string linea=string("disp(")+(args_count>1?"[":"")+(args_final)+(args_count>1?"]":"")+");";
	if (!saltar && !for_test) linea+=" %no hay forma de escribir sin saltar de linea en MatLab";
	insertar(prog,tabs+linea);
}

void MatLabExporter::leer(t_output &prog, t_arglist args, string tabs){
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		tipo_var t;
		string varname=expresion(*it,t);
		if (t==vt_caracter) insertar(prog,tabs+varname+"=input(\'\',\'s\');");
		else insertar(prog,tabs+varname+"=input(\'\');");
		++it;
	}
}

void MatLabExporter::asignacion(t_output &prog, string param1, string param2, string tabs){
	insertar(prog,tabs+expresion(param1)+"="+expresion(param2)+";");
}

void MatLabExporter::si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs){
	insertar(prog,tabs+"if "+expresion((*r).par1));
	bloque(prog,++r,q,tabs+"\t");
	if (q!=s) {
		insertar(prog,tabs+"else");
		bloque(prog,++q,s,tabs+"\t");
	}
	insertar(prog,tabs+"end");
}

void MatLabExporter::mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"while "+expresion((*r).par1));
	bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"end");
}

void MatLabExporter::segun(t_output &prog, list<t_proceso_it> its, string tabs){
	list<t_proceso_it>::iterator p,q,r;
	q=p=its.begin();r=its.end();
	t_proceso_it i=*q;
	insertar(prog,tabs+"switch "+expresion((*i).par1));
	++q;++p;
	while (++p!=r) {
		i=*q;
		if ((*i).par1=="DE OTRO MODO")
			insertar(prog,tabs+"otherwise");
		else {
			string e=expresion((*i).par1); int en=1;
			bool comillas=false; int parentesis=0, j=0,l=e.size();
			while(j<l) {
				if (e[j]=='\''||e[j]=='\"') comillas=!comillas;
				else if (!comillas) {
					if (e[j]=='['||e[j]=='(') parentesis++;
					else if (e[j]==']'||e[j]==')') parentesis--;
					else if (parentesis==0 && e[j]==',') {
						e.replace(j,1,","); l+=5; en++;
					}
				}
				j++;
			}
			if (en>1) e=string("{")+e+"}";
			insertar(prog,tabs+"case "+e);
		}
		bloque(prog,++i,*p,tabs+"\t");
		++q;
	}
	insertar(prog,tabs+"end");
}

void MatLabExporter::repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string auxvar=get_aux_varname("aux_logica_");
	insertar(prog,tabs+auxvar+"=true;");
	insertar(prog,tabs+"while "+auxvar);
	bloque(prog,++r,q,tabs+"\t");
	if ((*q).nombre=="HASTAQUE")
		insertar(prog,tabs+"\t"+auxvar+"="+expresion(invert_expresion((*q).par1))+";");
	else
		insertar(prog,tabs+"\t"+auxvar+"="+expresion((*q).par1)+";");
	insertar(prog,tabs+"end");
	release_aux_varname(auxvar);
}

void MatLabExporter::para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs) {
	string var=expresion((*r).par1), ini=expresion((*r).par2), fin=expresion((*r).par3), paso=(*r).par4;
	if (paso=="1")
		insertar(prog,tabs+"for "+var+"="+ini+":"+fin);
	else
		insertar(prog,tabs+"for "+var+"="+ini+":"+expresion(paso)+":"+fin);
	bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"end");
}

void MatLabExporter::paracada(t_output &out, t_proceso_it r, t_proceso_it q, string tabs){
	string var=ToLower((*r).par2), aux=ToLower((*r).par1);
	const int *dims=memoria->LeerDims(var);
	if (!dims) { insertar(out,string("ERROR: ")+var+" NO ES UN ARREGLO"); return; }
	int n=dims[0];
	
	string *auxvars=new string[n];
	for(int i=0;i<n;i++) auxvars[i]=get_aux_varname("aux_index_");
	
	string vname=var, sep="(";
	for(int i=0;i<n;i++) { 
		string idx=auxvars[i];
		insertar(out,tabs+"for "+idx+"=1:size("+var+","+IntToStr(i+1)+")");
		vname+=sep+idx; sep=",";
		tabs+="\t";
	}
	vname+=")";
	
	for(int i=n-1;i>=0;i--) release_aux_varname(auxvars[i]);
	delete []auxvars;
	
	t_output aux_out;
	bloque(aux_out,++r,q,tabs);
	replace_var(aux_out,aux,vname);
	insertar_out(out,aux_out);
	for(int i=0;i<n;i++) { 
		tabs.erase(tabs.size()-1);
		insertar(out,tabs+"end");
	}
}

string MatLabExporter::function(string name, string args) {
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
		return string("(randi"+args+"-1)");
	} else if (name=="ATAN") {
		return string("atan")+args;
	} else if (name=="TRUNC") {
		return string("floor")+args;
	} else if (name=="REDON") {
		return string("round")+args;
	} else if (name=="CONCATENAR") {
		return string("[")+args.substr(1,args.size()-2)+"]";
	} else if (name=="LONGITUD") {
		return string("length")+args;
	} else if (name=="SUBCADENA") {
		string desde=get_arg(args,2);
		string hasta=get_arg(args,3);
		if (input_base_zero_arrays) desde=sumarOrestarUno(desde,true);
		if (input_base_zero_arrays) hasta=sumarOrestarUno(hasta,true);
		return get_arg(args,1)+"("+desde+":"+hasta+")";
	} else if (name=="CONVERTIRANUMERO") {
		return string("str2num")+args;
	} else if (name=="CONVERTIRATEXTO") {
		return string("num2str")+args;
	} else if (name=="MINUSCULAS") {
		return string("lower")+args;
	} else if (name=="MAYUSCULAS") {
		return string("upper")+args;
	} else {
		return ToLower(name)+args; // no deberia pasar esto
	}
}

void MatLabExporter::translate_single_proc(t_output &out, Funcion *f, t_proceso &proc) {
	
	// cabecera del proceso
	if (!f) {
		out.push_back(string("function ")+ToLower(main_process_name)+"()");
	} else {
		string dec="function ";
		if (f->nombres[0]!="") {
			dec+=ToLower(f->nombres[0])+" = ";
		}
		dec+=ToLower(f->id)+"(";
		for(int i=1;i<=f->cant_arg;i++) {
			if (i!=1) dec+=", ";
			dec+=ToLower(f->nombres[i])/*,f->pasajes[i]==PP_REFERENCIA)*/;
		}
		dec+=")";
		out.push_back(dec);
	}
	
	// cuerpo del proceso
	bloque(out,++proc.begin(),proc.end(),"\t");
	
	// cola del proceso
	out.push_back("end");
	if (!for_test) out.push_back("");
}

void MatLabExporter::translate(t_output &out, t_programa &prog) {
	// cppcheck-suppress unusedScopedObject
	TiposExporter(prog,false); // para que se cargue el mapa_memorias con memorias que tengan ya definidos los tipos de variables que correspondan
	
	// procesos y subprocesos
	t_output out_aux;
	translate_all_procs(out_aux,prog);
	
	// cabecera
	init_header(out,"% ");
	if (!for_test) insertar(out,"");
	if (use_string_matrix && !for_test) {
		insertar(out,"% El algoritmo generado declara uno o más vectores/matrices cuyos elementos son");
		insertar(out,"% cadenas de caracteres En MatLab, estos no son arreglos comunes, sino arreglos de");
		insertar(out,"% celdas (se crean con la función cell en lugar de zeros como losdemás), y por");
		insertar(out,"% ello deben ser accedidos utilizando llaves {} en lugar de paréntisis () para");
		insertar(out,"% sus índices. Es posible que el código generado utilice paréntesis donde deberían");
		insertar(out,"% ir llaves, por lo que deberá realizar estas correcciones manualmente.");
		insertar(out,"");
	}
	if (use_comparar_cadenas) {
		if (!for_test) {
			insertar(out,"% No hay en matlab una función para comparar cadenas completas por mayor/menor,");
			insertar(out,"% solo por igualdad/desigualdad. Por eso se crea esta función auxiliar que lo hace");
			insertar(out,"% realizando comparaciones letra y por letra y de sus longitudes. Su comportamiento");
			insertar(out,"% es similar al strcmp de C, al comparar el resultado con 0 se obtiene el mismo");
			insertar(out,"% resultado que si compararamos las dos cadenas.");
			insertar(out,"%	Ej: para hacer a<=b usamos comparar_cadenas(a,b)<=0");
		}
		insertar(out,"function r=comparar_cadenas(a,b)");
		insertar(out,"\tn1=length(a);");
		insertar(out,"\tn2=length(b);");
		insertar(out,"\tn=min(n1,n2);");
		insertar(out,"\ti=0;");
		insertar(out,"\twhile i<n");
		insertar(out,"\t\tif (a(i)!=b(i))");
		insertar(out,"\t\t\tr=a(i)-b(i);");
		insertar(out,"\t\t\tbreak;");
		insertar(out,"\t\tend");
		insertar(out,"\t\ti=i+1;");
		insertar(out,"\tend");
		insertar(out,"\tif i==n");
		insertar(out,"\t\tr=n1-n2;");
		insertar(out,"\tend");
		insertar(out,"end");
		if (!for_test) insertar(out,"");
	}

	insertar_out(out,out_aux);
}

string MatLabExporter::get_constante(string name) {
	if (name=="PI") return "pi";
	if (name=="VERDADERO") return "true";
	if (name=="FALSO") return "false";
	return name;
}

string MatLabExporter::get_operator(string op, bool for_string) {
	// para agrupar operaciones y alterar la jerarquia
	if (op=="(") return "("; 
	if (op==")") return ")";
	// para llamadas a funciones
	if (op=="{") return "("; 
	if (op=="}") return ")";
	// para indices de arreglos
	if (op=="[") return "(";
	if (op==",") return ",";
	if (op=="]") return ")";
	// de cadenas
	if (for_string) {
		if (op=="+") { return "func [arg1,arg2]"; }
		if (op=="=") return "func strcmp(arg1,arg2)"; 
		if (op=="<>") { use_comparar_cadenas=true; return "func ~strcmp(arg1,arg2)"; }
		if (op=="<") { use_comparar_cadenas=true; return "func comparar_cadenas(arg1,arg2)<0"; }
		if (op==">") { use_comparar_cadenas=true; return "func comparar_cadenas(arg1,arg2)>0"; }
		if (op=="<=") { use_comparar_cadenas=true; return "func comparar_cadenas(arg1,arg2)<=0"; }
		if (op==">=") { use_comparar_cadenas=true; return "func comparar_cadenas(arg1,arg2)>=0"; }
	}
	// otros
	if (op=="+") return "+"; 
	if (op=="-") return "-"; 
	if (op=="/") return "/"; 
	if (op=="*") return "*";
	if (op=="^") return "^";
	if (op=="%") return "func mod(arg1,arg2)";
	if (op=="=") return "=="; 
	if (op=="<>") return "~="; 
	if (op=="<") return "<"; 
	if (op==">") return ">"; 
	if (op=="<=") return "<="; 
	if (op==">=") return ">="; 
	if (op=="&") return " && "; 
	if (op=="|") return " || "; 
	if (op=="~") return "~"; 
	return op; // no deberia pasar nunca
}

string MatLabExporter::make_string (string cont) {
	return string("\'")+cont+"\'";
}

void MatLabExporter::dimension(t_output &prog, t_arglist &args, string tabs) {
	ExporterBase::dimension(prog,args,tabs);
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		// obtener nombre y dimensiones
		string name=*it, dims=*it;
		name.erase(name.find("("));
		dims.erase(0,dims.find("(")+1);
		dims.erase(dims.size()-1,1);
		dims=expresion(dims);
		if (dims.find(",")==string::npos) dims=string("1,")+dims;
		// armar la linea que hace el new
		if (memoria->LeerTipo(name)==vt_caracter) {
			insertar(prog,tabs+ToLower(name)+"=cell("+dims+");");
			use_string_matrix=true;
		} else {
			insertar(prog,tabs+ToLower(name)+"=zeros("+dims+");");
		}
		++it;
	}
}

void MatLabExporter::definir(t_output &prog, t_arglist &arglist, string tipo, string tabs) {
	if (tipo=="ENTERO") tipo="0";
	else if (tipo=="REAL") tipo="0";
	else if (tipo=="LOGICO") tipo="false";
	else tipo="\'\'";
	t_arglist_it it=arglist.begin();
	while (it!=arglist.end()) {
		insertar(prog,tabs+expresion(*it)+"="+tipo+";");
		++it;
	}
}

void MatLabExporter::comentar (t_output & prog, string text, string tabs) {
	if (!for_test) insertar(prog,tabs+"% "+text);
}

