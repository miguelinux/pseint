#ifndef NEW_PROGRAMA_H
#define NEW_PROGRAMA_H
#include <string>
#include <vector>
#include "intercambio.h"
using namespace std;

enum InstructionType {
	IT_NULL, IT_ERROR,
	IT_PROCESO, IT_FINPROCESO, IT_SUBPROCESO, IT_FINSUBPROCESO,
	IT_LEER, IT_ASIGNAR, IT_ESCRIBIR, IT_ESCRIBIRNL, IT_DIMENSION, IT_DEFINIR,
	IT_ESPERARTECLA, IT_ESPERAR, IT_BORRARPANTALLA, IT_INVOCAR,
	IT_MIENTRAS, IT_FINMIENTRAS, IT_REPETIR, IT_HASTAQUE, IT_MIENTRASQUE,
	IT_SI, IT_ENTONCES, IT_SINO, IT_FINSI, IT_SEGUN, IT_OPCION, IT_DEOTROMODO, IT_FINSEGUN,
	IT_PARA, IT_PARACADA, IT_FINPARA
};

struct InstruccionLoc {
	int linea, inst;
	InstruccionLoc(int _linea, int _inst):linea(_linea),inst(_inst) {}
};

struct Instruccion {
	InstructionType type;
	string instruccion;
	int num_linea;
	int num_instruccion;
	Instruccion(string _instruccion, int _num_linea=-1, int _num_instruccion=-1)
		:type(IT_NULL),instruccion(_instruccion),num_linea(_num_linea),num_instruccion(_num_instruccion){}
	Instruccion(InstructionType _type, int _num_linea=-1, int _num_instruccion=-1)
		:type(_type),instruccion(""),num_linea(_num_linea),num_instruccion(_num_instruccion){}
	operator string() {return instruccion;}
//	bool operator==(const string &s) const { return instruccion==s; }
//	bool operator!=(const string &s) const { return instruccion!=s; }
	bool operator==(InstructionType t) const { return type==t; }
	bool operator!=(InstructionType t) const { return type!=t; }
	string &operator=(const string &s) { return instruccion=s; }
};

class Programa {
	vector<Instruccion> v;
	int cant_lines;
	int ref_point; // marcador para apuntar a una linea, y que PushBack y Erase lo actualicen si corresponde
public:
	Programa():cant_lines(0),ref_point(0) {}
	Instruccion &operator[](int i) { 
		return v[i];
	}
	void Insert(int pos,const string &inst, int num_line=-1, int num_inst=-1) { 
		if (ref_point>=pos) ref_point++;
		cant_lines++;
		if (pos) {
			if (num_line==-1) num_line=v[pos-1].num_linea;
			if (num_inst==-1) num_inst=v[pos-1].num_instruccion+1;
		}
		v.insert(v.begin()+pos,Instruccion(inst,num_line,num_inst));
	}
	void PushBack(string inst) { 
		if (ref_point>=int(v.size())) ref_point++; 
		v.push_back(Instruccion(inst,++cant_lines,1));
	}
	void Erase(int i) { 
		if (ref_point>=i) ref_point--; 
		v.erase(v.begin()+i); cant_lines--;
	}
	int GetSize() { 
		return cant_lines;
	}
	int GetInstSize() { 
		return v.size();
	}
//	Instruccion GetLoc(int x, string s) { 
//		return Instruccion(s,v[x].num_linea,v[x].num_instruccion);
//	}
	Instruccion GetLoc(int x, InstructionType t) { 
		return Instruccion(t,v[x].num_linea,v[x].num_instruccion);
	}
	void HardReset() { 
		v.clear(); cant_lines=0;
	}
	void SetRefPoint(int x=-1) { 
		ref_point=x;
	}
	int GetRefPoint() { 
		return ref_point;
	}
	static string aux_type2str(const string &inst, const string &args) {
		return args.empty()?inst:(args==";"?(inst+args):(inst+" "+args));
	}
	static string type2str(InstructionType type, const string &args) {
		if (type==IT_ERROR) return aux_type2str("ERROR",args);
		if (type==IT_PROCESO) return aux_type2str("PROCESO",args);
		if (type==IT_FINPROCESO) return aux_type2str("FINPROCESO",args);
		if (type==IT_SUBPROCESO) return aux_type2str("SUBPROCESO",args);
		if (type==IT_FINSUBPROCESO) return aux_type2str("FINSUBPROCESO",args);
		if (type==IT_LEER) return aux_type2str("LEER",args);
		if (type==IT_ASIGNAR) return args;
		if (type==IT_ESCRIBIR) return aux_type2str("ESCRIBIR",args);
		if (type==IT_ESCRIBIRNL) return aux_type2str("ESCRIBNL",args);
		if (type==IT_DIMENSION) return aux_type2str("DIMENSION",args);
		if (type==IT_DEFINIR) return aux_type2str("DEFINIR",args);
		if (type==IT_ESPERARTECLA) return aux_type2str("ESPERARTECLA",args);
		if (type==IT_ESPERAR) return aux_type2str("ESPERAR",args);
		if (type==IT_BORRARPANTALLA) return aux_type2str("BORRARPANTALLA",args);
		if (type==IT_INVOCAR) return aux_type2str("INVOCAR",args);
		if (type==IT_MIENTRAS) return aux_type2str("MIENTRAS",args);
		if (type==IT_FINMIENTRAS) return aux_type2str("FINMIENTRAS",args);
		if (type==IT_REPETIR) return aux_type2str("REPETIR",args);
		if (type==IT_HASTAQUE) return aux_type2str("HASTA QUE",args);
		if (type==IT_MIENTRASQUE) return aux_type2str("MIENTRAS QUE",args);
		if (type==IT_SI) return aux_type2str("SI",args);
		if (type==IT_ENTONCES) return aux_type2str("ENTONCES",args);
		if (type==IT_SINO) return aux_type2str("SINO",args);
		if (type==IT_FINSI) return aux_type2str("FINSI",args);
		if (type==IT_SEGUN) return aux_type2str("SEGUN",args);
		if (type==IT_OPCION) return args;
		if (type==IT_DEOTROMODO) return aux_type2str("DE OTRO MODO:",args);
		if (type==IT_FINSEGUN) return aux_type2str("FINSEGUN",args);
		if (type==IT_PARA) return aux_type2str("PARA",args);
		if (type==IT_PARACADA) return aux_type2str("PARACADA",args);
		if (type==IT_FINPARA) return aux_type2str("FINPARA",args);
		return args;
	}
};

extern Programa programa;

#endif

