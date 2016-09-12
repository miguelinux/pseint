#ifndef DATAVALUE_H
#define DATAVALUE_H
#include <string>
#include <cstdlib>
#include <cstdio>
#include "Variant.h"
#include <cassert>

#define FALSO "FALSO"
#define VERDADERO "VERDADERO"


inline double StrToDbl(const std::string &s) {
	return atof(s.c_str());
}

inline std::string DblToStr(double d) {
	char buf[512]; // DBL_MAX ocupa 310 caracteres
	sprintf(buf,"%.50f",d); // version de baja presición, se debería usar solo para mostrar
	// eliminar los ceros que sobren y el punto si era entero
	int i=0; while (buf[i]!=0) i++;
	while (buf[--i]=='0'); // contar ceros de atras para adelante
	if (buf[i]=='.') i--; // si llegamos al '.' sacarlo tambien
	if (buf[0]=='-'&&buf[1]=='0'&&i==1) buf[i=0]='0'; // si quedo "-0" sacar el -
	buf[++i]='\0'; // aplicar el corte
	return buf;
}

inline std::string DblToStr(double d, bool low) {
	char buf[512]; // DBL_MAX ocupa 310 caracteres
	sprintf(buf,"%.10f",d); // version de baja presición, se debería usar solo para mostrar
	// eliminar los ceros que sobren y el punto si era entero
	int i=0; while (buf[i]!=0) i++;
	while (buf[--i]=='0'); // contar ceros de atras para adelante
	if (buf[i]=='.') i--; // si llegamos al '.' sacarlo tambien
	if (buf[0]=='-'&&buf[1]=='0'&&i==1) buf[i=0]='0'; // si quedo "-0" sacar el -
	buf[++i]='\0'; // aplicar el corte
	return buf;
}

inline int StrToInt(const std::string &s) {
	return atoi(s.c_str());
}

inline std::string IntToStr(int l) {
	char buf[4*sizeof(int)]; // para un long de 64bits, el más largo tiene 20 cifras y estaría reservando 32
	sprintf(buf,"%i",l);
	return buf;
}

inline std::string BoolToStr(bool b) {
	return b?"VERDADERO":"FALSO";
}

inline bool StrToBool(const std::string &s) {
	return s=="VERDADERO";
}


struct tipo_var {
	friend class Memoria;
	//private: // el cliente pseint debe acceder a travez de memoria->LeerDims para que si es alias lo corrija (pero psexport si necesita acceder directo)
	int *dims; // dims[0] es la cantidad de dimensiones, dims[1...] son las dimensiones propiamente dichas
public:
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
		if (v.rounded) rounded=true;
		if (!error) {
			cb_log=cb_log&&v.cb_log;
			cb_num=cb_num&&v.cb_num;
			cb_car=cb_car&&v.cb_car;
			return true;
		} else
			return false;
	}
	bool is_known() const {
		return (cb_car?1:0)+(cb_log?1:0)+(cb_num?1:0)==1;
	}
	bool is_ok() const {
		return (cb_car?1:0)+(cb_log?1:0)+(cb_num?1:0)!=0;
	}
	bool operator==(const tipo_var &t) const {
		return cb_car==t.cb_car&&cb_num==t.cb_num&&cb_log==t.cb_log;
	}
	bool operator!=(const tipo_var &t) const {
		return cb_car!=t.cb_car||cb_num!=t.cb_num||cb_log!=t.cb_log;
	}
	bool can_be(const tipo_var &t) const {
		return (cb_car&&t.cb_car) || (cb_num&&t.cb_num) || (cb_log&&t.cb_log);
	}
	// cppcheck-suppress operatorEqVarError
	tipo_var &operator=(const tipo_var &t) {
		cb_log=t.cb_log;
		cb_num=t.cb_num;
		cb_car=t.cb_car;
		rounded=t.rounded;
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


struct DataValue {

	tipo_var type;
	Variant4<double,std::string,int,bool> value;
	
	DataValue() {}
	DataValue(tipo_var t) : type(t) {}
	DataValue(tipo_var t, int v) : type(t) { value.Set<int>(v); }
	DataValue(tipo_var t, double v) : type(t) { value.Set<double>(v); }
	DataValue(tipo_var t, bool v) : type(t) { value.Set<bool>(v); }
	DataValue(tipo_var t, const std::string &v) : type(t) { value.Set<std::string>(v); }
	
	bool IsOk() const { return type!=vt_error; } // tipo
	bool IsError() const { return type==vt_error; } // tipo
	bool IsEmpty() const { return value.IsDefined(); } // valor
	
	bool CanBeLogic() const  { return type.cb_log; }
	bool CanBeReal() const   { return type.cb_num; }
	bool CanBeString() const { return type.cb_car; }
	bool IsLogic() const  { return  type.cb_log && !type.cb_num && !type.cb_car; }
	bool IsReal() const   { return !type.cb_log &&  type.cb_num && !type.cb_car; }
	bool IsString() const { return !type.cb_log && !type.cb_num &&  type.cb_car; }
	
	bool GetAsBool() const {
//		Assert(!value.IsDefined()||value.Is<bool>());
		if (value.Is<bool>()) return value.As<bool>();
		else if (value.Is<std::string>()) {
			const std::string &str = value.As<std::string>();
			return !str.empty() && (str[0]=='1'||toupper(str[0]=='V'));
		}
		else if (value.Is<double>()) return value.As<double>()==1;
		else if (value.Is<int>()) return value.As<int>()==1;
		else return false;
	}
	
	double GetAsReal() const {
		Assert(!value.IsDefined()||!value.Is<bool>());
		if (value.Is<double>()) return value.As<double>();
		if (value.Is<int>()) return double(value.As<int>());
		if (value.Is<std::string>()) return StrToDbl(value.As<std::string>());
		return 0.0;
	}
	int GetAsInt() const {
		Assert(!value.IsDefined()||!value.Is<bool>());
		if (value.Is<double>()) return int(value.As<double>());
		if (value.Is<int>()) return value.As<int>();
		if (value.Is<std::string>()) return StrToInt(value.As<std::string>());
		return 0;
	}
	std::string GetAsString() const {
		Assert(!value.IsDefined()||value.Is<std::string>());
		if (value.Is<std::string>()) return value.As<std::string>();
		return "";
	}
	std::string GetForUser() const {
		if (type==vt_numerica) return DblToStr(GetAsReal(),true); // si era numerica, redondear para salida
		if (type==vt_logica) return GetAsBool()?VERDADERO:FALSO;
		return GetAsString();
	}
	
	void SetFromString(const std::string &s) { Assert(type.cb_car); value.ForceSet(s); }
	void SetFromInt(int i) { Assert(type.cb_num); value.ForceSet(i); }
	void SetValue(const DataValue &other) { value = other.value; } /// @todo: ver quien usa esto para ver si se copia tambien el tipo
	
	void Reset() { type.reset(); value.Clear(); }
	
	static DataValue MakeEmpty(tipo_var t) { return DataValue(t); }
	static DataValue MakeInt(int i) { return DataValue(vt_numerica_entera,i); }
	static DataValue MakeReal(double d) { return DataValue(vt_numerica,d); }
	static DataValue MakeLogic(bool b) { return DataValue(vt_logica,b); }
	static DataValue MakeString(const std::string &s) { return DataValue(vt_caracter,s); }
	
	static DataValue MakeReal(const std::string &s) { return DataValue(vt_numerica,StrToDbl(s)); }
	static DataValue MakeLogic(const std::string &s) { return DataValue(vt_logica,s==VERDADERO); }
	
	static DataValue DVError() { return DataValue(vt_error); }
};

#endif
