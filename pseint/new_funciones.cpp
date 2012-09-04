#include "new_funciones.h"
#include "utils.h"
#include "intercambio.h"
#include <cmath>
#include <cstdlib>
#include "global.h"
using namespace std;

map<string,funcion> funciones;
map<string,funcion> subprocesos;

funcion* EsFuncion(const string nombre) {
	map<string,funcion>::iterator it_func = subprocesos.find(nombre);
	if (it_func!=subprocesos.end()) return &(it_func->second);
	it_func = funciones.find(nombre);
	if (it_func!=funciones.end()) return &(it_func->second);
	return NULL;
}

string func_rc(string *arg) {
	if (StrToDbl(*arg)<0) {
		if (Inter.Running())
			ExeError(147,"Raiz de negativo.");
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
		if (Inter.Running())
			ExeError(148,"Logaritmo de 0 o negativo.");
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
	return DblToStr((double)(asin((double)(StrToDbl(*arg)))));
}
string func_acos(string *arg) {
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
		if (Inter.Running())
			ExeError(152,"Azar de 0 o negativo.");
		return "";
	} else
		return IntToStr(rand() % (int)StrToDbl(*arg));
}
string func_trunc(string *arg) {
	string str=*arg;
	if (str.find(".",0)>=0 && str.find(".",0)<str.size())
		str.erase(str.find(".",0),str.size()-str.find(".",0));
	if (str=="") str="0";
	return str;
}
string func_redon(string *arg) {
	string str=*arg;
	if (str.find(".",0)>=0 && str.find(".",0)<str.size())
		str.erase(str.find(".",0),str.size()-str.find(".",0));
	if (str=="") str="0";
	double a,b;
	a=(StrToDbl(*arg));
	b=(StrToDbl(str));
	if (a<0) {a=-a;b=-b;}
	if ((a-b)>.5) b=b+1;
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
		else if (s[i]=='É') s[i]='e';
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
	return s.substr(f,t-f+1);
}

string func_concatenar(string *arg) {
	return (arg[0])+(arg[1]);
}

void LoadFunciones(bool u) {
	funciones[u?"RC":"rc"]=funcion(vt_numerica,func_rc,vt_numerica); 
	funciones[u?"RAIZ":"raiz"]=funcion(vt_numerica,func_rc,vt_numerica); 
	funciones[u?"ABS":"abs"]=funcion(vt_numerica,func_abs,vt_numerica);
	funciones[u?"LN":"ln"]=funcion(vt_numerica,func_ln,vt_numerica);
	funciones[u?"EXP":"exp"]=funcion(vt_numerica,func_exp,vt_numerica);
	funciones[u?"SEN":"sen"]=funcion(vt_numerica,func_sen,vt_numerica);
	funciones[u?"ASEN":"asen"]=funcion(vt_numerica,func_asen,vt_numerica);
	funciones[u?"ACOS":"acos"]=funcion(vt_numerica,func_acos,vt_numerica);
	funciones[u?"COS":"cos"]=funcion(vt_numerica,func_cos,vt_numerica);
	funciones[u?"TAN":"tan"]=funcion(vt_numerica,func_tan,vt_numerica);
	funciones[u?"ATAN":"atan"]=funcion(vt_numerica,func_atan,vt_numerica);
	funciones[u?"AZAR":"azar"]=funcion(vt_numerica,func_azar,vt_numerica_entera);
	funciones[u?"TRUNC":"trunc"]=funcion(vt_numerica,func_trunc,vt_numerica);
	funciones[u?"REDON":"redon"]=funcion(vt_numerica,func_redon,vt_numerica);
	if (enable_string_functions) {
		funciones[u?"LONGITUD":"longitud"]=funcion(vt_numerica,func_longitud,vt_caracter);
		funciones[u?"SUBCADENA":"subcadena"]=funcion(vt_caracter,func_subcadena,vt_caracter,vt_numerica_entera,vt_numerica_entera);
		funciones[u?"MAYUSCULAS":"subcadena"]=funcion(vt_caracter,func_mayusculas,vt_caracter);
		funciones[u?"MINUSCULAS":"minusculas"]=funcion(vt_caracter,func_minusculas,vt_caracter);
		funciones[u?"CONCATENAR":"concatenar"]=funcion(vt_caracter,func_concatenar,vt_caracter,vt_caracter);
	}
}

