#include "new_funciones.h"
#include "utils.h"
#include "intercambio.h"
#include <cmath>
#include <cstdlib>
using namespace std;

map<string,funcion> funciones;

funcion* EsFuncion(const string nombre) {
	map<string,funcion>::iterator it_func = funciones.find(nombre);
	if (it_func==funciones.end()) return NULL;
	else return &(it_func->second);
}

string func_rc(string *arg,tipo_var &tipo) {
	if (StrToDbl(*arg)<0) {
		if (Inter.Running())
			ExeError(147,"Raiz de negativo.");
		return "0";
	} else
		return DblToStr((double)(sqrt(double(StrToDbl(*arg)))));
}
string func_abs(string *arg,tipo_var &tipo) {
	string str=*arg;
	if (str[0]=='-') str.erase(0,1);
	return str;
}

string func_ln(string *arg,tipo_var &tipo) {
	if (StrToDbl(*arg)<=0) {
		if (Inter.Running())
			ExeError(148,"Logaritmo de 0 o negativo.");
		return "";
	} else
		return DblToStr((double)(log((double)(StrToDbl(*arg)))));
}
string func_exp(string *arg,tipo_var &tipo) {
	return DblToStr((double)(exp((double)(StrToDbl(*arg)))));
}
string func_sen(string *arg,tipo_var &tipo) {
	return DblToStr((double)(sin((double)(StrToDbl(*arg)))));
}
string func_asen(string *arg,tipo_var &tipo) {
	return DblToStr((double)(asin((double)(StrToDbl(*arg)))));
}
string func_acos(string *arg,tipo_var &tipo) {
	return DblToStr((double)(acos((double)(StrToDbl(*arg)))));
}
string func_cos(string *arg,tipo_var &tipo) {
	return DblToStr((double)(cos((double)(StrToDbl(*arg)))));
}
string func_tan(string *arg,tipo_var &tipo) {
	return DblToStr((double)(tan((double)(StrToDbl(*arg)))));
}
string func_atan(string *arg,tipo_var &tipo) {
	return DblToStr((double)(atan((double)(StrToDbl(*arg)))));
}
string func_azar(string *arg,tipo_var &tipo) {
	if (int(StrToDbl(*arg))<=0) {
		if (Inter.Running())
			ExeError(152,"Azar de 0 o negativo.");
		return "";
	} else
		return IntToStr(rand() % (int)StrToDbl(*arg));
}
string func_trunc(string *arg,tipo_var &tipo) {
	string str=*arg;
	if (str.find(".",0)>=0 && str.find(".",0)<str.size())
		str.erase(str.find(".",0),str.size()-str.find(".",0));
	if (str=="") str="0";
	return str;
}
string func_redon(string *arg,tipo_var &tipo) {
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

void LoadFunciones(bool u) {
	funciones[u?"RC":"rc"]=funcion(1,func_rc); 
	funciones[u?"RAIZ":"raiz"]=funcion(1,func_rc); 
	funciones[u?"ABS":"abs"]=funcion(1,func_abs);
	funciones[u?"LN":"ln"]=funcion(1,func_ln);
	funciones[u?"EXP":"exp"]=funcion(1,func_exp);
	funciones[u?"SEN":"sen"]=funcion(1,func_sen);
	funciones[u?"ASEN":"asen"]=funcion(1,func_asen);
	funciones[u?"ACOS":"acos"]=funcion(1,func_acos);
	funciones[u?"COS":"cos"]=funcion(1,func_cos);
	funciones[u?"TAN":"tan"]=funcion(1,func_tan);
	funciones[u?"ATAN":"atan"]=funcion(1,func_atan);
	funciones[u?"AZAR":"azar"]=funcion(1,func_azar);
	funciones[u?"TRUNC":"trunc"]=funcion(1,func_trunc);
	funciones[u?"REDON":"redon"]=funcion(1,func_redon);
}
