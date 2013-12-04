#include "export_c.h"
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

CExporter::CExporter():CppExporter() {
	use_bool=false;
	declare_cstrings=false;
}

void CExporter::borrar_pantalla(t_output &prog, string param, string tabs){
	if (for_testing)
		insertar(prog,tabs+"printf(\"\\n\");");
	else
		insertar(prog,tabs+"printf(\"\\n\"); /* no hay forma directa de borrar la pantalla con C estandar */");
}

void CExporter::esperar_tecla(t_output &prog, string param, string tabs){
	if (for_testing)
		insertar(prog,tabs+"getchar();");
	else
		insertar(prog,tabs+"getchar(); /* a diferencia del pseudocódigo, espera un Enter, no cualquier tecla */");
}

void CExporter::escribir(t_output &prog, t_arglist args, bool saltar, string tabs){
	string format,arglist;
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		tipo_var t;
		string varname=expresion(*it,t);
		if (es_cadena_constante(*it)) {
			format+=varname.substr(1,varname.size()-2);
		} else {
			if (t==vt_numerica && t.rounded) {
				format+="%i"; arglist+=","; arglist+=varname;
			} else if (t==vt_numerica) {
				format+="%f"; arglist+=","; arglist+=varname;
			} else if (t==vt_logica) {
				format+="%i"; arglist+=","; arglist+=varname;
			} else {
				format+="%s"; arglist+=","; arglist+=varname;
			}
		}
		it++;
	}
	if (saltar) format+="\\n";
	insertar(prog,tabs+"printf(\""+format+"\""+arglist+");");
}

void CExporter::leer(t_output &prog, t_arglist args, string tabs) {
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		tipo_var t;
		string varname=expresion(*it,t);
		if (t==vt_numerica && t.rounded) insertar(prog,tabs+"scanf(\"%i\",&"+varname+");");
		else if (t==vt_numerica) insertar(prog,tabs+"scanf(\"%f\",&"+varname+");");
		else if (t==vt_logica) insertar(prog,tabs+"scanf(\"%i\",&"+varname+");");
		else { read_strings=true; insertar(prog,tabs+"scanf(\"%s\","+varname+");"); }
		it++;
	}
}


void CExporter::para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
	string var=expresion((*r).par1), ini=expresion((*r).par2), fin=expresion((*r).par3), paso=(*r).par4;
	if ((*r).par4[0]=='-') {
		insertar(prog,tabs+"for ("+var+"="+ini+";"+var+">="+fin+";"+var+"-="+expresion(paso.substr(1,paso.size()-1))+") {");
	} else {
		insertar(prog,tabs+"for ("+var+"="+ini+";"+var+"<="+fin+";"+var+"+="+expresion(paso)+") {");
	}
	bloque(prog,++r,q,tabs+"\t");
	insertar(prog,tabs+"}");
}

void CExporter::paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs){
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
	insertar(prog,tabs+"for (typeof(&("+first+")) "+aux+"=&("+first+");"+aux+"<=&("+last+");"+aux+"++) {");
	t_output out;
	bloque(out,++r,q,tabs+"\t");
	replace_var(out,aux,string("(*"+aux+")"));
	insertar_out(prog,out);
	insertar(prog,tabs+"}");
}



string CExporter::function(string name, string args) {
	if (name=="CONCATENAR") {
		return string("(")+convertirAString(get_arg(args,1))+"+"+get_arg(args,2)+")";
	} else if (name=="LONGITUD") {
		return string("strlen("+get_arg(args,1))+")";
	} else if (name=="SUBCADENA") {
		return convertirAString(get_arg(args,1))+".substr("+get_arg(args,2)+","+get_arg(args,3)+"-"+get_arg(args,2)+"+1)";
	} else if (name=="CONVERTIRANUMERO") {
		include_cstdlib=true;
		string s=get_arg(args,1);
		return string("atof(")+s+")";
	} else {
		return CppExporter::function(name,args); // las demás son iguales que en la conversión a C++
	}
}

// funcion usada por declarar_variables para las internas de una funcion
// y para obtener los tipos de los argumentos de la funcion para las cabeceras
string CExporter::get_tipo(map<string,tipo_var>::iterator &mit, bool for_func, bool by_ref) {
	tipo_var &t=mit->second;
	string stipo="SIN_TIPO ", spost;
	if (t==vt_caracter) { stipo="char "; if (for_func) stipo+="*"; else { declare_cstrings=true; spost="[MAX_STRLEN]"; } }
	else if (t==vt_numerica) stipo=t.rounded?"int ":"float ";
	else if (t==vt_logica) { use_bool=true; stipo="bool "; }
	else use_sin_tipo=true;
	if (t.dims) {
		string ret=stipo+ToLower(mit->first)+spost+make_dims(t.dims,"[","][","]",!for_func);
		while (ret.find("[0]")!=string::npos) {
			use_arreglo_max=true;
			ret.replace(ret.find("[0]"),3,"[ARREGLO_MAX]");
		}
		return ret;
	} else {
		if (by_ref) stipo+="*";
		return stipo+ToLower(mit->first)+spost;
	}
}


void CExporter::header(t_output &out) {
	// cabecera
	init_header(out,"/* "," */");
	out.push_back("#include<stdio.h>");
	if (include_cmath) out.push_back("#include<math.h>");
	if (include_cstdlib) out.push_back("#include<stdlib.h>");
	if (use_func_esperar) out.push_back("#include<time.h>");
	if (use_bool) out.push_back("#include<stdbool.h>");
	if (use_func_mayusculas||use_func_minusculas) out.push_back("#include<ctype.h>");
	if (use_string) out.push_back("#include<string.h>");
	out.push_back("using namespace std;");
	if (!for_testing) out.push_back("");
	if (use_func_esperar) {
		if (!for_testing) out.push_back("/* No hay en el C++ estandar una funcion equivalente a \"esperar\", pero puede programarse una similar */");
		out.push_back("void esperar(double t);");
		if (!for_testing) out.push_back("");
	}
	if (use_func_convertiratexto) {
		if (!for_testing) out.push_back("/* No hay en el C++ estandar una funcion equivalente a \"convertiratexto\", pero puede programarse una equivalente. */");
		out.push_back("char *convertiratexto(float f);");
		if (!for_testing) out.push_back("");
	}
	if (use_func_mayusculas) {
		if (!for_testing) out.push_back("/* No hay en el C++ estandar una funcion equivalente a \"mayusculas\", pero puede programarse una equivalente. */");
		out.push_back("char *mayusculas(const char *s);");
		if (!for_testing) out.push_back("");
	}
	if (use_func_minusculas) {
		if (!for_testing) out.push_back("/* No hay en el C++ estandar una funcion equivalente a \"minusculas\", pero puede programarse una equivalente. */");
		out.push_back("char *minusculas(const char *s);");
		if (!for_testing) out.push_back("");
	}
	if (use_get_aux_buffer) {
		if (!for_testing) out.push_back("/* Función auxiliar para el manejo de cstrings temporales, ver detalles más abajo. */");
		out.push_back("char *get_aux_buffer(double t);");
	}
	if (use_arreglo_max) {
		if (!for_testing) {
			out.push_back("/* En C no se puede dimensionar un arreglo estático con una dimensión no constante.");
			out.push_back("   PSeInt sobredimensionará el arreglo utilizando un valor simbólico ARREGLO_MAX.");
			out.push_back("   Sería posible crear un arreglo dinámicamente con los operadores new y delete, pero");
			out.push_back("   este mecanismo aún no está soportado en las traducciones automáticas de PSeInt. */");
		}
		out.push_back("#define ARREGLO_MAX 100");
		if (!for_testing) out.push_back("");
	}
	if (declare_cstrings) {
		if (!for_testing) {
			out.push_back("/* En C no hay variables para guardar cadenas de texto, sino que debe construirse");
			out.push_back("   un arreglo de caracteres (tipo char). El tamaño del arreglo determina la longitud");
			out.push_back("   máxima que puede tener la cadena que guarda (tamaño-1, por el caracter de terminación).");
			out.push_back("   La constante MAX_STRLEN define el tamaño máximo que se utiliza en este programa para");
			out.push_back("   cualquier cadena. */");
			
		}
		out.push_back("#define MAX_STRLEN 256");
		if (!for_testing) out.push_back("");
	}
	if (use_sin_tipo) {
		if (!for_testing) {
			out.push_back("/* Para las variables que no se pudo determinar el tipo se utiliza la constante");
			out.push_back("   SIN_TIPO. El usuario debe reemplazar sus ocurrencias por el tipo adecuado");
			out.push_back("   (usualmente int,float,bool, o char[]). */");
		}
		out.push_back("#define SIN_TIPO float");
		if (!for_testing) out.push_back("");
	}
	if (read_strings) {
		if (!for_testing) {
			out.push_back("/* Para leer variables de texto se utiliza scanf, que lee solo una palabra. ");
			out.push_back("   Para leer una linea completa (es decir, incluyendo los espacios en blanco)");
			out.push_back("   se debe utilzar ges (ej, reemplazar scanf(\"%s\",x) por gets(x)) pero ");
			out.push_back("   obliga a agregar un getchar() antes del gets si antes del mismo se leyó");
			out.push_back("   otra variable con scanf. */");
			out.push_back("");
		}
	}
}

void CExporter::footer(t_output &out) {
	if (use_get_aux_buffer) {
		if (!for_testing) { 
			out.push_back("");
			out.push_back("/* Algunas operaciones de cadenas de texto requieren cadenas auxiliares temporales. ");
			out.push_back("   Por ejemplo, la concatenación de cadenas. Como las cadenas son en realidad arreglos"); 
			out.push_back("   no hay forma simple de crear estos temporales dinámicamente y destruirlos cuando ya"); 
			out.push_back("   no sean necesarios. Si cada función que requiere un buffer auxiliar usa su propio"); 
			out.push_back("   buffer estático, entonces no se puede invocar a esa función dos veces en la misma"); 
			out.push_back("   expresión (porque ambas invocaciones retornarían un mismo buffer como resultado"); 
			out.push_back("   aunque sus argumentos sean diferentes, y además esto genera un comportamiento ");
			out.push_back("   indefinido según el estándar de C). Para salvar este problema, esta función define");
			out.push_back("   varios buffers auxiliares y los va entregando a las demás funciones a medida que");
			out.push_back("   estas los requieren, rotando cual entrega en orden. Entonces, una expresión puede");
			out.push_back("   requerir tantos buffers como MAX_BUFFERS indique sin que se generen problemas,");
			out.push_back("   y se evita utilizar memoria dinámica. */");
		}
		out.push_back("#define MAX_BUFFERS 10");
		out.push_back("char *get_aux_buffer(double t) {");
		out.push_back("\tstatic char buffers[MAX_BUFFERS][MAX_STRLEN];");
		out.push_back("\tstatic int count=-1;");
		out.push_back("\tcount=count+1;");
		out.push_back("\tif(count==MAX_BUFFERS) count=0;");
		out.push_back("\treturn buffers[count];");
		out.push_back("}");
		if (!for_testing) out.push_back("");
	}
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
		out.push_back("char *convertiratexto(float f) {");
		out.push_back("\tchar *buf=get_aux_buffer();");
		out.push_back("\tsprintf(buf,\"%f\",f);");
		out.push_back("\treturn buf;");
		out.push_back("}");
		if (!for_testing) out.push_back("");
	}
	if (use_func_mayusculas) {
		if (!for_testing) out.push_back("");
		out.push_back("char *mayusculas(const char *s) {");
		out.push_back("\tchar *buf=get_aux_buffer();");
		out.push_back("\tfor(unsigned int i=0;i<s.size();i++)");
		out.push_back("\t\tbuf[i]=toupper(s[i]);");
		out.push_back("\treturn buf;");
		out.push_back("}");
		if (!for_testing) out.push_back("");
	}
	if (use_func_minusculas) {
		if (!for_testing) out.push_back("");
		out.push_back("char *minusculas(const char *s) {");
		out.push_back("\tchar *buf=get_aux_buffer();");
		out.push_back("\tfor(unsigned int i=0;i<s.size();i++)");
		out.push_back("\t\tbuf[i]=tolower(s[i]);");
		out.push_back("\treturn buf;");
		out.push_back("}");
		if (!for_testing) out.push_back("");
	}
}


void CExporter::translate(t_output &out, t_proceso &proc) {
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
			ret=CppExporter::get_tipo(f->nombres[0]);
			dec=ret.substr(0,ret.find(" ")+1);
			ret=string("return")+ret.substr(ret.find(" "));
		}
		dec+=ToLower(f->id)+"(";
		for(int i=1;i<=f->cant_arg;i++) {
			if (i!=1) dec+=", ";
			string var_dec=CppExporter::get_tipo(f->nombres[i],f->pasajes[i]==PP_REFERENCIA);
			dec+=var_dec;
			if (f->pasajes[i]==PP_REFERENCIA && var_dec.find("[")==string::npos) 
				replace_var(out_proc,ToLower(f->nombres[i]),ToLower(string("(*")+f->nombres[i]+(")")));
		}
		dec+=")";
		prototipos.push_back(dec+";");
		out.push_back(dec+" {");
		delete f;
	}
	
	declarar_variables(out);
	
	insertar_out(out,out_proc);
	
	// cola del proceso
	if (ret.size()) out.push_back(string("\t")+ret+";");
	out.push_back("}");
	if (!for_testing) out.push_back("");
	
	delete memoria;
	
}


string CExporter::get_operator(string op, bool for_string) {
	if (for_string) {
		if (for_string) {
			use_string=true;
			if (op=="+") { use_get_aux_buffer=true; return "func strcat(strcpy(get_aux_buffer(),arg1),arg2)"; }
			if (op=="=") return "func strcmp(arg1,arg2)==0"; 
			if (op=="<>") return "func strcmp(arg1,arg2)!=0"; 
			if (op=="<") return "func strcmp(arg1,arg2)<0"; 
			if (op==">") return "func strcmp(arg1,arg2)>0"; 
			if (op=="<=") return "func strcmp(arg1,arg2)<=0"; 
			if (op==">=") return "func strcmp(arg1,arg2)>=0"; 
		}
	} else return CppExporter::get_operator(op,false);
}
