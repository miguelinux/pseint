
#include "new_funciones.h"
#include "utils.h"
#include "intercambio.h"
#include <cmath>
#include <cstdlib>
#include "global.h"
#include "DataValue.h"
using namespace std;

map<string,Funcion*> funciones;
map<string,Funcion*> subprocesos;

string main_process_name="<no_main_process>";

const Funcion* EsFuncionDelUsuario(const string &nombre, bool include_main_process) {
	if (nombre==main_process_name && !include_main_process) return NULL;
	map<string,Funcion*>::iterator it_func = subprocesos.find(nombre);
	if (it_func!=subprocesos.end()) return it_func->second;
	return NULL;
}

const Funcion* EsFuncionPredefinida(const string &nombre) {
	map<string,Funcion*>::iterator it_func = funciones.find(nombre);
	if (it_func!=funciones.end()) return it_func->second;
	return NULL;
}

const Funcion* EsFuncion(const string &nombre, bool include_main_process) {
	const Funcion *ret=EsFuncionDelUsuario(nombre, include_main_process);
	if (!ret) ret=EsFuncionPredefinida(nombre);
	return ret;
}

DataValue func_rc(DataValue *arg) {
	double x = arg[0].GetAsReal();
	if (x<0) {
		ExeError(147,"Raiz cuadrada de número negativo.",false);
		return DataValue::MakeEmpty(vt_numerica);
	} 
	return DataValue::MakeReal(sqrt(x));
}
DataValue func_abs(DataValue *arg) {
	double d = arg[0].GetAsReal();
	return DataValue::MakeReal( d<0 ? -d : d );
}

DataValue func_ln(DataValue *arg) {
	double x = arg[0].GetAsReal();
	if (x<=0) {
		ExeError(148,"Logaritmo de 0 o negativo.",false);
		return DataValue::MakeEmpty(vt_numerica);
	}
	return DataValue::MakeReal(log(x));
}
DataValue func_exp(DataValue *arg) {
	return DataValue::MakeReal(exp(arg[0].GetAsReal()));
}

DataValue func_sen(DataValue *arg) {
	return DataValue::MakeReal(sin(arg[0].GetAsReal()));
}

DataValue func_asen(DataValue *arg) {
	double x = arg[0].GetAsReal();
	if (x<-1||x>+1) {
		ExeError(312,"Argumento inválido para la función ASEN (debe estar en [-1;+1]).",false);
		return DataValue::MakeEmpty(vt_numerica);
	}
	return DataValue::MakeReal(asin(x));
}

DataValue func_acos(DataValue *arg) {
	double x = arg[0].GetAsReal();
	if (x<-1||x>+1) {
		ExeError(312,"Argumento inválido para la función ACOS (debe estar en [-1;+1]).",false);
		return DataValue::MakeEmpty(vt_numerica);
	}
	return DataValue::MakeReal(acos(x));
}

DataValue func_cos(DataValue *arg) {
	return DataValue::MakeReal(cos(arg[0].GetAsReal()));
}

DataValue func_tan(DataValue *arg) {
	return DataValue::MakeReal(tan(arg[0].GetAsReal()));
}

DataValue func_atan(DataValue *arg) {
	return DataValue::MakeReal(atan(arg[0].GetAsReal()));
}

DataValue func_azar(DataValue *arg) {
	int x = arg[0].GetAsInt();
	if (x<=0) {
		ExeError(306,"Azar de 0 o negativo.",false);
		return DataValue::MakeEmpty(vt_numerica_entera);
	} else
		return DataValue::MakeInt(rand()%x);
}

DataValue func_aleatorio(DataValue *arg) {
	int a = arg[0].GetAsInt();
	int b = arg[1].GetAsInt();
	if (b<a) { int x=a; a=b; b=x; }
	return DataValue::MakeInt(a+rand() % (b-a+1));
}

DataValue func_trunc(DataValue *arg) {
	double dbl = arg[0].GetAsReal();
	int i = int(dbl);
	// intentar compensar algunos errores numéricos... que al menos parezca el 
	// número que se ve al escribir, el cual se redondea a 10 decimales... 
	// por eso uso un epsilon un poquito más chico que eso
	if ((i+1)-dbl<1e-12) i++;      //  ((+56)+1)-(+56.999999) = +0.00001,   
	else if ((i-1)-dbl>-1e-12) i--; // ((-56)-1)-(-56.999999) = -0.00001
	return DataValue::MakeInt(i);
}

DataValue func_redon(DataValue *arg) {
	double x = arg[0].GetAsReal();
	return DataValue::MakeInt(int(x+(x<0?-.5:+.5)));
}

DataValue func_longitud(DataValue *arg) {
	return DataValue::MakeInt(arg[0].GetAsString().size());
}

DataValue func_mayusculas(DataValue *arg) {
	string s = arg[0].GetAsString(); size_t l = s.size();
	for(size_t i=0;i<l;i++) { 
		if (s[i]>='a'&&s[i]<='z') s[i]+='A'-'a';
		else if (s[i]=='ñ') s[i]='Ñ';
		else if (s[i]=='á') s[i]='Á';
		else if (s[i]=='é') s[i]='É';
		else if (s[i]=='í') s[i]='Í';
		else if (s[i]=='ó') s[i]='Ó';
		else if (s[i]=='ú') s[i]='Ú';
		else if (s[i]=='ü') s[i]='Ü';
	}
	return DataValue::MakeString(s);
}

DataValue func_minusculas(DataValue *arg) {
	string s = arg[0].GetAsString(); size_t l = s.length();
	for(size_t i=0;i<l;i++) { 
		if (s[i]>='A'&&s[i]<='Z') s[i]+='a'-'A';
		else if (s[i]=='Ñ') s[i]='ñ';
		else if (s[i]=='Á') s[i]='á';
		else if (s[i]=='É') s[i]='é';
		else if (s[i]=='Í') s[i]='í';
		else if (s[i]=='Ó') s[i]='ó';
		else if (s[i]=='Ú') s[i]='ú';
		else if (s[i]=='Ü') s[i]='ü';
	}
	return DataValue::MakeString(s);
}

DataValue func_subcadena(DataValue *arg) {
	string s = arg[0].GetAsString(); int l=s.length(), f=arg[1].GetAsInt(), t=arg[2].GetAsInt();
	if (!lang[LS_BASE_ZERO_ARRAYS]) { f--; t--; }
	if (t>l-1) t=l-1; if (f<0) f=0;
	if (t<f) return DataValue::MakeEmpty(vt_caracter);
	return DataValue::MakeString(s.substr(f,t-f+1));
}

DataValue func_concatenar(DataValue *arg) {
	return DataValue::MakeString(arg[0].GetAsString()+arg[1].GetAsString());
}

DataValue func_atof(DataValue *arg) {
	// verificar formato
	string s = arg[0].GetAsString();
	bool punto=false; int j=0;
	if (s.size() && (s[0]=='+'||s[0]=='-')) j++;
	for(unsigned int i=j;i<s.size();i++) {
		if (!punto && s[i]=='.')
			punto=true;
		else if (s[i]<'0'||s[i]>'9') {
			ExeError(311,string("La cadena (\"")+s+"\") no representa un número.",true);
			return DataValue::MakeEmpty(vt_numerica);
		}
	}
	// convertir
	return DataValue::MakeReal(s);
}

DataValue func_ftoa(DataValue *arg) {
	return DataValue::MakeString(arg[0].GetForUser()); // la conversión es para que redondee
}

DataValue func_pi(DataValue *arg) {
	return DataValue::MakeReal("3.141592653589793238462643383279502884197169399375105820974944592");
}

void LoadFunciones() {
	funciones["PI"]=new Funcion(vt_numerica,func_pi); 
	funciones["RC"]=new Funcion(vt_numerica,func_rc,vt_numerica); 
	funciones["RAIZ"]=new Funcion(vt_numerica,func_rc,vt_numerica); 
	funciones["ABS"]=new Funcion(vt_numerica,func_abs,vt_numerica);
	funciones["LN"]=new Funcion(vt_numerica,func_ln,vt_numerica);
	funciones["EXP"]=new Funcion(vt_numerica,func_exp,vt_numerica);
	funciones["SEN"]=new Funcion(vt_numerica,func_sen,vt_numerica);
	funciones["ASEN"]=new Funcion(vt_numerica,func_asen,vt_numerica);
	funciones["ACOS"]=new Funcion(vt_numerica,func_acos,vt_numerica);
	funciones["COS"]=new Funcion(vt_numerica,func_cos,vt_numerica);
	funciones["TAN"]=new Funcion(vt_numerica,func_tan,vt_numerica);
	funciones["ATAN"]=new Funcion(vt_numerica,func_atan,vt_numerica);
	funciones["AZAR"]=new Funcion(vt_numerica,func_azar,vt_numerica_entera);
	funciones["ALEATORIO"]=new Funcion(vt_numerica,func_aleatorio,vt_numerica_entera,vt_numerica_entera);
	funciones["TRUNC"]=new Funcion(vt_numerica,func_trunc,vt_numerica);
	funciones["REDON"]=new Funcion(vt_numerica,func_redon,vt_numerica);
	if (lang[LS_ENABLE_STRING_FUNCTIONS]) {
		funciones["CONVERTIRANÚMERO"]=new Funcion(vt_numerica,func_atof,vt_caracter);
		funciones["CONVERTIRANUMERO"]=new Funcion(vt_numerica,func_atof,vt_caracter);
		funciones["CONVERTIRATEXTO"]=new Funcion(vt_caracter,func_ftoa,vt_numerica);
		funciones["LONGITUD"]=new Funcion(vt_numerica,func_longitud,vt_caracter);
		funciones["SUBCADENA"]=new Funcion(vt_caracter,func_subcadena,vt_caracter,vt_numerica_entera,vt_numerica_entera);
		funciones["MAYUSCULAS"]=new Funcion(vt_caracter,func_mayusculas,vt_caracter);
		funciones["MINUSCULAS"]=new Funcion(vt_caracter,func_minusculas,vt_caracter);
		funciones["MAYÚSCULAS"]=new Funcion(vt_caracter,func_mayusculas,vt_caracter);
		funciones["MINÚSCULAS"]=new Funcion(vt_caracter,func_minusculas,vt_caracter);
		funciones["CONCATENAR"]=new Funcion(vt_caracter,func_concatenar,vt_caracter,vt_caracter);
	}
}

void UnloadSubprocesos() {
	map<string,Funcion*>::iterator it1=subprocesos.begin(), it2=subprocesos.end();
	while (it1!=it2) {
		delete (it1)->second->memoria;
		delete (it1)->second;
		++it1;
	}
	subprocesos.clear();
}

void UnloadFunciones() {
	map<string,Funcion*>::iterator it1=funciones.begin(), it2=funciones.end();
	while (it1!=it2) {
		delete (it1)->second;
		++it1;
	}
	funciones.clear();
}

string GetNombreFuncion(const Funcion * func) {
	map<string,Funcion*>::iterator it1=subprocesos.begin(), it2=subprocesos.end();
	while (it1!=it2) {
		if (it1->second==func) return it1->first;
		else ++it1;
	}
	return "";
}

