#ifndef NEW_FUNCIONES_H
#define NEW_FUNCIONES_H
#include <string>
#include <map>
#include "new_memoria.h"
#include <vector>
using namespace std;

#define FUNC_MAX_NUM_ARGS 3

struct funcion {
	int cant_arg;
	vector<tipo_var> tipos; // la pos 0 es para el tipo que retorna
	vector<string> nombres; // nombres de los argumentos, la pos 0 es para el valor de retorno
	string (*func)(string *args); // NULL si es de las definidas por el usuario como subproceso
	int line_start; // linea del pseudocodigo donde comienza la funcion
	void AddArg(string arg) { nombres.push_back(arg); tipos.push_back(vt_desconocido); cant_arg++; }
	funcion(){}
//	~funcion() { if (!func) delete memoria; } // esto trae problemas si se copian funcs, por eso se borran aparte, es menos lio que el ctor de copia correcto
	funcion(int line):func(NULL),line_start(line) { AddArg(""); cant_arg=0; }
	funcion(tipo_var tipo_ret, string (*af)(string *args), tipo_var tipo_arg_1):cant_arg(1),func(af) { 
		tipos.resize(cant_arg+1); tipos[0]=tipo_ret; tipos[1]=tipo_arg_1;
	}
	funcion(tipo_var tipo_ret, string (*af)(string *args), tipo_var tipo_arg_1, tipo_var tipo_arg_2):cant_arg(2),func(af) { 
		tipos.resize(cant_arg+1); tipos[0]=tipo_ret; tipos[1]=tipo_arg_1; tipos[2]=tipo_arg_2;
	}
	funcion(tipo_var tipo_ret, string (*af)(string *args), tipo_var tipo_arg_1, tipo_var tipo_arg_2, tipo_var tipo_arg_3):cant_arg(3),func(af) { 
		tipos.resize(cant_arg+1); tipos[0]=tipo_ret; tipos[1]=tipo_arg_1; tipos[2]=tipo_arg_2; tipos[3]=tipo_arg_3;
	}
};

extern map<string,funcion> funciones;
extern map<string,funcion> subprocesos;

void LoadFunciones(bool u=true);
funcion* EsFuncion(const string nombre);

#endif

