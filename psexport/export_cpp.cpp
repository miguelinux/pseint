#include "export_cpp.h"
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

CppExporter::CppExporter() {
	has_matrix_func=false;
	include_cmath=false;
	include_cstdlib=false;
	use_sin_tipo=false;
	use_string=false;
	use_func_esperar=false;
	use_func_minusculas=false;
	use_func_mayusculas=false;
	use_func_convertiratexto=false;
	base_zero_arrays=true;
	use_arreglo_max=false;
}

void CppExporter::borrar_pantalla(t_output &prog, string param, string tabs){
	if (for_testing)
		insertar(prog,tabs+"cout<<endl;");
	else
		insertar(prog,tabs+"cout<<endl; // no hay forma directa de borrar la pantalla con C++ estandar");
}

void CppExporter::esperar_tecla(t_output &prog, string param, string tabs){
	if (for_testing)
		insertar(prog,tabs+"cin.get();");
	else
		insertar(prog,tabs+"cin.get(); // a diferencia del pseudocódigo, espera un Enter, no cualquier tecla");
}

void CppExporter::esperar_tiempo(t_output &prog, float tiempo, bool mili, string tabs) {
	use_func_esperar=true;
	stringstream inst;
	inst<<"esperar(";
	if (mili) inst<<tiempo; 
	else inst<<"("<<tiempo<<")*1000";
	inst<<");";
	insertar(prog,tabs+inst.str());
}

void CppExporter::invocar(t_output &prog, string param, string tabs){
	string linea=expresion(param);
	if (linea[linea.size()-1]!=')') 
		linea+="()";
		linea+=";";
		insertar(prog,tabs+linea);
}

void CppExporter::escribir(t_output &prog, t_arglist args, bool saltar, string tabs){
	t_arglist_it it=args.begin();
	string linea="cout";
	while (it!=args.end()) {
		linea+="<<";
		linea+=expresion(*it);
		it++;
	}
	insertar(prog,tabs+linea+(saltar?"<<endl;":";"));
}

void CppExporter::leer(t_output &prog, t_arglist args, string tabs) {
	t_arglist_it it=args.begin();
	string linea="cin";
	while (it!=args.end()) {
		linea+=">>";
		linea+=expresion(*it);
		it++;
	}
	insertar(prog,tabs+linea+";");
}

void CppExporter::asignacion(t_output &prog, string param1, string param2, string tabs){
	insertar(prog,tabs+param1+"="+param2+";");
}

void CppExporter::si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs){
	insertar(prog,tabs+"if ("+expresion((*r).par1)+") {");
	bloque(prog,++r,q,tabs+"\t");
	if (q!=s) {
		insertar(prog,tabs+"} else {");
		bloque(prog,++q,s,tabs+"\t");
	}
	insertar(prog,tabs+"}");
}

void CppExporter::mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"while ("+expresion((*r).par1)+") {");
	bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"}");
}

void CppExporter::segun(t_output &prog, list<t_proceso_it> its, string tabs){
	list<t_proceso_it>::iterator p,q,r;
	q=p=its.begin();r=its.end();
	t_proceso_it i=*q;
	string opcion=expresion((*i).par1); int p1=0, p2=opcion.size()-1;
	AplicarTipo((*i).par1,p1,p2,vt_numerica_entera);
	insertar(prog,tabs+"switch ("+expresion((*i).par1)+") {");
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
						e.replace(j,1,": case "); l+=6;
					}
				}
				j++;
			}
			insertar(prog,tabs+e);
		}
		bloque(prog,++i,*p,tabs+"\t");
		insertar(prog,tabs+"\tbreak;");
		q++;
	}
	insertar(prog,tabs+"}");
}

void CppExporter::repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	insertar(prog,tabs+"do {");
	bloque(prog,++r,q,tabs+"\t");
	if ((*q).nombre=="HASTAQUE")
		insertar(prog,tabs+"} while ("+invert_expresion(expresion((*q).par1))+");");
	else
		insertar(prog,tabs+"} while ("+expresion((*q).par1)+");");
}

void CppExporter::para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string var=expresion((*r).par1), ini=expresion((*r).par2), fin=expresion((*r).par3), paso=(*r).par4;
	if ((*r).par4[0]=='-') {
		if (paso=="-1")
			insertar(prog,tabs+"for ("+var+"="+ini+";"+var+">="+fin+";"+var+"--) {");
		else
			insertar(prog,tabs+"for ("+var+"="+ini+";"+var+">="+fin+";"+var+"-="+expresion(paso.substr(1,paso.size()-1))+") {");
	} else {
		if (paso=="1")
			insertar(prog,tabs+"for ("+var+"="+ini+";"+var+"<="+fin+";"+var+"++) {");
		else
			insertar(prog,tabs+"for ("+var+"="+ini+";"+var+"<="+fin+";"+var+"+="+expresion(paso)+") {");
	}
	bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"}");
}

void CppExporter::paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string var=ToLower((*r).par2), aux=ToLower((*r).par1);
	string first=var,last=var,inc=var;
	const int *dims=memoria->LeerDims(var);
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
	bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"}");
}


// retorna true para un objeto string (variable o expresion), y falso para un cstring (constante de tipo cadena)
bool esString(const string &s) {
	return s[0]=='\''||s[0]=='\"';
}
string convertirAString(const string &s) {
	if (esString(s)) return string("string("+s+")");
	else return s;
}

string CppExporter::function(string name, string args) {
	if (name=="SEN") {
		include_cmath=true;
		return string("sin")+args;
	} else if (name=="TAN") {
		include_cmath=true;
		return string("tan")+args;
	} else if (name=="ASEN") {
		include_cmath=true;
		return string("asin")+args;
	} else if (name=="ACOS") {
		include_cmath=true;
		return string("acos")+args;
	} else if (name=="COS") {
		include_cmath=true;
		return string("cos")+args;
	} else if (name=="RAIZ") {
		include_cmath=true;
		return string("sqrtf")+args;
	} else if (name=="RC") {
		include_cmath=true;
		return string("sqrtf")+args;
	} else if (name=="ABS") {
		include_cmath=true;
		return string("abs")+args;
	} else if (name=="LN") {
		include_cmath=true;
		return string("log")+args;
	} else if (name=="EXP") {
		include_cmath=true;
		return string("exp")+args;
	} else if (name=="AZAR") {
		include_cstdlib=true;
		return string("(rand()%")+colocarParentesis(get_arg(args,1))+")";
	} else if (name=="ATAN") {
		include_cmath=true;
		return string("atan")+args;
	} else if (name=="TRUNC") {
		include_cmath=true;
		return string("floor")+args;
	} else if (name=="REDON") {
		include_cmath=true;
		return string("floor(")+colocarParentesis(get_arg(args,1))+"+.5)";
	} else if (name=="CONCATENAR") {
		return string("(")+convertirAString(get_arg(args,1))+"+"+get_arg(args,2)+")";
	} else if (name=="LONGITUD") {
		return convertirAString(get_arg(args,1))+".size()";
	} else if (name=="SUBCADENA") {
		return convertirAString(get_arg(args,1))+".substr("+get_arg(args,2)+","+get_arg(args,3)+"-"+get_arg(args,2)+"+1)";
	} else if (name=="CONVERTIRANUMERO") {
		include_cstdlib=true;
		string s=get_arg(args,1);
		if (esString(s)) return string("atof(")+s+")";
		else return string("atof(")+colocarParentesis(s)+".c_str())";
	} else {
		if (name=="MINUSCULAS") use_func_minusculas=true;
		if (name=="MAYUSCULAS") use_func_mayusculas=true;
		if (name=="CONVERTIRATEXTO") use_func_convertiratexto=true;
		return ToLower(name)+args; // no deberia pasar esto
	}
}

// funcion usada por cpp_declarar_variables para las internas de una funcion
// y para obtener los tipos de los argumentos de la funcion para las cabeceras
string CppExporter::get_tipo(map<string,tipo_var>::iterator &mit, bool for_func, bool by_ref) {
	tipo_var &t=mit->second;
	string stipo="SIN_TIPO ";
	if (t==vt_caracter) { stipo="string "; use_string=true; }
	else if (t==vt_numerica) stipo=t.rounded?"int ":"float ";
	else if (t==vt_logica) stipo="bool ";
	else use_sin_tipo=true;
	if (t.dims) {
		string ret=stipo+ToLower(mit->first)+make_dims(t.dims,"[","][","]",!for_func);
		while (ret.find("[0]")!=string::npos) {
			use_arreglo_max=true;
			ret.replace(ret.find("[0]"),3,"[ARREGLO_MAX]");
		}
		return ret;
	} else {
		if (by_ref) stipo+="&";
		return stipo+ToLower(mit->first);
	}
}

// resolucion de tipos (todo lo que acceda a cosas privadas de memoria tiene que estar en esta clase porque es la unica amiga)
void CppExporter::declarar_variables(t_output &prog) {
	map<string,tipo_var>::iterator mit=memoria->GetVarInfo().begin(), mit2=memoria->GetVarInfo().end();
	string tab("\t"),stipo;
	while (mit!=mit2) {
		prog.push_back(tab+get_tipo(mit)+";");
		mit++;
	}
}

// retorna el tipo y elimina de la memoria a esa variable
// se usa para armar las cabeceras de las funciones, las elimina para que no se
// vuelvan a declarar adentro
string CppExporter::get_tipo(string name, bool by_ref) {
	map<string,tipo_var>::iterator mit=memoria->GetVarInfo().find(name);
	if (mit==memoria->GetVarInfo().end()) 
		return "SIN_TIPO _variable_desconocida_"; // no debería pasar
	string ret = get_tipo(mit,true,by_ref);
	memoria->GetVarInfo().erase(mit);
	return ret;
}

void CppExporter::header(t_output &out) {
	// cabecera
	stringstream version; 
	version<<VERSION<<"-"<<ARCHITECTURE;
	if (!for_testing) {
		out.push_back(string("// Este codigo ha sido generado por el modulo psexport ")+version.str()+" de PSeInt");
		out.push_back("// dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental");
		out.push_back("// puede que el codigo generado no sea completamente correcto. Si encuentra");
		out.push_back("// errores por favor reportelos en el foro (http://pseint.sourceforge.net).");
		if (!for_testing) out.push_back("");
	}
	out.push_back("#include<iostream>");
	if (include_cmath) out.push_back("#include<cmath>");
	if (include_cstdlib) out.push_back("#include<cstdlib>");
	if (use_func_esperar) out.push_back("#include<ctime>");
	if (use_func_mayusculas||use_func_minusculas) out.push_back("#include<cctype>");
	if (use_func_convertiratexto) out.push_back("#include<stringstream>");
	out.push_back("using namespace std;");
	if (!for_testing) out.push_back("");
	if (use_func_esperar) {
		if (!for_testing) out.push_back("// No hay en el C++ estandar una funcion equivalente a \"esperar\", pero puede programarse una similar");
		out.push_back("void esperar(double t);");
		if (!for_testing) out.push_back("");
	}
	if (use_func_convertiratexto) {
		if (!for_testing) out.push_back("// No hay en el C++ estandar una funcion equivalente a \"convertiratexto\", pero puede programarse una equivalente.");
		out.push_back("string convertiratexto(float f);");
		if (!for_testing) out.push_back("");
	}
	if (use_func_mayusculas) {
		if (!for_testing) out.push_back("// No hay en el C++ estandar una funcion equivalente a \"mayusculas\", pero puede programarse una equivalente.");
		out.push_back("string mayusculas(string s);");
		if (!for_testing) out.push_back("");
	}
	if (use_func_minusculas) {
		if (!for_testing) out.push_back("// No hay en el C++ estandar una funcion equivalente a \"minusculas\", pero puede programarse una equivalente.");
		out.push_back("string minusculas(string s);");
		if (!for_testing) out.push_back("");
	}
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
	if (use_sin_tipo) {
		if (!for_testing) {
			out.push_back("// Para las variables que no se pudo determinar el tipo se utiliza la constante");
			out.push_back("// SIN_TIPO. El usuario debe reemplazar sus ocurrencias por el tipo adecuado");
			out.push_back("// (usualmente int,float,string o bool).");
		}
		out.push_back("#define SIN_TIPO string");
		if (!for_testing) out.push_back("");
	}
	if (use_string) {
		if (!for_testing) {
			out.push_back("// Para leer variables de texto se utiliza el operador << del objeto cin, que");
			out.push_back("// lee solo una palabra. Para leer una linea completa (es decir, incluyendo los");
			out.push_back("// espacios en blanco) se debe utilzar getline (ej, reemplazar cin>>x por");
			out.push_back("// getline(cin,x), pero obliga a agregar un cin.ignore() si antes del getline");
			out.push_back("// se leyó otra variable con >>.");
			out.push_back("");
		}
	}
}

void CppExporter::footer(t_output &out) {
	if (use_func_esperar) {
		if (!for_testing) out.push_back("");
		out.push_back("void esperar(double t) {");
		out.push_back("\tclock_t t0=clock();");
		out.push_back("\tdouble e=0;");
		out.push_back("\tdo {");
		out.push_back("\t\te=1000*double(clock()-t0)/CLOCKS_PER_SEC;");
		out.push_back("\t} while (e<t);");
		out.push_back("}");
		if (!for_testing) out.push_back("");
	}
	if (use_func_convertiratexto) {
		if (!for_testing) out.push_back("");
		out.push_back("string convertiratexto(float f) {");
		out.push_back("\tstringstream ss;");
		out.push_back("\tss<<f;");
		out.push_back("\treturn ss.str();");
		out.push_back("}");
		if (!for_testing) out.push_back("");
	}
	if (use_func_mayusculas) {
		if (!for_testing) out.push_back("");
		out.push_back("string mayusculas(string s) {");
		out.push_back("\tfor(unsigned int i=0;i<s.size();i++)");
		out.push_back("\t\ts[i]=toupper(s[i]);");
		out.push_back("\treturn s;");
		out.push_back("}");
		if (!for_testing) out.push_back("");
	}
	if (use_func_minusculas) {
		if (!for_testing) out.push_back("");
		out.push_back("string minusculas(string s) {");
		out.push_back("\tfor(unsigned int i=0;i<s.size();i++)");
		out.push_back("\t\ts[i]=tolower(s[i]);");
		out.push_back("\treturn s;");
		out.push_back("}");
		if (!for_testing) out.push_back("");
	}
}


void CppExporter::translate(t_output &out, t_proceso &proc) {
	
	memoria=new Memoria(NULL);
	
	//cuerpo del proceso
	t_output out_proc;
	bloque(out_proc,++proc.begin(),proc.end(),"\t");
	
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
		if (f->nombres[0]=="") {
			dec="void "; 
		} else {
			ret=get_tipo(f->nombres[0]);
			dec=ret.substr(0,ret.find(" ")+1);
			ret=string("return")+ret.substr(ret.find(" "));
		}
		dec+=ToLower(f->id)+"(";
		for(int i=1;i<=f->cant_arg;i++) {
			if (i!=1) dec+=", ";
			dec+=get_tipo(f->nombres[i],f->pasajes[i]==PP_REFERENCIA);
		}
		dec+=")";
		prototipos.push_back(dec+";");
		out.push_back(dec+" {");
		delete f;
	}
	
	declarar_variables(out);
	
	copy(out_proc.begin(),out_proc.end(),back_inserter(out));
	
	// cola del proceso
	if (ret.size()) out.push_back(string("\t")+ret+";");
	out.push_back("}");
	if (!for_testing) out.push_back("");
	
	delete memoria;
	
}

void CppExporter::translate(t_output &out, t_programa &prog) {
	t_output aux;
	for (t_programa_it it=prog.begin();it!=prog.end();++it)
		translate(aux,*it);	
	header(out);
	if (prototipos.size()) {
		if (!for_testing) {
			out.push_back("// Declaraciones adelantadas de las funciones");
			if (has_matrix_func) {
				out.push_back("// Las funciones que reciben arreglos en C++ deben especificar en sus");
				out.push_back("// prototipos las dimensiones de los mismos para todas las dimensiones,");
				out.push_back("// excepto la primera (que puede quedar vacía). Las funciones traducidas");
				out.push_back("// del pseudocódigo tienen todas sus dimensiones vacías, ya que PSeInt");
				out.push_back("// aún no determina automáticamente las mismas. Deberá completar las");
				out.push_back("// requeridas para poder compilar el programa.");
			}
		}
		copy(prototipos.begin(),prototipos.end(),back_inserter(out));
	}
	copy(aux.begin(),aux.end(),back_inserter(out));
	footer(out);
}

string CppExporter::get_constante(string name) {
	if (name=="PI") { include_cmath=true; return "M_PI"; }
	if (name=="VERDADERO") return "true";
	if (name=="FALSO") return "false";
	return name;
}

string CppExporter::get_operator(string op, bool for_string) {
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
	if (op=="^") return "func pow(arg1,arg2)";
	if (op=="%") return "%";
	if (op=="=") return "=="; 
	if (op=="<>") return "!="; 
	if (op=="<") return "<"; 
	if (op==">") return ">"; 
	if (op=="<=") return "<="; 
	if (op==">=") return ">="; 
	if (op=="&") return "&&"; 
	if (op=="|") return "||"; 
	if (op=="~") return "!"; 
	return op; // no deberia pasar nunca
}

string CppExporter::make_string (string cont) {
	for(unsigned int i=0;i<cont.size();i++)
		if (cont[i]=='\\') cont.insert(i++,"\\");
	return string("\"")+cont+"\"";
}

