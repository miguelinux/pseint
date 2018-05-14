#ifndef NEW_MEMORIA_H
#define NEW_MEMORIA_H
#include <cstddef>
#include <map>
#include <string>
#include <iostream>
#include <list>
#include <sstream>
#include <set>
#include "new_memoria_inter.h"
#include "DataValue.h"
using namespace std;

class Memoria;

struct alias {
	string nom;
	Memoria *mem;
	alias():mem(NULL){}
	alias(string n, Memoria *m):nom(n),mem(m){}
};

// cppcheck-suppress noConstructor
class Funcion;

class Memoria {
	
	map<string,alias> var_alias; // lista de aliases, para el pasaje por referencia
	string alias_nom; Memoria *alias_mem;
	
	map<string,tipo_var> var_info;
	map<string,DataValue> var_value;
	void QuitarIndices(string &str) {
		int sz=str.size();
		for (int i=0;i<sz;i++)
			if (str[i]=='['||str[i]=='(') {
				str.erase(i);
				break;
			}
#ifdef _FOR_PSEXPORT
			else str[i]=toupper(str[i]);
#endif
		
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
	const Funcion *funcion; // scope
public:
	// cppcheck-suppress uninitMemberVar
	Memoria(const Funcion *parent):funcion(parent){} // guarda el puntero a su scope para usar en EsArgumento
	bool EsArgumento(const string &nom) const; // determina si un nombre dado es uno de los argumentos de la función actual o no
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
//	void Agregartipo_var(const string &nombre, const tipo_var &tipo=vt_desconocido) {
//		tipo_var &v = var_info[nombre];
//		v.set(tipo);
//	}
	void AgregarArreglo(string nombre, int *dims) {
#ifdef _FOR_PSEXPORT
		for(unsigned int i=0;i<nombre.size();i++) nombre[i]=toupper(nombre[i]);
#endif
		tipo_var &v = var_info[nombre];
		v.dims=dims;
	}
	// esta version de definir tipo se usa en las definiciones implicitas
	void AgregarAlias(const string &nom_here, const string &nom_orig, Memoria *mem) {
		var_alias[nom_here]=alias(nom_orig,mem);
		var_info[nom_here]=mem->var_info[nom_orig];
		if (nom_orig.find('(')!=string::npos) var_info[nom_here].dims=NULL;
	}
	// esta version de definir tipo se usa en las definiciones implicitas
	bool DefinirTipo(string nombre, const tipo_var &tipo) {
#ifdef _FOR_PSEXPORT
		// esto es porque el parseo que hace la funcion expresion pasa todo a minuscula,
		// pero despues los i_* pueden llamar a DefinirTipo por su cuenta con la expresion 
		// ya parseada y pasada a minusculas
		for(unsigned int i=0;i<nombre.size();i++) nombre[i]=toupper(nombre[i]);
#endif
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->DefinirTipo(alias_nom,tipo);
		tipo_var &vi=var_info[nombre];
//		vi.defined=true; // en las implicitas esto no va
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
	void EscribirValor(const string &nombre, const DataValue &valor) {
		if (EsAlias(nombre,true)) return alias_mem->EscribirValor(alias_nom,valor);
		string nom=nombre; QuitarIndices(nom);
		tipo_var &vi = var_info[nom];
		vi.enabled=true;
		vi.used=true;
		if (vi.rounded) {
			/// @todo: ver si esto puede pasar, deberia controlarse antes y dar un error
			var_value[nombre].SetFromInt(valor.GetAsInt());
		}
		var_value[nombre].SetValue(valor);
	}
	const int *LeerDims(string nombre) {
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
	bool EstaInicializada(const string &nombre) {
		string nom=nombre; QuitarIndices(nom);
		if (EsAlias(nombre,true)) return alias_mem->EstaInicializada(alias_nom);
		map<string,tipo_var>::iterator it_info = var_info.find(nom);
		if (it_info==var_info.end()) return false;
//		tipo_var &vi=it_info->second;
		return var_value.count(nombre);
	}
	bool EstaDefinida(string nombre) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->EstaDefinida(alias_nom);
		map<string,tipo_var>::iterator it_info = var_info.find(nombre);
		if (it_info==var_info.end()) return false;
//		tipo_var &vi=it_info->second;
		return  var_info[nombre].defined;
	}
	void Desinicializar(string nombre) {
		QuitarIndices(nombre);
		if (EsAlias(nombre)) return alias_mem->Desinicializar(alias_nom);
		var_value.erase(nombre);
	}
	DataValue LeerValor(const string &nombre) {
		if (EsAlias(nombre,true)) return alias_mem->LeerValor(alias_nom);
		DataValue ret = var_value[nombre];
		if (ret.IsEmpty()) {
			string nom = nombre; QuitarIndices(nom);
			tipo_var &vi = var_info[nom];
			vi.used=true; // fuera del if no hace falta porque si ret!=0 es porque paso por EscribirValor, y entonces ya esta used=true
		}
		return ret;
	}
	DataValue Leer(const string &nombre) {
		DataValue res = LeerValor(nombre);
		res.type = LeerTipo(nombre);
		return res;
	}
	void ListVars(map<string,string> *case_map) { // para que el proceso de rt_syntax le pase a la gui la lista de variables
		map<string,tipo_var>::iterator it=var_info.begin(), it2=var_info.end();
		while (it!=it2) {
			if (it->first=="") { ++it; continue; } // cuando hay errores de sintaxis, pueden salir variables sin nombre
			if (case_map) cout<<(*case_map)[it->first];
			else cout<<it->first;
			if (it->second.dims&&it->second.dims[0]>0) {
				cout<<"[";
				for (int i=1;i<it->second.dims[0];i++)
					cout<<it->second.dims[i]<<",";
				cout<<it->second.dims[it->second.dims[0]]<<"]";
			}
			cout<<" "<<char(LV_BASE_CHAR+(
				(it->second.cb_log?LV_LOGICA:0)+
				(it->second.cb_num?LV_NUMERICA:0)+
				(it->second.cb_car?LV_CARACTER:0)+
				(it->second.defined?LV_DEFINIDA:0)
				))<<endl;;
			++it;
		}
		
	}
	
#ifdef _FOR_PSEXPORT
	map<string,tipo_var> &GetVarInfo() { return var_info; }
	void RemoveVar(string nombre) {
		for(unsigned int i=0;i<nombre.size();i++) nombre[i]=toupper(nombre[i]);
		if (var_info.find(nombre)!=var_info.end()) var_info.erase(var_info.find(nombre));
		if (var_value.find(nombre)!=var_value.end()) var_value.erase(var_value.find(nombre));
	}
#endif
	~Memoria() {
		for(map<string,tipo_var>::iterator it1=var_info.begin(), it2=var_info.end();it1!=it2;++it1)
			if (it1->second.dims && var_alias.count(it1->first)==0)
				delete [] it1->second.dims;
	}
	
private:
	Memoria(const Memoria &); // forbidden
	Memoria &operator=(const Memoria &);  // forbidden
};

extern Memoria *memoria;

#endif

