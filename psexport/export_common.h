#ifndef EXPORT_COMMON_H
#define EXPORT_COMMON_H
#include <string>
#include "defines.h"
#include <map>
#include <stack>
#include <vector>
class Memoria;
class Funcion;
using namespace std;

#if __cplusplus >= 201103
#define _CPP11_ENABLED 1
#else

// missing keywords
#define final
#define override

#endif // cpp11

class ExporterBase {

	static map<string,Memoria*> mapa_memorias;
	
protected:
	
	void set_memoria(string key);
	
	void replace_var(t_output &out, string src, string dst);

	// funciones de utilería para las demás
	vector<string> aux_varnames;
	virtual string get_aux_varname(string pref);
	virtual void release_aux_varname(string vname);
	void init_header(t_output &out, string comment_pre, string comment_post="");
	void bloque(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs);
	string get_arg(string args, int cual);
	string make_dims(const int *tdims, string c1="[", string c2=",", string c3="]", bool numbers=true);
	void crop_name_and_dims(string decl, string &name, string &dims, string c1="[", string c2=",", string c3="]");
	bool es_cadena_constante(const string &s);
	bool es_numerica_constante(const string &s);
	bool es_numerica_entera_constante(const string &s);
	void sep_args(const string &args, t_arglist &out);
	bool replace_all(string &str, string from, string to);
	
	// funciones que traducen instrucciones y estructuras de control
	virtual void esperar_tiempo(t_output &prog, string tiempo, bool mili, string tabs)=0;
	virtual void esperar_tecla(t_output &prog, string param,string tabs)=0;
	virtual void borrar_pantalla(t_output &prog, string param,string tabs)=0;
	virtual void invocar(t_output &prog, string param,string tabs)=0;
	virtual void escribir(t_output &prog, t_arglist args, bool saltar, string tabs)=0;
	virtual void leer(t_output &prog, t_arglist args, string tabs)=0;
	virtual void asignacion(t_output &prog, string param1, string param2,string tabs)=0;
	virtual void si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s,string tabs)=0;
	virtual void mientras(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs)=0;
	virtual void segun(t_output &prog, list<t_proceso_it> its,string tabs)=0;
	virtual void repetir(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs)=0;
	virtual void paracada(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs)=0;
	virtual void para(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs)=0;
	virtual void dimension(t_output &prog, t_arglist &args, string tabs);
	virtual void definir(t_output &prog, t_arglist &arglist, string tipo, string tabs);
	virtual void comentar(t_output &prog, string text, string tabs);
	
public:
	
	// funciones para traducir expresiones
	
	/**
	* @brief retorna un identificador de variable, convirtiendo todo a minuscula y agregando algun prefijo si es necesario (como el $ en php)
	*
	* @param nombre de la variable, puede ser en mayusculas o minusculas, pero el valor de retorno debe ser siempre case-insensitive
	**/
	virtual string make_varname(string varname);
	
	/**
	* @brief returns a literal string with the given contend, adding colons and escape chars if required
	*
	* @param the string content, without colons
	**/
	virtual string make_string(string cont)=0;
	
	/**
	* @brief traduce una constante
	*
	* @param name    nombre de la constante
	**/
	virtual string get_constante(string name)=0;
	
	/**
	* @brief traduce un operador
	*
	* @param name        nombre de la constante
	* @param for_string  indica si el operador se aplica sobre operandos de tipo string
	**/
	virtual string get_operator(string op, bool for_string=false)=0;
	
	/**
	* @brief traduce la llamada a una función predefinida
	*
	* @param name    nombre de la función en pseudocódigo
	* @param args    string con la lista de argumentos (incluye los paréntesis)
	**/
	virtual string function(string name, string args)=0;
	
	
	// función principal, que arma el esqueleto e invoca a las demas
	
	/**
	* @brief traduce el algoritmo
	*
	* @param out     argumento de salida, donde colocar las instrucciones traducidas
	* @param prog    argumento de entrada, con el algoritmo ya parseado
	**/
	virtual void translate(t_output &out, t_programa &prog)=0;
	
	virtual void translate_all_procs(t_output &out, t_programa &prog, string tabs="");
	virtual void translate_all_procs(t_output &out_main, t_output &out_procs, t_programa &prog, string tabs="");
	virtual void translate_single_proc(t_output &out, Funcion *f, t_proceso &proc);
	
};

extern ExporterBase *exporter;

#endif

