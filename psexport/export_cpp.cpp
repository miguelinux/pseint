#include "export_cpp.h"
#include <sstream>
#include <cstdlib>
#include "../pseint/new_evaluar.h"
#include "../pseint/utils.h"
#include "version.h"
#include "new_memoria.h"
#include "exportexp.h"
using namespace std;

#define MAIN_LINE_TEXT "int main() {"
#define USING_NAMESPACE_TEXT "using namespace std;"

bool include_cmath=false;
bool include_cstdlib=false;

string cpp_function(string name, string args) {
	if (name=="sen") {
		include_cmath=true;
		return string("sin")+args;
	} else if (name=="tan") {
		include_cmath=true;
		return string("tan")+args;
	} else if (name=="asen") {
		include_cmath=true;
		return string("asin")+args;
	} else if (name=="acos") {
		include_cmath=true;
		return string("acos")+args;
	} else if (name=="cos") {
		include_cmath=true;
		return string("cos")+args;
	} else if (name=="raiz") {
		include_cmath=true;
		return string("sqrtf")+args;
	} else if (name=="rc") {
		include_cmath=true;
		return string("sqrtf")+args;
	} else if (name=="abs") {
		include_cmath=true;
		return string("abs")+args;
	} else if (name=="ln") {
		include_cmath=true;
		return string("log")+args;
	} else if (name=="exp") {
		include_cmath=true;
		return string("exp")+args;
	} else if (name=="azar") {
		include_cstdlib=true;
		return string("(rand()%")+colocarParentesis(args.substr(1,args.size()-2))+")";
	} else if (name=="atan") {
		include_cmath=true;
		return string("atan")+args;
	} else if (name=="trunc") {
		include_cmath=true;
		return string("floor")+args;
	} else if (name=="redon") {
		include_cmath=true;
		return string("floor(")+colocarParentesis(args.substr(1,args.size()-2))+".5)";
	} else 
		return name+args; // no deberia pasar esto
}

// resolucion de tipos
void declarar_variables(t_programa &prog, bool &use_sin_tipo, bool &use_string) {
	
	// saltear comentarios??
	t_programa::iterator it = prog.begin();
	while (*it!=MAIN_LINE_TEXT) it++; it++; it++;
	
	map<string,tipo_var>::iterator mit=memoria->var_info.begin(), mit2=memoria->var_info.end();
	string tab("\t"),dims, c1="[", c2="]", stipo;
	
	use_sin_tipo=false;
	use_string=false;
	
	while (mit!=mit2) {
		tipo_var &t=mit->second;
		stipo="SIN_TIPO ";
		if (t==vt_caracter) { stipo="string "; use_string=true; }
		else if (t==vt_numerica) stipo=t.rounded?"int ":"float ";
		else if (t==vt_logica) stipo="bool ";
		else use_sin_tipo=true;
		if (t.dims) {
			dims="";
			for (int j=1;j<=t.dims[0];j++)
				dims+=c1+IntToStr(t.dims[j])+c2;
			prog.insert(it,tab+stipo+toLower(mit->first)+dims+";");
		} else {
			prog.insert(it,tab+stipo+toLower(mit->first)+";");
		}
		mit++;
	}
}

void i_esperar(t_programa &prog, string param,string tabs);
void i_borrar(t_programa &prog, string param,string tabs);
void i_escribir(t_programa &prog, string param,string tabs);
void i_leer(t_programa &prog, string param,string tabs);
void i_secuencial(t_programa &prog, string param,string tabs);
void i_asignacion(t_programa &prog, string param1, string param2,string tabs);
void i_si(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q, t_algoritmo_it s,string tabs);
void i_mientras(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q,string tabs);
void i_segun(t_programa &prog, list<t_algoritmo_it> its,string tabs);
void i_repetir(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q,string tabs);
void i_paracada(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q,string tabs);
void i_para(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q,string tabs);
void i_bloque(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q,string tabs);
void i_dimension(t_programa &prog, string params, string tabs);

void i_dimension(t_programa &prog, string params, string tabs) {
	params=params+",";
	while (params.size()>1) {
		unsigned int i=1;
		while (params[i]!=')') i++;
		while (params[i]!=',') i++;
		string arr = params.substr(0,i);
		params.erase(0,i+1);
		i=0;
		while(i<arr.size())
			if (arr[i]==' ' || arr[i]=='\t')
				arr.erase(i,1);
			else 
				i++;
		arr[arr.size()-1]=',';
		unsigned int p=arr.size(), c=0, pars=0;
		for (i=0;i<arr.size();i++)
			if (arr[i]=='(' && pars==0) { 
				pars++;
				if (i<p) p=i;
			} else if (arr[i]==')') {
				pars--;
			} else if (arr[i]==',' && pars==1) {
				c++;
			} else if (arr[i]>='A' && arr[i]<='Z') {
				arr[i]=tolower(arr[i]);
			}
		int *dims=new int[c+1];
		string nom=arr.substr(0,p);
		dims[0]=c;
		int f=++p; c=1; pars=0;
		while (p<arr.size()) {
			if (arr[p]=='(')
				pars++;
			else if (arr[p]==')')
				pars++;
			else if (arr[p]==',' && pars==0) {
				tipo_var ch;
				dims[c++]=atoi(Evaluar(arr.substr(f,p-f),ch).c_str());
				f=p+1;
			}
			p++;
		}
		memoria->AgregarArreglo(nom,dims);
	}
}

void i_esperar(t_programa &prog, string param, string tabs){
	insertar(prog,tabs+"cin.get();");
}

void i_borrar(t_programa &prog, string param, string tabs){
	insertar(prog,tabs+"cout<<endl; // no hay forma directa de borrar la pantalla con C++ estandar");
}

void i_escribir(t_programa &prog, string param, string tabs){
	bool comillas=false, saltar=true;
	int parentesis=0;
	string linea="cout",expr;
	int lastcoma=0;
	for (unsigned int i=0;i<param.size();i++) {
		if (param[i]=='\'') {
			comillas=!comillas;
			param[i]='"';
		} else if (!comillas) {
			if (param[i]=='(') parentesis++;
			else if (param[i]==')') parentesis--;
			else if (parentesis==0 && param[i]==',') {
				expr=param.substr(lastcoma,i-lastcoma);
				if (expr=="**SINSALTAR**") saltar=false;
				else
					linea+=string("<<")+expresion(expr);
				lastcoma=i+1;
			}
		}
	}
	expr=param.substr(lastcoma,param.size()-lastcoma);
	if (expr=="**SINSALTAR**") saltar=false;
	else linea+=string("<<")+expresion(expr);
	if (saltar) linea+="<<endl;"; else linea+=";";
	insertar(prog,tabs+linea);
}

void i_leer(t_programa &prog, string param, string tabs){
	param+=",";
	bool comillas=false;
	int parentesis=0;
	string linea="cin";
	int lastcoma=0;
	for (unsigned int i=0;i<param.size();i++) {
		if (param[i]=='\'') {
			comillas=!comillas;
			param[i]='"';
		} else if (!comillas) {
			if (param[i]=='(') parentesis++;
			else if (param[i]==')') parentesis--;
			else if (parentesis==0 && param[i]==',') {
				string varname=expresion(param.substr(lastcoma,i-lastcoma));
				linea+=">>";
				linea+=varname;
				if (varname.find('[')==string::npos) {
					tipo_var t;
					memoria->DefinirTipo(varname,t);
				}
				lastcoma=i+1;
			}
		}
	}
	insertar(prog,tabs+linea+";");
}

void i_definir(t_programa &prog, string param, string tabs){
	int lastcoma=0;
	char tipo='*';
	if (param.size()>12 && param.substr(param.size()-12,12)==" COMO ENTERO") { tipo='i'; param=param.substr(0,param.size()-12); }
	if (param.size()>10 && param.substr(param.size()-10,10)==" COMO REAL") { tipo='f'; param=param.substr(0,param.size()-10); }
	if (param.size()>14 && param.substr(param.size()-14,14)==" COMO CARACTER") { tipo='s'; param=param.substr(0,param.size()-14); }
	if (param.size()>12 && param.substr(param.size()-12,12)==" COMO LOGICO") { tipo='b'; param=param.substr(0,param.size()-12); }
	param+=",";
	for (unsigned int i=0;i<param.size();i++) {
		if (param[i]==',') {
			string varname=expresion(param.substr(lastcoma,i-lastcoma));
			if (tipo=='i') memoria->DefinirTipo(varname,vt_numerica,true);
			else if (tipo=='f') memoria->DefinirTipo(varname,vt_numerica,false);
			else if (tipo=='s') memoria->DefinirTipo(varname,vt_caracter,false);
			else if (tipo=='b') memoria->DefinirTipo(varname,vt_logica,false);
			lastcoma=i+1;
		}
	}
}

void i_asignacion(t_programa &prog, string param1, string param2, string tabs){
	tipo_var t;
	param1=expresion(param1);
	param2=expresion(param2,t);
	insertar(prog,tabs+param1+"="+param2+";");
	memoria->DefinirTipo(param1,t);
}

void i_si(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q, t_algoritmo_it s, string tabs){
	insertar(prog,tabs+"if ("+expresion((*r).par1)+") {");
	i_bloque(prog,++r,q,tabs+"\t");
	if (q!=s) {
		insertar(prog,tabs+"} else {");
		i_bloque(prog,++q,s,tabs+"\t");
	}
	insertar(prog,tabs+"}");
}

void i_mientras(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q, string tabs){
	insertar(prog,tabs+"while ("+expresion((*r).par1)+") {");
	i_bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"}");
}

void i_segun(t_programa &prog, list<t_algoritmo_it> its, string tabs){
	list<t_algoritmo_it>::iterator p,q,r;
	q=p=its.begin();r=its.end();
	t_algoritmo_it i=*q;
	string opcion=expresion((*i).par1); int p1=0, p2=opcion.size()-1;
	AplicarTipo(opcion,p1,p2,vt_numerica);
	insertar(prog,tabs+"switch (int("+expresion((*i).par1)+")) {");
	q++;p++;
	while (++p!=r) {
		i=*q;
		if ((*i).par1=="DE OTRO MODO")
			insertar(prog,tabs+"default:");
		else
			insertar(prog,tabs+"case "+expresion((*i).par1)+":");
		i_bloque(prog,++i,*p,tabs+"\t");
		insertar(prog,tabs+"\tbreak;");
		q++;
	}
	insertar(prog,tabs+"}");
}

void i_repetir(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q, string tabs){
	insertar(prog,tabs+"do {");
	i_bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"} while ("+invert_expresion(expresion((*q).par1))+");");
}

void i_para(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q, string tabs){
	
	memoria->DefinirTipo(toLower((*r).par1),vt_numerica);
	
	string var=expresion((*r).par1), ini=expresion((*r).par2), fin=expresion((*r).par3), paso=(*r).par4;
	if ((*r).par4[0]=='-') {
//		if (lang_cpp) {
			if (paso=="-1")
				insertar(prog,tabs+"for ("+var+"="+ini+";"+var+">="+fin+";"+var+"--) {");
			else
				insertar(prog,tabs+"for ("+var+"="+ini+";"+var+">="+fin+";"+var+"-="+expresion(paso.substr(1,paso.size()-1))+") {");
//		} else
//			insertar(prog,tabs+"for ("+var+"="+ini+";"+var+">="+fin+";"+var+"="+var+"-"+expresion(paso)+") {");
	} else {
//		if (lang_cpp) {
			if (paso=="1")
				insertar(prog,tabs+"for ("+var+"="+ini+";"+var+"<="+fin+";"+var+"++) {");
			else
				insertar(prog,tabs+"for ("+var+"="+ini+";"+var+"<="+fin+";"+var+"+="+expresion(paso)+") {");
//		} else
//			insertar(prog,tabs+"for ("+var+"="+ini+";"+var+"<="+fin+";"+var+"="+var+"+"+expresion(paso)+") {");
	}
	i_bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"}");
}

void i_paracada(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q, string tabs){
	string var=toLower((*r).par2), aux=toLower((*r).par1);
	string first=var,last=var,inc=var;
	int *dims=memoria->LeerDims(var);
	if (!dims) { insertar(prog,string("Error: ")+var+" no es un arreglo"); return; }
	for (int i=1;i<=dims[0];i++) {
		first+="[0]";
		if (i!=dims[0]) inc+="[0]";
		last+="[";
		last+=IntToStr(dims[i]-1);
		last+="]";
	}
	insertar(prog,tabs+"for (typeof(&("+first+")) ptr_aux=&("+first+");ptr_aux<=&("+last+");ptr_aux++) {");
	insertar(prog,tabs+"\ttypeof("+first+") &"+aux+"=*ptr_aux;");
	i_bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"}");
}

//-----------------------++;
void i_bloque(t_programa &prog, t_algoritmo_it r, t_algoritmo_it q,string tabs){
	if (r==q) return;
	int deep;
	string s;
	
	while (r!=q) {
		s=(*r).nombre;
		if (s=="ESCRIBIR") i_escribir(prog,(*r).par1,tabs);
		else if (s=="BORRARPANTALLA") i_borrar(prog,(*r).par1,tabs);
		else if (s=="ESPERARTECLA") i_esperar(prog,(*r).par1,tabs);
		else if (s=="DIMENSION") i_dimension(prog,(*r).par1,tabs);
		else if (s=="DEFINIR") i_definir(prog,(*r).par1,tabs);
		else if (s=="LEER") i_leer(prog,(*r).par1,tabs);
		else if (s=="ASIGNACION") i_asignacion(prog,(*r).par1,(*r).par2,tabs);
		else if (s=="MIENTRAS") {
			t_algoritmo_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINMIENTRAS"  && deep==0) ) {
				if ((*r).nombre=="MIENTRAS") deep++;
				else if ((*r).nombre=="FINMIENTRAS") deep--;
				r++;
			}
			i_mientras(prog,r1,r,tabs);
		} else if (s=="REPETIR") {
			t_algoritmo_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="HASTAQUE"  && deep==0) ) {
				if ((*r).nombre=="REPETIR") deep++;
				else if ((*r).nombre=="HASTAQUE") deep--;
				r++;
			}
			i_repetir(prog,r1,r,tabs);
		} else if (s=="SEGUN") {
			list<t_algoritmo_it> its;
			its.insert(its.end(),r);
			deep=0;
			r++;
			while ( ! ((*r).nombre=="FINSEGUN" && deep==0) ) {
				if (deep==0 && (*r).nombre=="OPCION") 
					its.insert(its.end(),r);
				else if ((*r).nombre=="SEGUN") 
					deep++;
				else if ((*r).nombre=="FINSEGUN")
					deep--;
				r++;
			}
			its.insert(its.end(),r);
			i_segun(prog,its,tabs);
		} else if (s=="PARA") {
			t_algoritmo_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINPARA" && deep==0) ) {
				if ((*r).nombre=="PARA"||(*r).nombre=="PARACADA") deep++;
				else if ((*r).nombre=="FINPARA") deep--;
				r++;
			}
			i_para(prog,r1,r,tabs);
		} else if (s=="PARACADA") {
			t_algoritmo_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINPARA" && deep==0) ) {
				if ((*r).nombre=="PARA"||(*r).nombre=="PARACADA") deep++;
				else if ((*r).nombre=="FINPARA") deep--;
				r++;
			}
			i_paracada(prog,r1,r,tabs);
		} else if (s=="SI") {
			t_algoritmo_it r1=r++,r2;
			r++;
			deep=0;
			while ( ! ( ( (*r).nombre=="FINSI" || (*r).nombre=="SINO") && deep==0) ) {
				if ((*r).nombre=="SI") deep++;
				else if ((*r).nombre=="FINSI") deep--;
				r++;
			}
			r2=r;
			while ( ! ( (*r).nombre=="FINSI" && deep==0) ) {
				if ((*r).nombre=="SI") deep++;
				else if ((*r).nombre=="FINSI") deep--;
				r++;
			}
			i_si(prog,r1,r2,r,tabs);
		}
		r++;
	}
}

void cpp_main(t_programa &prog, t_algoritmo &alg) {
	
	
	t_algoritmo_it r=alg.begin(),q=alg.end();
	
	// cabecera
	stringstream version; 
	version<<VERSION<<"-"<<ARCHITECTURE;
	prog.insert(prog.end(),string("// Este codigo ha sido generado por el modulo psexport ")+version.str()+" de PSeInt");
	prog.insert(prog.end(),"// dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental puede que el codigo generado no sea completamente correcto");
	prog.insert(prog.end(),"// si encuentra errores, por favor reportelos a zaskar_84@yahoo.com.ar");
	prog.insert(prog.end(),"");
	prog.insert(prog.end(),"#include<iostream>");
	prog.insert(prog.end(),USING_NAMESPACE_TEXT);
	prog.insert(prog.end(),"");
	prog.insert(prog.end(),MAIN_LINE_TEXT);
	prog.insert(prog.end(),"");
	prog.insert(prog.end(),"");
	
	//cuerpo del proceso
	r++;
	i_bloque(prog,r,q,"\t");
	
	// cola
	prog.insert(prog.end(),"");
	prog.insert(prog.end(),"\treturn 0;");
	prog.insert(prog.end(),"");
	prog.insert(prog.end(),"}");
	prog.insert(prog.end(),"");
	
	bool use_sin_tipo, use_string;
	declarar_variables(prog,use_sin_tipo, use_string);
	if (include_cmath||include_cstdlib) {
		t_programa::iterator it=prog.begin();
		while (*it!=USING_NAMESPACE_TEXT) it++;
		if (include_cmath) it=prog.insert(it,"#include<cmath>");
		if (include_cstdlib) it=prog.insert(it,"#include<cstdlib>");
	}
	if (use_string||use_sin_tipo) {
		t_programa::iterator it=prog.begin();
		while (*it!=MAIN_LINE_TEXT) it++;
		if (use_sin_tipo) {
			it=prog.insert(it,"");
			it=prog.insert(it,"#define SIN_TIPO string");
			it=prog.insert(it,"// el usuario debe reemplazar sus ocurrencias por el tipo adecuado (usualmente int,float,string o bool)");
			it=prog.insert(it,"// para las variables que no se pudo determinar el tipo se utiliza la constante SIN_TIPO");
		}
		if (use_string) {
			it=prog.insert(it,"");
			it=prog.insert(it,"// pero obliga a agregar un cin.ignore() si antes del getline se leyó otra variable con >>.");
			it=prog.insert(it,"// para leer una linea completa se debe utilzar getline (ej, reemplazar cin>>x por getline(cin,x),");
			it=prog.insert(it,"// para leer variables de texto se utiliza el operador << del objeto cin, que lee solo una palabra");
		}
	}
	
}

