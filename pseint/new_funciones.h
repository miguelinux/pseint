#ifndef NEW_FUNCIONES_H
#define NEW_FUNCIONES_H
#include <string>
#include <map>
#include "new_memoria.h"
using namespace std;

struct funcion {
	int cant_arg;
	string (*func)(string *args, tipo_var &tipo);
	funcion(){}
	funcion(int ac, string (*af)(string *args, tipo_var &tipo)):cant_arg(ac),func(af){}
};

extern map<string,funcion> funciones;

void LoadFunciones(bool u=true);
funcion* EsFuncion(const string nombre);

#endif

