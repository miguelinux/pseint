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
	Memoria *memoria; // instancia de la clase memoria que se usa para el analisis sintático
	funcion(){}
//	~funcion() { if (!func) delete memoria; } // esto trae problemas si se copian funcs, por eso se borran aparte, es menos lio que el ctor de copia correcto
	funcion(int line):func(NULL),line_start(line) { 
		AddArg(""); cant_arg=0; memoria=new Memoria;
	}
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

extern map<string,funcion> funciones; ///< funciones predefinidas del lenguaje
extern map<string,funcion> subprocesos; ///< funciones definidas por el usuario (proceso y subprocesos)

void LoadFunciones(bool u=true); ///< carga la lista de funciones predefinidas, el argumento es para saber si van en mayúsculas o minúsculas
funcion* EsFuncion(const string nombre, bool include_main_process=false); ///< busca una funcion (predefinida o subproceso del usuario), devuelve NULL si no la encuetra
void UnloadSubprocesos(); ///< carga la lista de funciones predefinidas, el argumento es para saber si van en mayúsculas o minúsculas

extern string main_process_name; ///< nombre de la funcion que representa al proceso principal, se asigna en SynCheck y se usa en el main para saber desde donde ejecutar

#endif

