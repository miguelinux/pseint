/**
* @file export_qbasic.cpp
*
* Las traducciones están basadas en los ejemplos enviados por el Prof. Tito Sánchez
**/
#include "../pseint/new_funciones.h"
#include "../pseint/new_evaluar.h"
#include "../pseint/utils.h"
#include "export_qbasic.h"
#include "export_common.h"
#include "version.h"
#include "exportexp.h"
#include "export_tipos.h"

QBasicExporter::QBasicExporter() {
	use_pi=use_rand=use_asin=use_acos=false;
	output_base_zero_arrays=input_base_zero_arrays;
}

void QBasicExporter::esperar_tecla(t_output &prog, string param, string tabs){
	insertar(prog,tabs+"WHILE INKEY$<>\"\": WEND");
}

void QBasicExporter::esperar_tiempo(t_output &prog, string tiempo, bool milis, string tabs){
	tipo_var t; tiempo=expresion(tiempo,t); // para que arregle los nombres de las variables
	stringstream inst;
	inst<<"SLEEP ";
	if (!milis) inst<<tiempo; 
	else inst<<"INT("<<colocarParentesis(tiempo)<<"/1000)";
	insertar(prog,tabs+inst.str());
}

void QBasicExporter::borrar_pantalla(t_output &prog, string param, string tabs){
	insertar(prog,tabs+"CLS");
}

void QBasicExporter::invocar(t_output &prog, string param, string tabs){
	string linea=expresion(param);
	if (linea[linea.size()-1]!=')') 
		linea+="()";
	string fname=ToUpper(linea.substr(0,linea.find('(')));
	const Funcion *func=EsFuncion(fname);
	if (func->nombres[0]=="") linea=string("CALL ")+linea;
	insertar(prog,tabs+linea);
}

void QBasicExporter::escribir(t_output &prog, t_arglist args, bool saltar, string tabs){
	t_arglist_it it=args.begin();
	string linea;
	while (it!=args.end()) {
		if (linea.size()) linea+=";";
		linea+=expresion(*it);
		++it;
	}
	if (saltar) linea=string("PRINT ")+linea; 
	else linea=string("PRINT ")+linea+";";
	insertar(prog,tabs+linea);
}

void QBasicExporter::leer(t_output &prog, t_arglist args, string tabs){
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		tipo_var t;
		string varname=expresion(*it,t);
		if (t==vt_caracter||t==vt_desconocido) insertar(prog,tabs+"LINE INPUT "+varname);
		else  insertar(prog,tabs+"INPUT "+varname);
		++it;
	}
}

void QBasicExporter::asignacion(t_output &prog, string param1, string param2, string tabs){
	insertar(prog,tabs+expresion(param1)+" = "+expresion(param2));
}

void QBasicExporter::si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs){
	insertar(prog,tabs+"IF "+expresion((*r).par1)+" THEN");
	bloque(prog,++r,q,tabs+"\t");
	if (q!=s) {
		insertar(prog,tabs+"ELSE");
		bloque(prog,++q,s,tabs+"\t");
	}
	insertar(prog,tabs+"END");
}

void QBasicExporter::mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"WHILE "+expresion((*r).par1));
	bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"WEND");
}

void QBasicExporter::segun(t_output &prog, list<t_proceso_it> its, string tabs){
	list<t_proceso_it>::iterator p,q,r;
	q=p=its.begin();r=its.end();
	t_proceso_it i=*q;
	insertar(prog,tabs+"SELECT CASE "+expresion((*i).par1));
	++q;++p;
	while (++p!=r) {
		i=*q;
		if ((*i).par1=="DE OTRO MODO")
			insertar(prog,tabs+"\tCASE ELSE");
		else {
			string e="CASE "+expresion((*i).par1);
			bool comillas=false; int parentesis=0, j=0,l=e.size();
			while(j<l) {
				if (e[j]=='\''||e[j]=='\"') comillas=!comillas;
				else if (!comillas) {
					if (e[j]=='['||e[j]=='(') parentesis++;
					else if (e[j]==']'||e[j]==')') parentesis--;
					else if (parentesis==0 && e[j]==',') {
						e.replace(j,1,", "); l+=6;
					}
				}
				j++;
			}
			insertar(prog,tabs+"\t"+e);
		}
		bloque(prog,++i,*p,tabs+"\t\t");
		++q;
	}
	insertar(prog,tabs+"END SELECT");
}

void QBasicExporter::repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"DO");
	bloque(prog,++r,q,tabs+"\t");
	if ((*q).nombre=="HASTAQUE")
		insertar(prog,tabs+"LOOP UNTIL "+expresion((*q).par1));
	else
		insertar(prog,tabs+"LOOP WHILE "+expresion((*q).par1));
}

void QBasicExporter::para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string var=expresion((*r).par1), ini=expresion((*r).par2), fin=expresion((*r).par3), paso=(*r).par4;
	if (paso=="1")
		insertar(prog,tabs+"FOR "+var+" = "+ini+" TO "+fin);
	else
		insertar(prog,tabs+"FOR "+var+" = "+ini+" TO "+fin+" STEP "+expresion(paso));
	bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"NEXT "+var);
}

void QBasicExporter::paracada(t_output &out, t_proceso_it r, t_proceso_it q, string tabs){
	string var=ToLower((*r).par2), aux=ToLower((*r).par1);
	const int *dims=memoria->LeerDims(var);
	int n=dims[0];
	
	string *auxvars=new string[n];
	for(int i=0;i<n;i++) auxvars[i]=get_aux_varname("aux_index_");
	
	string vname=var;
	for(int i=0;i<n;i++) { 
		string idx=auxvars[i];
		insertar(out,tabs+"FOR "+idx+" = LBOUND("+var+","+IntToStr(i+1)+") TO UBOUND("+var+","+IntToStr(i+1)+")");
		if (vname==var) vname+="("; else vname[vname.size()-1]=',';
		vname+=idx+")";
		tabs+="\t";
	}
	
	t_output aux_out;
	bloque(aux_out,++r,q,tabs);
	replace_var(aux_out,aux,vname);
	insertar_out(out,aux_out);
	
	for(int i=n-1;i>=0;i--) { 
		tabs=tabs.substr(0,tabs.size()-1);
		insertar(out,tabs+"NEXT "+auxvars[i]);
		release_aux_varname(auxvars[i]);
	}
	delete []auxvars;
}

string QBasicExporter::function(string name, string args) {
	if (name=="SEN") {
		return string("SIN")+args;
	} else if (name=="TAN") {
		return string("TAN")+args;
	} else if (name=="ASEN") {
		use_asin=true;
		return string("ARCSIN")+args;
	} else if (name=="ACOS") {
		use_acos=true;
		return string("ARCCOS")+args;
	} else if (name=="COS") {
		return string("COS")+args;
	} else if (name=="RAIZ"||name=="RC") {
		return string("SQR")+args;
	} else if (name=="ABS") {
		return string("ABS")+args;
	} else if (name=="LN") {
		return string("LOG")+args;
	} else if (name=="EXP") {
		return string("EXP")+args;
	} else if (name=="AZAR") {
		use_rand=true;
		return string("INT(RND*")+get_arg(args,1)+")";
	} else if (name=="ATAN") {
		return string("ATN")+args;
	} else if (name=="TRUNC") {
		return string("INT")+args;
	} else if (name=="REDON") {
		return string("CINT")+args;
	} else if (name=="CONCATENAR") {
		return get_arg(args,1)+"+"+get_arg(args,2);
	} else if (name=="LONGITUD") {
		return string("LEN")+args;
	} else if (name=="SUBCADENA") {
		if (!input_base_zero_arrays) args=
			string("(")+get_arg(args,1)+","
			+sumarOrestarUno(get_arg(args,2),false)
			+","+sumarOrestarUno(get_arg(args,3),false)+")";
		return string("MID$")+args;
	} else if (name=="CONVERTIRANUMERO") {
		return string("VAL")+args;
	} else if (name=="CONVERTIRATEXTO") {
		return string("STR$")+args;
	} else if (name=="MINUSCULAS") {
		return string("LCASE$")+args;
	} else if (name=="MAYUSCULAS") {
		return string("UCASE$")+args;
	} else {
		return ToLower(name)+args; // no deberia pasar esto
	}
}

// funcion usada por declarar_variables para las internas de una funcion
// y para obtener los tipos de los argumentos de la funcion para las cabeceras
string QBasicExporter::get_tipo(map<string,tipo_var>::iterator &mit, bool for_func, bool by_ref) {
	tipo_var &t=mit->second;
	string stipo="STRING";
	if (t==vt_caracter) { stipo="STRING"; }
	else if (t==vt_numerica) stipo=t.rounded?"LONG":"DOUBLE";
	else if (t==vt_logica) stipo="INTEGER";
	if (t.dims) {
		if (!for_func) return "";
		string pre=for_func?"":"DIM ";
		return pre+ToLower(mit->first)+make_dims(t.dims,"(",",",")",!for_func)+" AS "+stipo;
	} else {
		string pre=for_func?"":"DIM ";
		return pre+ToLower(mit->first)+" AS "+stipo;
	}
}

// resolucion de tipos (todo lo que acceda a cosas privadas de memoria tiene que estar en esta clase porque es la unica amiga)
void QBasicExporter::declarar_variables(t_output &prog) {
	map<string,tipo_var>::iterator mit=memoria->GetVarInfo().begin(), mit2=memoria->GetVarInfo().end();
	while (mit!=mit2) {
		string dec=get_tipo(mit);
		if (dec.size()) prog.push_back(dec);
		++mit;
	}
}

// retorna el tipo y elimina de la memoria a esa variable
// se usa para armar las cabeceras de las funciones, las elimina para que no se
// vuelvan a declarar adentro
string QBasicExporter::get_tipo(string name, bool by_ref) {
	map<string,tipo_var>::iterator mit=memoria->GetVarInfo().find(name);
	if (mit==memoria->GetVarInfo().end()) 
		return string("DIM ")+ToLower(name)+" AS STRING"; // puede pasar si hay variables que no se usan dentro de la funcion
	string ret = get_tipo(mit,true,by_ref);
	memoria->GetVarInfo().erase(mit);
	return ret;
}

void QBasicExporter::translate_single_proc(t_output &out, Funcion *f, t_proceso &proc) {
	
	//cuerpo del proceso
	t_output out_proc;
	bloque(out_proc,++proc.begin(),proc.end(),"");
	
	// cabecera del proceso
	bool is_sub=true;
	string ret; // sentencia "Return ..." de la funcion
	if (f) {
		string dec;
		if (f->nombres[0]=="") {
			dec="SUB "; 
		} else {
			is_sub=false;
			dec="FUNCTION ";
			ret=ToLower(f->id)+" = "+ToLower(f->nombres[0]);
		}
		dec+=ToLower(f->id)+" (";
		for(int i=1;i<=f->cant_arg;i++) {
			if (i!=1) dec+=", ";
			dec+=get_tipo(f->nombres[i],f->pasajes[i]==PP_REFERENCIA);
		}
		dec+=")";
		out.push_back("");
		out.push_back(dec);
	}
	
	declarar_variables(out);
	
	insertar_out(out,out_proc);
	
	// cola del proceso
	if (ret.size()) out.push_back(ret);
	if (f) out.push_back(is_sub?"END SUB":"END FUNCTION");
	if (!for_test) out.push_back("");
}

void QBasicExporter::translate(t_output &out, t_programa &prog) {
	
	// cppcheck-suppress unusedScopedObject
	TiposExporter(prog,false); // para que se cargue el mapa_memorias con memorias que tengan ya definidos los tipos de variables que correspondan
	
	// cabecera
	init_header(out,"' ");
	if (!for_test) out.push_back("");
	// procesos y subprocesos
	t_output aux;
	translate_all_procs(aux,prog);
	
	if (use_pi) out.push_back("CONST PI = 3.141593");
	if (use_rand) out.push_back("RANDOMIZE TIMER");
	insertar_out(out,aux);
	
	if (use_asin) {
		out.push_back("");
		out.push_back("FUNCTION ARCSIN(x)");
		out.push_back("	IF x <= 1 AND x >= -1 THEN");
		out.push_back("		ARCSIN = ATN(x / SQR(1 - (x * x)))");
		out.push_back("	ELSE");
		out.push_back("		PRINT \"Domain Error\"");
		out.push_back("		END");
		out.push_back("	END IF");
		out.push_back("END FUNCTION");
	}
	if (use_acos) {
		out.push_back("");
		out.push_back("FUNCTION ARCCOS(x)");
		out.push_back("	IF x <= 1 AND x >= -1 THEN");
		out.push_back("		ARCCOS = (2 * ATN(1)) - ATN(x / SQR(1 - x * x))");
		out.push_back("	ELSE");
		out.push_back("		PRINT \"Domain Error\"");
		out.push_back("		END");
		out.push_back("	END IF");
		out.push_back("END FUNCTION  ");
	}
}

string QBasicExporter::get_constante(string name) {
	if (name=="PI") { use_pi=true; return "PI"; }
	if (name=="VERDADERO") return "1";
	if (name=="FALSO") return "0";
	return name;
}

string QBasicExporter::get_operator(string op, bool for_string) {
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
	if (op=="+") return "+"; 
	if (op=="-") return "-"; 
	if (op=="/") return "/"; 
	if (op=="*") return "*";
	if (op=="^") return "^";
	if (op=="%") return " MOD ";
	if (op=="=") return "="; 
	if (op=="<>") return "<>"; 
	if (op=="<") return "<"; 
	if (op==">") return ">"; 
	if (op=="<=") return "<="; 
	if (op==">=") return ">="; 
	if (op=="&") return " AND "; 
	if (op=="|") return " OR "; 
	if (op=="~") return "NOT "; 
	return op; // no deberia pasar nunca
}

string QBasicExporter::make_string (string cont) {
	return string("\"")+cont+"\"";
}

void QBasicExporter::dimension(t_output &prog, t_arglist &args, string tabs) {
	ExporterBase::dimension(prog,args,tabs);
	t_arglist_it it=args.begin();
	string line;
	while (it!=args.end()) {
		// obtener nombre y dimensiones
		string name, dims; 
		crop_name_and_dims(*it,name,dims,
			input_base_zero_arrays?"(0 TO ":"(1 TO ",
			input_base_zero_arrays?"-1, 0 TO":", 1 TO ",
			input_base_zero_arrays?"-1)":")");
		// armar la linea que hace el dim
		tipo_var t = memoria->LeerTipo(name);
		string stipo="STRING";
		if (t==vt_caracter) { stipo="STRING"; }
		else if (t==vt_numerica) stipo=t.rounded?"LONG":"DOUBLE";
		else if (t==vt_logica) stipo="INTEGER";
		if (line!="") line+=", "; line+=ToLower(name)+dims+" AS "+stipo;
		++it;
	}
	insertar(prog,tabs+"DIM "+line);
}

void QBasicExporter::comentar (t_output & prog, string text, string tabs) {
	if (!for_test) insertar(prog,tabs+"' "+text);
}

