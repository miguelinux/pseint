
#include "new_funciones.h"
#include "utils.h"
#include "intercambio.h"
#include <cmath>
#include <cstdlib>
#include "global.h"
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

string func_rc(string *arg) {
	if (StrToDbl(*arg)<0) {
		ExeError(147,"Raiz de negativo.",false);
		return "0";
	} else
		return DblToStr((double)(sqrt(double(StrToDbl(*arg)))));
}
string func_abs(string *arg) {
	string str=*arg;
	if (str[0]=='-') str.erase(0,1);
	return str;
}

string func_ln(string *arg) {
	if (StrToDbl(*arg)<=0) {
		ExeError(148,"Logaritmo de 0 o negativo.",false);
		return "";
	} else
		return DblToStr((double)(log((double)(StrToDbl(*arg)))));
}
string func_exp(string *arg) {
	return DblToStr((double)(exp((double)(StrToDbl(*arg)))));
}
string func_sen(string *arg) {
	return DblToStr((double)(sin((double)(StrToDbl(*arg)))));
}
string func_asen(string *arg) {
	if (int(StrToDbl(*arg))<-1||int(StrToDbl(*arg))>+1) {
		ExeError(312,"Argumento inválido para la función ASEN (debe estar en [-1;+1]).",false);
		return "";
	}
	return DblToStr((double)(asin((double)(StrToDbl(*arg)))));
}
string func_acos(string *arg) {
	if (int(StrToDbl(*arg))<-1||int(StrToDbl(*arg))>+1) {
		ExeError(312,"Argumento inválido para la función ACOS (debe estar en [-1;+1]).",false);
		return "";
	}
	return DblToStr((double)(acos((double)(StrToDbl(*arg)))));
}
string func_cos(string *arg) {
	return DblToStr((double)(cos((double)(StrToDbl(*arg)))));
}
string func_tan(string *arg) {
	return DblToStr((double)(tan((double)(StrToDbl(*arg)))));
}
string func_atan(string *arg) {
	return DblToStr((double)(atan((double)(StrToDbl(*arg)))));
}
string func_azar(string *arg) {
	if (int(StrToDbl(*arg))<=0) {
		ExeError(306,"Azar de 0 o negativo.",false);
		return "";
	} else
		return IntToStr(rand() % (int)StrToDbl(*arg));
}
string func_trunc(string *arg) {
	string str=*arg;
	size_t pos_pt=str.find(".",0);
	if (pos_pt!=string::npos)
		str.erase(pos_pt,str.size()-pos_pt);
	if (str=="") str="0";
	return str;
}
string func_redon(string *arg) {
	string str=*arg;
	size_t pos_pt=str.find(".",0);
	if (pos_pt!=string::npos)
		str.erase(pos_pt,str.size()-pos_pt);
	if (str=="") str="0";
	double a,b;
	a=(StrToDbl(*arg));
	b=(StrToDbl(str));
	if (a<0) {a=-a;b=-b;}
	if ((a-b)>=.5) b=b+1;
	if ((*arg)[0]=='-') b=-b;
	str=DblToStr(b);
	return str;
}


string func_longitud(string *arg) {
	return IntToStr(arg[0].length());
}

string func_mayusculas(string *arg) {
	string &s=arg[0]; int l=s.length();
	for(int i=0;i<l;i++) { 
		if (s[i]>='a'&&s[i]<='z') s[i]+='A'-'a';
		else if (s[i]=='ñ') s[i]='Ñ';
		else if (s[i]=='á') s[i]='Á';
		else if (s[i]=='é') s[i]='É';
		else if (s[i]=='í') s[i]='Í';
		else if (s[i]=='ó') s[i]='Ó';
		else if (s[i]=='ú') s[i]='Ú';
		else if (s[i]=='ü') s[i]='Ü';
	}
	return s;
}

string func_minusculas(string *arg) {
	string &s=arg[0]; int l=s.length();
	for(int i=0;i<l;i++) { 
		if (s[i]>='A'&&s[i]<='Z') s[i]+='a'-'A';
		else if (s[i]=='Ñ') s[i]='ñ';
		else if (s[i]=='Á') s[i]='á';
		else if (s[i]=='É') s[i]='é';
		else if (s[i]=='Í') s[i]='í';
		else if (s[i]=='Ó') s[i]='ó';
		else if (s[i]=='Ú') s[i]='ú';
		else if (s[i]=='Ü') s[i]='ü';
	}
	return s;
}

string func_subcadena(string *arg) {
	string &s=arg[0]; int l=s.length(), f=(int)StrToDbl(arg[1]), t=(int)StrToDbl(arg[2]);
	if (!base_zero_arrays) { f--; t--; }
	if (t>l-1) t=l-1; if (f<0) f=0;
	if (t<f) return "";
	return s.substr(f,t-f+1);
}

string func_concatenar(string *arg) {
	return (arg[0])+(arg[1]);
}

string func_atof(string *arg) {
	// verificar formato
	string &s=arg[0];
	bool punto=false; int j=0;
	if (s.size() && (s[0]=='+'||s[0]=='-')) j++;
	for(unsigned int i=j;i<s.size();i++) {
		if (!punto && s[i]=='.')
			punto=true;
		else if (s[i]<'0'||s[i]>'9') {
			ExeError(311,string("La cadena (\"")+s+"\") no representa un número.",true);
			return "0";
		}
	}
	// convertir
	return DblToStr(StrToDbl(arg[0]));
}

string func_ftoa(string *arg) {
	return arg[0];
}

string func_pi(string *arg) {
	return "3.141592653589793238462643383279502884197169399375105820974944592";
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
	funciones["TRUNC"]=new Funcion(vt_numerica,func_trunc,vt_numerica);
	funciones["REDON"]=new Funcion(vt_numerica,func_redon,vt_numerica);
	if (enable_string_functions) {
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

