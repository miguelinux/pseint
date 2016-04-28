#ifndef NEW_FUNCIONES_H
#define NEW_FUNCIONES_H
#include <string>
#include <map>
#include "new_memoria.h"
#include <vector>
#include "DataValue.h"
using namespace std;

#define FUNC_MAX_NUM_ARGS 3

enum PASAJE {PP_VALOR, PP_REFERENCIA, PP_DESCONOCIDO};

// cppcheck-suppress noCopyConstructor
struct Funcion {
	string id; // nombre de la función
	int cant_arg; // cantidad de argumentos (sin contar el valor de retorno, los argumentos seran nombres[1...cant_arg])
	vector<PASAJE> pasajes; // tipo de pasaje de parametros (solo para las definidas por el usuario)
	vector<tipo_var> tipos; // la pos 0 es para el tipo que retorna
	const tipo_var &GetTipo(int i) const { return tipos[i]; } // para acceder a los tipos desde punteros const
	vector<string> nombres; // nombres de los argumentos, la pos 0 es para el valor de retorno (solo para las definidas por el usuario, es "" si no retorna nada)
	DataValue (*func)(DataValue *args); // NULL si es de las definidas por el usuario como subproceso
	int line_start; // linea del pseudocodigo parseado donde comienza la funcion (solo para las definidas por el usuario)
	int userline_start, userline_end; // linea del pseudocodigo original donde empieza y termina la funcion (para pasarsela a la lista de variables del editor)
	void AddArg(string arg, PASAJE por=PP_DESCONOCIDO) { nombres.push_back(arg); tipos.push_back(vt_desconocido); pasajes.push_back(por); cant_arg++; }
	void SetLastPasaje(PASAJE por) { pasajes[pasajes.size()-1]=por; } // para modificar el tipo de pasaje del último argumento insertado con AddArg
	Memoria *memoria; // instancia de la clase memoria que se usa para el analisis sintático
	Funcion(int line):func(NULL),line_start(line),userline_start(-1),userline_end(-1) { 
		AddArg(""); cant_arg=0; memoria=NULL;
	}
	// cppcheck-suppress uninitMemberVar
	Funcion(tipo_var tipo_ret, DataValue (*af)(DataValue *args)):cant_arg(0),func(af) { 
		tipos.resize(cant_arg+1); tipos[0]=tipo_ret;
		pasajes.resize(cant_arg+1); pasajes[0]=PP_DESCONOCIDO;
	}
	// cppcheck-suppress uninitMemberVar
	Funcion(tipo_var tipo_ret, DataValue (*af)(DataValue *args), tipo_var tipo_arg_1):cant_arg(1),func(af) { 
		tipos.resize(cant_arg+1); tipos[0]=tipo_ret; tipos[1]=tipo_arg_1;
		pasajes.resize(cant_arg+1); pasajes[0]=pasajes[1]=PP_DESCONOCIDO;
	}
	// cppcheck-suppress uninitMemberVar
	Funcion(tipo_var tipo_ret, DataValue (*af)(DataValue *args), tipo_var tipo_arg_1, tipo_var tipo_arg_2):cant_arg(2),func(af) { 
		tipos.resize(cant_arg+1); tipos[0]=tipo_ret; tipos[1]=tipo_arg_1; tipos[2]=tipo_arg_2;
		pasajes.resize(cant_arg+1); pasajes[0]=pasajes[1]=pasajes[2]=PP_DESCONOCIDO;
	}
	// cppcheck-suppress uninitMemberVar
	Funcion(tipo_var tipo_ret, DataValue (*af)(DataValue *args), tipo_var tipo_arg_1, tipo_var tipo_arg_2, tipo_var tipo_arg_3):cant_arg(3),func(af) { 
		tipos.resize(cant_arg+1); tipos[0]=tipo_ret; tipos[1]=tipo_arg_1; tipos[2]=tipo_arg_2; tipos[3]=tipo_arg_3;
		pasajes.resize(cant_arg+1); pasajes[0]=pasajes[1]=pasajes[2]=pasajes[3]=PP_DESCONOCIDO;
	}
};

extern map<string,Funcion*> funciones; ///< funciones predefinidas del lenguaje
extern map<string,Funcion*> subprocesos; ///< funciones definidas por el usuario (proceso y subprocesos)

void LoadFunciones(); ///< carga la lista de funciones predefinidas (los nombres se cargan en mayusculas)
const Funcion* EsFuncion(const string &nombre, bool include_main_process=false); ///< busca una funcion (predefinida o subproceso del usuario), devuelve NULL si no la encuetra
const Funcion* EsFuncionPredefinida(const string &nombre); ///< busca una funcion predefinida, devuelve NULL si no la encuetra
const Funcion* EsFuncionDelUsuario(const string &nombre, bool include_main_process); ///< busca una funcion predefinida, devuelve NULL si no la encuetra
string GetNombreFuncion(const Funcion *func); ///< obtiene el nombre de la función a partir de un puntero (para mostrar en los mensajes de error)
void UnloadSubprocesos(); ///< borra de la memoria todos los subprocesos definidos por el usuario
void UnloadFunciones(); ///< borra de la memoria todas las funciones predefinidas (nunca es necesario, solo para depurar el rtsyntax con memcheck y evitar falsos positivos)

extern string main_process_name; ///< nombre de la funcion que representa al proceso principal, se asigna en SynCheck y se usa en el main para saber desde donde ejecutar

#endif

