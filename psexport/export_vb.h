#ifndef EXPORT_VB_H
#define EXPORT_VB_H
#include "export_common.h"
#include "new_memoria.h"
using namespace std;

class VbExporter:public ExporterBase {
	
	string get_tipo(map<string,tipo_var>::iterator &mit);
	void declarar_variables(t_output &prog);
	string get_tipo(string name);
	void translate(t_output &out, t_proceso &proc);
		
	
	void esperar(t_output &prog, string param, string tabs);
	void borrar(t_output &prog, string param, string tabs);
	void invocar(t_output &prog, string param, string tabs);
	void escribir(t_output &prog, string param, string tabs);
	void leer(t_output &prog, string param, string tabs);
	void asignacion(t_output &prog, string param1, string param2, string tabs);
	void si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs);
	void mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	void segun(t_output &prog, list<t_proceso_it> its, string tabs);
	void repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	void para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	void paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	
	string function(string name, string args);

public:
	void translate(t_output &prog, t_programa &alg);
	VbExporter();
};

#endif

