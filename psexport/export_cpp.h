#ifndef EXPORT_CPP_H
#define EXPORT_CPP_H

#include "export_common.h"
#include "new_memoria.h"
using namespace std;

class CppExporter : public ExporterBase {
	
protected:
	
	bool include_cmath;
	bool include_cstdlib;
	bool use_sin_tipo;
	bool use_string;
	bool use_func_esperar;
	bool use_func_minusculas;
	bool use_func_mayusculas;
	bool use_func_convertiratexto;
	bool use_arreglo_max;
	bool has_matrix_func;
	bool read_strings;
	t_output prototipos; // forward declarations de las funciones
	
	virtual string convertirAString(const string &s);
	
	virtual string get_tipo(map<string,tipo_var>::iterator &mit, bool for_func=false, bool by_ref=false); // se usa tanto desde el otro get_tipo como desde declarar_variables
	virtual void declarar_variables(t_output &prog, string tab="\t", bool ignore_arrays=false);
	virtual string get_tipo(string name, bool by_ref=false, bool do_erase=true); // solo se usa para cabeceras de funciones
	virtual void header(t_output &out);
	virtual void footer(t_output &out);
	virtual void translate_single_proc(t_output &out, Funcion *f, t_proceso &proc);
	virtual void translate_all_procs(t_output &out, t_programa &prog, string tabs="") override;
	
	virtual void esperar_tiempo(t_output &prog, string tiempo, bool mili, string tabs);
	virtual void esperar_tecla(t_output &prog, string param,string tabs);
	virtual void borrar_pantalla(t_output &prog, string param,string tabs);
	virtual void invocar(t_output &prog, string param, string tabs);
	virtual void escribir(t_output &prog, t_arglist args, bool saltar, string tabs);
	virtual void leer(t_output &prog, t_arglist args, string tabs);
	virtual void asignacion(t_output &prog, string param1, string param2, string tabs);
	virtual void si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs);
	virtual void mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	virtual void segun(t_output &prog, list<t_proceso_it> its, string tabs);
	virtual void repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	virtual void para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	virtual void paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	virtual void comentar(t_output &prog, string text, string tabs);
	
public:
	virtual string make_string(string cont);
	virtual string function(string name, string args);
	virtual string get_constante(string name);
	virtual string get_operator(string op, bool for_string=false);	
	virtual void translate(t_output &out, t_programa &prog);
	CppExporter();
	
};

#endif

