#ifndef EXPORT_PASCAL_H
#define EXPORT_PASCAL_H

#include "export_common.h"
#include "new_memoria.h"
using namespace std;

class PascalExporter:public ExporterBase {
	
protected:
	
	bool uses_crt;
	bool uses_sin_tipo;
	bool uses_math;
	bool uses_sysutils;
	bool uses_randomize;
	bool has_matrix_func;
	bool use_arreglo_max;
	
	virtual string get_tipo(map<string,tipo_var>::iterator &mit, bool for_func=false, bool by_ref=false); // se usa tanto desde el otro get_tipo como desde declarar_variables
	virtual void declarar_variables(t_output &prog);
	virtual string get_tipo(string name, bool by_ref=false, bool do_erase=true); // solo se usa para cabeceras de funciones
	virtual void header(t_output &out);
	virtual void translate_single(t_output &out, t_proceso &proc);
	
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
	
public:
	virtual string make_string(string cont);
	virtual string function(string name, string args);
	virtual string get_constante(string name);
	virtual string get_operator(string op, bool for_string=false);	
	virtual void translate(t_output &out, t_programa &prog);
	PascalExporter();
	
};

#endif

