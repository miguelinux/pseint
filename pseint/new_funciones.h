#ifndef NEW_FUNCIONES_H
#define NEW_FUNCIONES_H
#include <string>
#include <map>
#include "new_memoria.h"
using namespace std;

#define FUNC_MAX_NUM_ARGS 3

struct funcion {
	int cant_arg;
	tipo_var tipos[FUNC_MAX_NUM_ARGS+1]; // la pos 0 es para el tipo que retorna
	string (*func)(string *args);
	funcion(){}
	funcion(tipo_var tipo_ret, string (*af)(string *args), tipo_var tipo_arg_1):cant_arg(1),func(af) { tipos[0]=tipo_ret; tipos[1]=tipo_arg_1; }
	funcion(tipo_var tipo_ret, string (*af)(string *args), tipo_var tipo_arg_1, tipo_var tipo_arg_2):cant_arg(2),func(af) { tipos[0]=tipo_ret; tipos[1]=tipo_arg_1; tipos[2]=tipo_arg_2; }
	funcion(tipo_var tipo_ret, string (*af)(string *args), tipo_var tipo_arg_1, tipo_var tipo_arg_2, tipo_var tipo_arg_3):cant_arg(3),func(af) { tipos[0]=tipo_ret; tipos[1]=tipo_arg_1; tipos[2]=tipo_arg_2; tipos[3]=tipo_arg_3; }
};

extern map<string,funcion> funciones;

void LoadFunciones(bool u=true);
funcion* EsFuncion(const string nombre);

#endif

