#ifndef EXPORT_VB_H
#define EXPORT_VB_H
#include "export_common.h"
#include "new_memoria.h"
using namespace std;

class VbExporter : public ExporterBase {
	
	string get_tipo(map<string,tipo_var>::iterator &mit, bool for_func=false, bool by_ref=false); // se usa tanto desde el otro get_tipo como desde declarar_variables
	void declarar_variables(t_output &prog);
	string get_tipo(string name,bool by_ref=false); // solo se usa para cabeceras de funciones
	void translate_single_proc(t_output &out, Funcion *f, t_proceso &proc);
	
	void invocar(t_output &prog, string param, string tabs);
	void esperar_tiempo(t_output &prog, string tiempo, bool mili, string tabs);
	void esperar_tecla(t_output &prog, string param,string tabs);
	void borrar_pantalla(t_output &prog, string param,string tabs);
	void escribir(t_output &prog, t_arglist args, bool saltar, string tabs);
	void leer(t_output &prog, t_arglist args, string tabs);
	void asignacion(t_output &prog, string param1, string param2, string tabs);
	void si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs);
	void mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	void segun(t_output &prog, list<t_proceso_it> its, string tabs);
	void repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	void para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	void paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	void dimension(t_output &prog, t_arglist &args, string tabs);
	void comentar(t_output &prog, string text, string tabs);
	
public:
	string make_string(string cont);
	string function(string name, string args);
	string get_constante(string name);
	string get_operator(string op, bool for_string=false);
	void translate(t_output &out, t_programa &prog);
	VbExporter();
	
};

#endif

