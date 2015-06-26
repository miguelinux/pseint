#ifndef EXPORT_CS_H
#define EXPORT_CS_H
#include "export_cpp.h"

class CSharpExporter : public CppExporter {
	
	bool use_random;
	bool use_threading;
	bool have_subprocesos;
	bool use_reference;
	
	string get_tipo(string name, bool by_ref, bool do_erase); // se usa para argumentos de funciones
	string get_tipo(map<string,tipo_var>::iterator &mit, bool for_func=false, bool by_ref=false); // se usa tanto desde el otro get_tipo como desde declarar_variables
	void header(t_output &out);
	void footer(t_output &out);
	void translate_single_proc(t_output &out, Funcion *f, t_proceso &proc);
	void translate_all_procs(t_output &out, t_programa &prog, string tabs) override;
	string translate_tipo(const tipo_var &t);
	
	void dimension(t_output &prog, t_arglist &args, string tabs);
	void esperar_tiempo(t_output &prog, string tiempo, bool mili, string tabs);
	void esperar_tecla(t_output &prog, string param,string tabs);
	void borrar_pantalla(t_output &prog, string param,string tabs);
//	void invocar(t_output &prog, string param, string tabs);
	void escribir(t_output &prog, t_arglist args, bool saltar, string tabs);
	void leer(t_output &prog, t_arglist args, string tabs);
//	void asignacion(t_output &prog, string param1, string param2, string tabs);
//	void si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s, string tabs);
//	void mientras(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
//	void segun(t_output &prog, list<t_proceso_it> its, string tabs);
//	void repetir(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
//	void para(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	void paracada(t_output &prog, t_proceso_it r, t_proceso_it q, string tabs);
	
public:
	string function(string name, string args);
	string get_constante(string name);
	string get_operator(string op, bool for_string=false);	
//	void translate(t_output &out, t_programa &prog);
	CSharpExporter();
	
};

#endif

