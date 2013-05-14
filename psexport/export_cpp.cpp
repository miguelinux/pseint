#include "export_cpp.h"
#include <sstream>
#include <cstdlib>
#include "../pseint/new_evaluar.h"
#include "../pseint/utils.h"
#include "version.h"
#include "new_memoria.h"
#include "exportexp.h"
#include "new_funciones.h"
using namespace std;

static bool include_cmath=false;
static bool include_cstdlib=false;
static bool use_sin_tipo=false;
static bool use_string=false;
static bool use_func_minusculas=false;
static bool use_func_mayusculas=false;
static bool use_func_convertiratexto=false;
t_output prototipos; // forward declarations de las funciones

// retorna true para un objeto string (variable o expresion), y falso para un cstring (constante de tipo cadena)
static bool esString(const string &s) {
	return s[0]=='\''||s[0]=='\"';
}
static string convertirAString(const string &s) {
	if (esString(s)) return string("string("+s+")");
	else return s;
}

// recibe los argumentos de una funcion (incluyendo los parentesis que los envuelven, y extra alguno (cual, base 1)
string get_arg(string args, int cual) {
	int i=1,i0=1,parentesis=0,n=0; bool comillas=false;
	while (true) {
		if (args[i]=='\''||args[i]=='\"') comillas=!comillas;
		else if (!comillas) {
			if (parentesis==0 && (args[i]==','||args[i]==')')) {
				if (++n==cual) { return args.substr(i0,i-i0); }
				i0=i+1;
			} 
			else if (args[i]=='('||args[i]=='[') parentesis++;
			else if (args[i]==')'||args[i]==']') parentesis--;
		}
		i++;
	}
	return "";
}

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
		return string("(rand()%")+colocarParentesis(get_arg(args,1))+")";
	} else if (name=="atan") {
		include_cmath=true;
		return string("atan")+args;
	} else if (name=="trunc") {
		include_cmath=true;
		return string("floor")+args;
	} else if (name=="redon") {
		include_cmath=true;
		return string("floor(")+colocarParentesis(get_arg(args,1))+".5)";
	} else if (name=="concatenar") {
		return string("(")+convertirAString(get_arg(args,1))+"+"+get_arg(args,2)+")";
	} else if (name=="longitud") {
		return convertirAString(get_arg(args,1))+".size()";
	} else if (name=="subcadena") {
		return convertirAString(get_arg(args,1))+".substr("+get_arg(args,2)+","+get_arg(args,3)+"-"+get_arg(args,2)+"+1)";
	} else if (name=="convertiranumero") {
		include_cstdlib=true;
		string s=get_arg(args,1);
		if (esString(s)) return string("atof(")+s+")";
		else return string("atof(")+colocarParentesis(s)+".c_str())";
	} else {
		if (name=="minusculas") use_func_minusculas=true;
		if (name=="mayusculas") use_func_mayusculas=true;
		if (name=="convertiratexto") use_func_convertiratexto=true;
		return name+args; // no deberia pasar esto
	}
}

// resolucion de tipos (todo lo que acceda a cosas privadas de memoria tiene que estar en esta clase porque es la unica amiga)
class MemoriaForExport {
public:
	static void declarar_variables(t_output &prog) {
		map<string,tipo_var>::iterator mit=memoria->var_info.begin(), mit2=memoria->var_info.end();
		string tab("\t"),stipo;
		while (mit!=mit2) {
			prog.push_back(tab+get_tipo(mit)+";");
			mit++;
		}
	}
	
	// retorna el tipo y elimina de la memoria a esa variable
	// se usa para armar las cabeceras de las funciones, las elimina para que no se
	// vuelvan a declarar adentro
	static string get_tipo(string name) {
		map<string,tipo_var>::iterator mit=memoria->var_info.find(name);
		if (mit==memoria->var_info.end()) 
			return "SIN_TIPO _variable_desconocida_";
		string ret = get_tipo(mit);
		memoria->var_info.erase(mit);
		return ret;
	}
	
	// funcion usada por declarar_variables para las internas de una funcion
	// y para obtener los tipos de los argumentos de la funcion para las cabeceras
	static string get_tipo(map<string,tipo_var>::iterator &mit) {
		tipo_var &t=mit->second;
		string stipo="SIN_TIPO ";
		if (t==vt_caracter) { stipo="string "; use_string=true; }
		else if (t==vt_numerica) stipo=t.rounded?"int ":"float ";
		else if (t==vt_logica) stipo="bool ";
		else use_sin_tipo=true;
		if (t.dims) {
			string dims="[", c2="][", c3="]";
			for (int j=1;j<=t.dims[0];j++) {
				if (j==t.dims[0]) dims+=IntToStr(t.dims[j])+c3;
				else dims+=IntToStr(t.dims[j])+c2;
			}
			return stipo+ToLower(mit->first)+dims;
		} else {
			return stipo+ToLower(mit->first);
		}
	}
	
};

void i_invocar(t_output &prog, string param,string tabs);
void i_esperar(t_output &prog, string param,string tabs);
void i_borrar(t_output &prog, string param,string tabs);
void i_escribir(t_output &prog, string param,string tabs);
void i_leer(t_output &prog, string param,string tabs);
void i_secuencial(t_output &prog, string param,string tabs);
void i_asignacion(t_output &prog, string param1, string param2,string tabs);
void i_si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s,string tabs);
void i_mientras(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs);
void i_segun(t_output &prog, list<t_proceso_it> its,string tabs);
void i_repetir(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs);
void i_paracada(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs);
void i_para(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs);
void i_bloque(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs);
void i_dimension(t_output &prog, string params, string tabs);

void i_dimension(t_output &prog, string params, string tabs) {
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

void i_esperar(t_output &prog, string param, string tabs){
	insertar(prog,tabs+"cin.get();");
}

void i_borrar(t_output &prog, string param, string tabs){
	insertar(prog,tabs+"cout<<endl; // no hay forma directa de borrar la pantalla con C++ estandar");
}

void i_invocar(t_output &prog, string param, string tabs){
	string linea=expresion(param);
	if (linea[linea.size()-1]!=')') 
		linea+="()";
	linea+=";";
	insertar(prog,tabs+linea);
}

void i_escribir(t_output &prog, string param, string tabs){
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

void i_leer(t_output &prog, string param, string tabs){
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

void i_definir(t_output &prog, string param, string tabs){
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

void i_asignacion(t_output &prog, string param1, string param2, string tabs){
	tipo_var t;
	param1=expresion(param1);
	param2=expresion(param2,t);
	insertar(prog,tabs+param1+"="+param2+";");
	memoria->DefinirTipo(param1,t);
}

void i_si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs){
	insertar(prog,tabs+"if ("+expresion((*r).par1)+") {");
	i_bloque(prog,++r,q,tabs+"\t");
	if (q!=s) {
		insertar(prog,tabs+"} else {");
		i_bloque(prog,++q,s,tabs+"\t");
	}
	insertar(prog,tabs+"}");
}

void i_mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"while ("+expresion((*r).par1)+") {");
	i_bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"}");
}

void i_segun(t_output &prog, list<t_proceso_it> its, string tabs){
	list<t_proceso_it>::iterator p,q,r;
	q=p=its.begin();r=its.end();
	t_proceso_it i=*q;
	string opcion=expresion((*i).par1); int p1=0, p2=opcion.size()-1;
	AplicarTipo(opcion,p1,p2,vt_numerica);
	insertar(prog,tabs+"switch (int("+expresion((*i).par1)+")) {");
	q++;p++;
	while (++p!=r) {
		i=*q;
		if ((*i).par1=="DE OTRO MODO")
			insertar(prog,tabs+"default:");
		else {
			string e="case "+expresion((*i).par1)+":";
			bool comillas=false; int parentesis=0, j=0,l=e.size();
			while(j<l) {
				if (e[j]=='\''||e[j]=='\"') comillas=!comillas;
				else if (!comillas) {
					if (e[j]=='['||e[j]=='(') parentesis++;
					else if (e[j]==']'||e[j]==')') parentesis--;
					else if (parentesis==0 && e[j]==',') {
						e.replace(j,1,": case ");
					}
				}
				j++;
			}
			insertar(prog,tabs+e);
		}
		i_bloque(prog,++i,*p,tabs+"\t");
		insertar(prog,tabs+"\tbreak;");
		q++;
	}
	insertar(prog,tabs+"}");
}

void i_repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"do {");
	i_bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"} while ("+invert_expresion(expresion((*q).par1))+");");
}

void i_para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	
	memoria->DefinirTipo(ToLower((*r).par1),vt_numerica);
	
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

void i_paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string var=ToLower((*r).par2), aux=ToLower((*r).par1);
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
void i_bloque(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs){
	if (r==q) return;
	int deep;
	string s;
	
	while (r!=q) {
		s=(*r).nombre;
		if (s=="ESCRIBIR") i_escribir(prog,(*r).par1,tabs);
		else if (s=="INVOCAR") i_invocar(prog,(*r).par1,tabs);
		else if (s=="BORRARPANTALLA") i_borrar(prog,(*r).par1,tabs);
		else if (s=="ESPERARTECLA") i_esperar(prog,(*r).par1,tabs);
		else if (s=="DIMENSION") i_dimension(prog,(*r).par1,tabs);
		else if (s=="DEFINIR") i_definir(prog,(*r).par1,tabs);
		else if (s=="LEER") i_leer(prog,(*r).par1,tabs);
		else if (s=="ASIGNACION") i_asignacion(prog,(*r).par1,(*r).par2,tabs);
		else if (s=="MIENTRAS") {
			t_proceso_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINMIENTRAS"  && deep==0) ) {
				if ((*r).nombre=="MIENTRAS") deep++;
				else if ((*r).nombre=="FINMIENTRAS") deep--;
				r++;
			}
			i_mientras(prog,r1,r,tabs);
		} else if (s=="REPETIR") {
			t_proceso_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="HASTAQUE"  && deep==0) ) {
				if ((*r).nombre=="REPETIR") deep++;
				else if ((*r).nombre=="HASTAQUE") deep--;
				r++;
			}
			i_repetir(prog,r1,r,tabs);
		} else if (s=="SEGUN") {
			list<t_proceso_it> its;
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
			t_proceso_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINPARA" && deep==0) ) {
				if ((*r).nombre=="PARA"||(*r).nombre=="PARACADA") deep++;
				else if ((*r).nombre=="FINPARA") deep--;
				r++;
			}
			i_para(prog,r1,r,tabs);
		} else if (s=="PARACADA") {
			t_proceso_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINPARA" && deep==0) ) {
				if ((*r).nombre=="PARA"||(*r).nombre=="PARACADA") deep++;
				else if ((*r).nombre=="FINPARA") deep--;
				r++;
			}
			i_paracada(prog,r1,r,tabs);
		} else if (s=="SI") {
			t_proceso_it r1=r++,r2;
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

void cpp_cabecera(t_output &out) {
	// cabecera
	stringstream version; 
	version<<VERSION<<"-"<<ARCHITECTURE;
	out.push_back(string("// Este codigo ha sido generado por el modulo psexport ")+version.str()+" de PSeInt");
	out.push_back("// dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental");
	out.push_back("// puede que el codigo generado no sea completamente correcto. Si encuentra");
	out.push_back("// errores por favor reportelos en el foro (http://pseint.sourceforge.net).");
	out.push_back("");
	out.push_back("#include<iostream>");
	if (include_cmath) out.push_back("#include<cmath>");
	if (include_cstdlib) out.push_back("#include<cstdlib>");
	if (use_func_mayusculas||use_func_minusculas) out.push_back("#include<cctype>");
	if (use_func_convertiratexto) out.push_back("#include<stringstream>");
	out.push_back("using namespace std;");
	out.push_back("");
	if (use_func_convertiratexto) {
		out.push_back("// No hay en el C++ estandar una funcion equivalente a \"convertiratexto\".");
		out.push_back("#include<stringstream>");
		out.push_back("string convertiratexto(float f) {");
		out.push_back("\tstringstream ss;");
		out.push_back("\tss<<f;");
		out.push_back("\treturn ss.str();");
		out.push_back("}");
		out.push_back("");
	}
	if (use_func_mayusculas) {
		out.push_back("// No hay en el C++ estandar una funcion equivalente a \"mayusculas\".");
		out.push_back("string mayusculas(string s) {");
		out.push_back("\tfor(unsigned int i=0;i<s.size();i++)");
		out.push_back("\t\ts[i]=toupper(s[i]);");
		out.push_back("\treturn s;");
		out.push_back("}");
		out.push_back("");
	}
	if (use_func_minusculas) {
		out.push_back("// No hay en el C++ estandar una funcion equivalente a \"minusculas\".");
		out.push_back("string minusculas(string s) {");
		out.push_back("\tfor(unsigned int i=0;i<s.size();i++)");
		out.push_back("\t\ts[i]=tolower(s[i]);");
		out.push_back("\treturn s;");
		out.push_back("}");
		out.push_back("");
	}
	if (use_sin_tipo) {
		out.push_back("// Para las variables que no se pudo determinar el tipo se utiliza la constante");
		out.push_back("// SIN_TIPO. El usuario debe reemplazar sus ocurrencias por el tipo adecuado");
		out.push_back("// (usualmente int,float,string o bool).");
		out.push_back("#define SIN_TIPO string");
		out.push_back("");
	}
	if (use_string) {
		out.push_back("// Para leer variables de texto se utiliza el operador << del objeto cin, que");
		out.push_back("// lee solo una palabra. Para leer una linea completa (es decir, incluyendo los");
		out.push_back("// espacios en blanco) se debe utilzar getline (ej, reemplazar cin>>x por");
		out.push_back("// getline(cin,x), pero obliga a agregar un cin.ignore() si antes del getline");
		out.push_back("// se leyó otra variable con >>.");
		out.push_back("");
	}
}


void translate_cpp(t_output &out, t_proceso &proc) {
	
	memoria=new Memoria(NULL);
	
	//cuerpo del proceso
	t_output out_proc;
	i_bloque(out_proc,++proc.begin(),proc.end(),"\t");
	
	// cabecera del proceso
	t_proceso_it it=proc.begin();
	string ret; // sentencia "return ..." de la funcion
	if (it->nombre=="PROCESO") {
		out.push_back("int main() {");
		ret="return 0";
	} else {
		int x;
		Funcion *f=ParsearCabeceraDeSubProceso(it->par1,false,x);
		string dec;
		if (f->nombres[0]=="") dec="void "; else {
			ret=MemoriaForExport::get_tipo(f->nombres[0]);
			dec=ret.substr(0,ret.find(" ")+1);
			ret=string("return")+ret.substr(ret.find(" "));
		}
		dec+=ToLower(f->id)+"(";
		for(int i=1;i<=f->cant_arg;i++) {
			if (i!=1) dec+=", ";
			dec+=MemoriaForExport::get_tipo(f->nombres[i]);
		}
		dec+=")";
		prototipos.push_back(dec+";");
		out.push_back(dec+" {");
		delete f;
	}
	
	MemoriaForExport::declarar_variables(out);
	
	copy(out_proc.begin(),out_proc.end(),back_inserter(out));
	
	// cola del proceso
	if (ret.size()) out.push_back(string("\t")+ret+";");
	out.push_back("}");
	out.push_back("");
	
	delete memoria;
	
}

void translate_cpp(t_output &out, t_programa &prog) {
	t_output aux;
	for (t_programa_it it=prog.begin();it!=prog.end();++it)
		translate_cpp(aux,*it);	
	cpp_cabecera(out);
	if (prototipos.size()) {
		out.push_back("// forward declarations");
		copy(prototipos.begin(),prototipos.end(),back_inserter(out));
		out.push_back("");
	}
	copy(aux.begin(),aux.end(),back_inserter(out));
}

