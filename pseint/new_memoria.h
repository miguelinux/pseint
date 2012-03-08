#ifndef NEW_MEMORIA_H
#define NEW_MEMORIA_H
#include <cstddef>
#include <map>
#include <string>
#include <iostream>
#include <list>
#include <sstream>
#include <set>
using namespace std;

struct tipo_var {
	int *dims;
	bool enabled; // para que queden registradas luego del primer parseo, pero actue como si no existieran
	bool cb_log,cb_num,cb_car;
	bool rounded; // para cuando se definen como enteras
	tipo_var():dims(NULL),enabled(true),cb_log(true),cb_num(true),cb_car(true),rounded(false) {}
	tipo_var(bool l, bool n, bool c):dims(NULL),enabled(true),cb_log(l),cb_num(n),cb_car(c),rounded(false) {}
	bool set(const tipo_var &v) {
		enabled=true;
		cb_log=cb_log&&v.cb_log;
		cb_num=cb_num&&v.cb_num;
		cb_car=cb_car&&v.cb_car;
		return (cb_car?1:0)+(cb_log?1:0)+(cb_num?1:0)!=0;
	}
	bool set(const tipo_var &v, bool) {
		enabled=true;
		bool error = ((cb_log&&v.cb_log)?1:0)+((cb_num&&v.cb_num)?1:0)+((cb_car&&v.cb_car)?1:0)==0;
		if (!error) {
			cb_log=cb_log&&v.cb_log;
			cb_num=cb_num&&v.cb_num;
			cb_car=cb_car&&v.cb_car;
			return true;
		} else
			return false;
	}
	bool is_known() {
		return (cb_car?1:0)+(cb_log?1:0)+(cb_num?1:0)==1;
	}
	bool is_ok() {
		return (cb_car?1:0)+(cb_log?1:0)+(cb_num?1:0)!=0;
	}
	bool operator==(const tipo_var &t) {
		return cb_car==t.cb_car&&cb_num==t.cb_num&&cb_log==t.cb_log;
	}
	bool operator!=(const tipo_var &t) {
		return cb_car!=t.cb_car||cb_num!=t.cb_num||cb_log!=t.cb_log;
	}
	bool can_be(const tipo_var &t) {
		return (cb_car&&t.cb_car) || (cb_num&&t.cb_num) || (cb_log&&t.cb_log);
	}
	tipo_var &operator=(const tipo_var &t) {
		cb_log=t.cb_log;
		cb_num=t.cb_num;
		cb_car=t.cb_car;
		dims=t.dims;
		return *this;
	}
};

extern tipo_var vt_error;
extern tipo_var vt_desconocido;
extern tipo_var vt_logica;
extern tipo_var vt_numerica;
extern tipo_var vt_caracter;
extern tipo_var vt_caracter_o_numerica;
extern tipo_var vt_caracter_o_logica;

class Memoria {
	map<string,tipo_var> var_info;
	map<string,string> var_value;
	set<string> var_defined;
	friend void declarar_variables(list<string> &prog, bool &use_sin_tipo, bool &use_string);
	void QuitarIndices(string &str) {
		for (int i=0;i<int(str.size());i++)
			if (str[i]=='['||str[i]=='(') {
				str.erase(i);
				break;
			}
	}
public:
	void FakeReset() {
		map<string,tipo_var>::iterator it = var_info.begin();
		while (it!=var_info.end()) (it++)->second.enabled=false;
		var_defined.clear(); var_value.clear();
	}
	void Agregartipo_var(string nombre, const tipo_var &tipo=vt_desconocido) {
		tipo_var &v = var_info[nombre];
		v.set(tipo);
	}
	void AgregarArreglo(string nombre, int *dims) {
		tipo_var &v = var_info[nombre];
		v.dims=dims;
	}
	bool DefinirTipo(string nombre, const tipo_var &tipo) {
		QuitarIndices(nombre);
		return var_info[nombre].set(tipo,true);
	}
	bool DefinirTipo(string nombre, const tipo_var &tipo, bool rounded) {
		QuitarIndices(nombre);
		if (rounded) var_info[nombre].rounded=true;
		var_defined.insert(nombre);
		return var_info[nombre].set(tipo,true);
	}
	void EscribirValor(string nombre, string valor) {
		string nom=nombre; QuitarIndices(nom);
		var_info[nom].enabled=true;
		if (var_info[nom].rounded) { 
			size_t p=valor.find(".");
			if (p!=string::npos) valor.erase(p);
		}
		var_value[nombre]=valor;
	}
	int *LeerDims(string nombre) {
		QuitarIndices(nombre);
		return var_info[nombre].dims;
	}
	tipo_var LeerTipo(string nombre) {
		QuitarIndices(nombre);
		return var_info[nombre];
	}
	bool Existe(string nombre) {
		QuitarIndices(nombre);
		map<string,tipo_var>::iterator it_info = var_info.find(nombre);
		return it_info!=var_info.end() && it_info->second.enabled;
	}
	bool EstaInicializada(string nombre) {
		map<string,string>::iterator it_value = var_value.find(nombre);
		return it_value!=var_value.end();
	}
	bool EstaDefinida(string nombre) {
		QuitarIndices(nombre);
		return var_defined.find(nombre)!=var_defined.end();
	}
	string LeerValor(string nombre) {
		// verificar aca si esta inicializada??
		string ret=var_value[nombre];
		if (ret.size()==0) {
			string tnombre=nombre; QuitarIndices(tnombre);
			tipo_var &t = var_info[tnombre];
			if (t==vt_numerica)
				return "0";
			else if (t==vt_logica)
				return "FALSO";
			else
				return "";
		}
		return ret;
	}
};

extern Memoria *memoria;

#endif

