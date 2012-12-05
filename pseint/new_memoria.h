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

class Memoria;

struct alias {
	string nom;
	Memoria *mem;
	alias():mem(NULL){}
	alias(string n, Memoria *m):nom(n),mem(m){}
};

struct tipo_var {
	int *dims; // dims[0] es la cantidad de dimensiones, dims[1...] son las dimensiones propiamente dichas
	bool enabled; // para que queden registradas luego del primer parseo, pero actue como si no existieran
	bool cb_log,cb_num,cb_car; // si puede ser logica, numerica o caracter
	bool rounded; // para cuando se definen como enteras
	bool defined; // para saber si fueron definidas explicitamente (definir...)
	bool used; // para saber si fue usada, asignada, leida, algo que no sea dimensionada o definida explicitamente, lo setean Escribir y LeerValor
	tipo_var():dims(NULL),enabled(true),cb_log(true),cb_num(true),cb_car(true),rounded(false),defined(false),used(false) {}
	tipo_var(bool l, bool n, bool c, bool r=false):dims(NULL),enabled(true),cb_log(l),cb_num(n),cb_car(c),rounded(r),defined(false),used(false) {}
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
	void reset() { // para borrar la información que genera el analisis sintáctico antes de la ejecución y que no debe pasar a la ejecución
		defined=used=enabled=false;
		if (dims) { delete [] dims; dims=NULL; }
	}
};

extern tipo_var vt_error;
extern tipo_var vt_desconocido;
extern tipo_var vt_logica;
extern tipo_var vt_numerica;
extern tipo_var vt_caracter;
extern tipo_var vt_caracter_o_numerica;
extern tipo_var vt_caracter_o_logica;
extern tipo_var vt_numerica_entera;

class Funcion;
class Memoria {
	
	map<string,alias> var_alias; // lista de aliases, para el pasaje por referencia
	string alias_nom; Memoria *alias_mem;
	
	map<string,tipo_var> var_info;
	map<string,string> var_value;
	friend void declarar_variables(list<string> &prog, bool &use_sin_tipo, bool &use_string);
	void QuitarIndices(string &str) {
		for (int i=0;i<int(str.size());i++)
			if (str[i]=='['||str[i]=='(') {
				str.erase(i);
				break;
			}
	}
	bool EsAlias(const string &s) { // mira si la variable que recibe es un alias, setea it_alias
		map<string,alias>::iterator it_alias=var_alias.find(s);
		if (it_alias==var_alias.end()) return false;
		alias_nom=it_alias->second.nom;
		alias_mem=it_alias->second.mem;
		return true;
	}
	bool EsAlias(const string &s, bool) { // mira si la expresion que recibe es un alias (puede tener dimensiones, las quita si es asi), setea it_alias
		string s2=s; QuitarIndices(s2);
		map<string,alias>::iterator it_alias=var_alias.find(s2);
		if (it_alias==var_alias.end()) return false;
		alias_nom=it_alias->second.nom+s.substr(s2.size());
		alias_mem=it_alias->second.mem;
		return true;
	}
	Funcion *funcion; // scope
public:
	Memoria(Funcion *parent):funcion(parent){} // guarda el puntero a su scope para usar en EsArgumento
	bool EsArgumento(string nom); // determina si un nombre dado es uno de los argumentos de la función actual o no
	void HardReset() {
		var_value.clear();
		var_info.clear();
	}
	void FakeReset() {
		map<string,tipo_var>::iterator it = var_info.begin();
		while (it!=var_info.end())
			(it++)->second.reset();
		var_value.clear();
	}
	void Agregartipo_var(string nombre, const tipo_var &tipo=vt_desconocido) {
		tipo_var &v = var_info[nombre];
		v.set(tipo);
	}
	void AgregarArreglo(string nombre, int *dims) {
		tipo_var &v = var_info[nombre];
		v.dims=dims;
	}
	// esta version de definir tipo se usa en las definiciones implicitas
	void AgregarAlias(string nom_here, string nom_orig, Memoria *mem) {
		var_alias[nom_here]=alias(nom_orig,mem);
		var_info[nom_here]=mem->var_info[nom_orig];
		if (nom_orig.find('(')!=string::npos) var_info[nom_here].dims=NULL;
	}
	bool DefinirTipo(string nombre, const tipo_var &tipo) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->DefinirTipo(alias_nom,tipo);
		tipo_var &vi=var_info[nombre];
		vi.defined=true;
		return vi.set(tipo,true);
	}
	// esta version de definir tipo se usa en las definiciones explictas
	void DefinirTipo(string nombre, const tipo_var &tipo, bool rounded) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->DefinirTipo(alias_nom,tipo,rounded);
		tipo_var &vi=var_info[nombre];
		if (rounded) vi.rounded=true;
		vi.defined=true;
		vi.cb_car=tipo.cb_car;
		vi.cb_log=tipo.cb_log;
		vi.cb_num=tipo.cb_num;
	}
	void EscribirValor(string nombre, string valor) {
		if (EsAlias(nombre,true)) return alias_mem->EscribirValor(alias_nom,valor);
		string nom=nombre; QuitarIndices(nom);
		tipo_var &vi = var_info[nom];
		vi.enabled=true;
		if (vi.rounded) { 
			size_t p=valor.find(".");
			if (p!=string::npos) valor.erase(p);
		}
		vi.used=true;
		var_value[nombre]=valor;
	}
	int *LeerDims(string nombre) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) {
			if (alias_nom.find('(')!=string::npos) // si el original era una arreglo, pero el alias apunta solo a una posicion
				return NULL;
			else
				return alias_mem->LeerDims(alias_nom);
		}
		return var_info[nombre].dims;
	}
	tipo_var LeerTipo(string nombre) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->LeerTipo(alias_nom);
		tipo_var &vi = var_info[nombre];
		return vi;
	}
	bool Existe(string nombre) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->Existe(alias_nom);
		map<string,tipo_var>::iterator it_info = var_info.find(nombre);
		if (it_info==var_info.end()) return false;
//		tipo_var &vi=it_info->second;
		return it_info->second.enabled;
	}
	bool HaSidoUsada(string nombre) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->HaSidoUsada(alias_nom);
		map<string,tipo_var>::iterator it_info = var_info.find(nombre);
		if (it_info==var_info.end()) return false;
//		tipo_var &vi=it_info->second;
		return it_info->second.enabled && it_info->second.used;
	}
	bool EstaInicializada(string nombre) {
		string nom=nombre; QuitarIndices(nom);
		if (EsAlias(nombre,true)) return alias_mem->EstaInicializada(alias_nom);
		map<string,tipo_var>::iterator it_info = var_info.find(nom);
		if (it_info==var_info.end()) return false;
//		tipo_var &vi=it_info->second;
		map<string,string>::iterator it_value = var_value.find(nombre);
		return it_value!=var_value.end();
	}
	bool EstaDefinida(string nombre) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->EstaDefinida(alias_nom);
		map<string,tipo_var>::iterator it_info = var_info.find(nombre);
		if (it_info==var_info.end()) return false;
//		tipo_var &vi=it_info->second;
		return  var_info[nombre].defined;
	}
	string LeerValor(string nombre) {
		if (EsAlias(nombre,true)) return alias_mem->LeerValor(alias_nom);
		string ret=var_value[nombre];
		if (ret.size()==0) {
			string nom=nombre; QuitarIndices(nom);
			tipo_var &vi = var_info[nom];
			vi.used=true; // fuera del if no hace falta porque si ret!=0 es porque paso por EscribirValor, y entonces ya esta used=true
			if (vi==vt_numerica)
				return "0";
			else if (vi==vt_logica)
				return "FALSO";
			else
				return "";
		}
		return ret;
	}
	void ListVars() { // para que el proceso de rt_syntax le pase a la gui la lista de variables
		map<string,tipo_var>::iterator it=var_info.begin(), it2=var_info.end();
		while (it!=it2) {
			cout<<it->first;
			if (it->second.dims&&it->second.dims[0]>0) {
				cout<<"[";
				for (int i=1;i<it->second.dims[0];i++)
					cout<<it->second.dims[i]<<",";
				cout<<it->second.dims[it->second.dims[0]]<<"]";
			}
			cout<<" "<<(it->second.cb_log?1:0)+(it->second.cb_num?2:0)+(it->second.cb_car?4:0)<<endl;;
			it++;
		}
		
	}
	
};

extern Memoria *memoria;

#endif

