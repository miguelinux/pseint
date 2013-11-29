#ifndef EXPORT_COMMON_H
#define EXPORT_COMMON_H
#include <string>
#include "defines.h"
using namespace std;

class ExporterBase {

protected:
	
	// funciones de utilería para las demás
	void bloque(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs);
	string get_arg(string args, int cual);
	string make_dims(const int *tdims, string c1="[", string c2=",", string c3="]", bool numbers=true);
	
	// funciones que traducen instrucciones y estructuras de control
	virtual void invocar(t_output &prog, string param,string tabs)=0;
	virtual void esperar(t_output &prog, string param,string tabs)=0;
	virtual void borrar(t_output &prog, string param,string tabs)=0;
	virtual void escribir(t_output &prog, t_arglist args, bool saltar, string tabs)=0;
	virtual void leer(t_output &prog, t_arglist args, string tabs)=0;
	virtual void asignacion(t_output &prog, string param1, string param2,string tabs)=0;
	virtual void si(t_output &prog, t_proceso_it r, t_proceso_it q, t_proceso_it s,string tabs)=0;
	virtual void mientras(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs)=0;
	virtual void segun(t_output &prog, list<t_proceso_it> its,string tabs)=0;
	virtual void repetir(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs)=0;
	virtual void paracada(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs)=0;
	virtual void para(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs)=0;
	virtual void dimension(t_output &prog, string params, string tabs);
	virtual void definir(t_output &prog, string param, string tabs);

public:
	
	// funciones para traducir expresiones
	
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
	* @param prog    argumento de salida, donde colocar las instrucciones traducidas
	* @param alg     argumento de entrada, con el algoritmo ya parseado
	**/
	virtual void translate(t_output &prog, t_programa &alg)=0;
	
};

extern ExporterBase *exporter;

#endif

