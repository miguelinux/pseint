#include "new_evaluar.h"
#include "global.h"
#include "new_funciones.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "intercambio.h"
#include <cstdlib>
#include "utils.h"
#include "Ejecutar.h"
using namespace std;

void WriteError(int num, string s) { 
	if (Inter.Running()) ExeError(num,s);
	else                 SynError(num,s);
}

bool PalabraReservada(const string &str) {
	// Comprobar que no sea palabra reservada
	if (lang[LS_WORD_OPERATORS] && (str=="Y" || str=="O" || str=="NO" || str=="MOD"))
		return true;
	if (str=="LEER" || str=="ESCRIBIR" || str=="MIENTRAS" || str=="HACER" || str=="SEGUN" || str=="VERDADERO" || str=="FALSO" || str=="PARA")
		return true;
	if (str=="REPETIR" || str=="SI" || str=="SINO" || str=="ENTONCES" ||   str=="DIMENSION" || str=="PROCESO")
		return true;
	if (str=="FINSI" ||  str=="FINPARA" || str=="FINSEGUN" || str=="FINPROCESO" || str=="FINMIENTRAS" ||  str=="HASTA" || str=="DEFINIR" || str=="COMO")
		return true;
	if (str=="FIN" ||  str=="IMPRIMIR" || str=="BORRAR" || str=="LIMPIAR" || str=="PANTALLA" ||  str=="BORRARPANTALLA" || str=="LIMPIARPANTALLA")
		return true;
	if (str=="SIN" || str=="BAJAR" || str=="SINBAJAR" || str=="SALTAR" || str=="SINSALTAR")
		return true;
	return false;
}

int BuscarComa(const string &expresion, int p1, int p2, char coma) {
	int parentesis = 0;
	while (p1<=p2) {
		char c = expresion[p1];
		if (c=='\'' || c=='\"') {
			do {
				c = expresion[++p1];
			} while(p1<p2 && c!='\'' && c!='\"');
		} else {
			if (c=='(' || c=='[')
				parentesis++;
			else if (parentesis && (c==')' || c==']'))
				parentesis--;
			else if (parentesis==0)
				if (c==coma) return p1;
		}
		p1++;
	}
	return -1;
}


struct InfoOperador {
	char oper;
	int level;
	bool right;
};

static const InfoOperador operadores[] = {
	{',',0,true},{'|',1,true},{'&',2,true},{'~',2,false},{'!',2,true},{'=',3,true},{'<',4,true},{'>',4,true},
	{'+',8,true},{'-',8,true},{'*',10,true},{'/',10,true},{'%',10,true},{'^',12,true},{' ',14,true} };

int BuscarOperador(const string &expresion, int &p1, int &p2) {
	bool parentesis_externos=true;
	static const int max_ind=sizeof(operadores)/sizeof(InfoOperador)-1;
	int j, indice_operador=max_ind, posicion_operador=-1;
	while (parentesis_externos) {
		char c=expresion[p1]; int i=p1;
		parentesis_externos = i<=p2&&c=='(';
		bool comillas=false;
		int parentesis=0;
		while(i<=p2) {
			if (c=='\"' || c=='\'') {
				comillas = !comillas;
			} else if (!comillas) {
				if (c=='(' || c=='[') {
					parentesis++;
				} else if (c==')' || c==']') {
					parentesis--;
				} else if (parentesis==0) {
						parentesis_externos=false;
					if ((c<'A'||c>'Z')&&(c<'0'||c>'9')) { // este if no es necesario pero baja considerablemente los tiempos
						j=0;
						while (j<max_ind && c!=operadores[j].oper) j++;
						if ( j!=max_ind && (
								operadores[j].level<operadores[indice_operador].level || 
								(operadores[j].right && operadores[j].level==operadores[indice_operador].level)
										   ) )
						{
							posicion_operador=i;
							indice_operador=j;
							char nc=expresion[i+1];
							if ( (c=='<'||c=='>'||c=='=') && (nc=='=' || nc=='>') ) i++;
							else if ( (c=='&'||c=='|') && (nc=='|' || nc=='&') ) i++;
						}
					}
				}
			}
			c=expresion[++i];
		}
		if (parentesis_externos) {
			p1++; p2--;
		}
	}
	return posicion_operador;
}

tipo_var DeterminarTipo(const string &expresion, int p1, int p2) {
	if (p2<p1) return vt_desconocido;
	int pos_op = BuscarOperador(expresion,p1,p2);
	if (pos_op==-1) {
//		if (p2<p1) return vt_desconocido;
		char c=expresion[p1];
		if ( c=='\''||c=='\"')
			return vt_caracter;
		else if ( (c>='0'&&c<='9') || c=='.' || c=='+' || c=='-')
			return vt_numerica;
		else if (expresion.substr(p1,p2-p1+1)==VERDADERO||expresion.substr(p1,p2-p1+1)==FALSO)
			return vt_logica;
		else {
			size_t pp=expresion.find('(',p1);
//			if (pp!=string::npos) {
				const Funcion *func=EsFuncion(expresion.substr(p1,pp-p1));
				if (func) return func->tipos[0];
//			}
			return memoria->LeerTipo(expresion.substr(p1,p2-p1+1));
		}
	} else {
		tipo_var t1,t2;
		char op = expresion[pos_op];
		int p1b=pos_op-1, p2a=pos_op+1;
		switch (op) {
		case '|':
		case '&':
		case '~':
		case '=':
		case '<':
		case '>':
			return vt_logica;
		case '+':
			if (lang[LS_ALLOW_CONCATENATION]) {
				int p1a=p1, p2b=p2;
				t1 = DeterminarTipo(expresion,p1a,p1b);
				t2 = DeterminarTipo(expresion,p2a,p2b);
				t1.set(vt_caracter_o_numerica);
				t2.set(vt_caracter_o_numerica);
				if (!t1.set(t2) || !t2.set(t1)) {
					return vt_error;
				}
				return t1;
			}
		case '-':
		case '*':
		case '/':
		case '^':
		case '%':
			return vt_numerica;
		}
	}
	return vt_desconocido;
}

bool AplicarTipo(const string &expresion, int &p1, int &p2, tipo_var tipo) {
	if (p2<p1) return true;
	int pos_op = BuscarOperador(expresion,p1,p2);
	if (pos_op==-1) { // si no hay operador, es una variable o constante
		string sub=expresion.substr(p1,p2-p1+1); char c=sub[0];
		if (sub==VERDADERO||sub==FALSO) return tipo.cb_log;
		if (c=='.'||(c>='0'&&c<='9') ) return tipo.cb_num;
		if (c=='\'') return tipo.cb_car;
		size_t p=sub.find('(');
		if (p!=string::npos) { // si era funcion no tiene que llegar a DefinirTipo porque sino aparece en el panel de variables
			sub.erase(p);
			const Funcion *f=EsFuncion(sub,true);
			if (f) {
				tipo_var tv=f->GetTipo(0);
				return tv.set(tipo);
			}
		}
		return memoria->DefinirTipo(sub,tipo);
	} else { // si era operador
		char op=expresion[pos_op];
		if (op=='='||op=='<'||op=='>'||op=='~'||op=='!') {
			return tipo.cb_log;
		}
		int p1b=pos_op-1, p2a=pos_op+1;
		int p1a=p1;
		tipo_var t1 = DeterminarTipo(expresion,p1a,p1b);
		if (t1!=tipo && !AplicarTipo(expresion,p1a,p1b,tipo)) return false;
		tipo_var t2 = DeterminarTipo(expresion,p2a,p2);
		int p2b=p2;
		if (t2!=tipo && !AplicarTipo(expresion,p2a,p2b,tipo)) return false;
		return true;
	}
	return false;  // nunca llega aca
}

#ifdef LOG_EVALUAR
	static int tabs=0;
	DataValue ev_aux(const DataValue &a,int &tabs)
	{ cerr<<setw(tabs)<<""<<"RET: *"<<(a.GetForUser())<<"*\n"; tabs-=2; return a;}
#	define ev_return(a) return ev_aux(a,tabs)
#else
#	define ev_return(a) return a
#endif

// estructura auxiliar para la funcion EvaluarFuncion, para que el destructor del objeto libere la memoria si la función aborta de forma temprana, entre otras cosas
// cppcheck-suppress noCopyConstructor
struct ActualArgs { 
	DataValue *values;
	PASAJE *pasajes;
	ActualArgs(int n) {
		values=new DataValue[n];
		pasajes=new PASAJE[n];
	}
	~ActualArgs() {
		delete []values;
		delete []pasajes;
	}
};

static bool EsArreglo(const string &nombre) {
	return nombre.find('(')==string::npos && memoria->Existe(nombre) && memoria->LeerDims(nombre);
}

DataValue EvaluarFuncion(const Funcion *func, const string &argumentos, const tipo_var &forced_tipo, bool for_expresion) {
	if (for_expresion && func->GetTipo(0)==vt_error) {
		WriteError(278,string("El subproceso (")+GetNombreFuncion(func)+(") no devuelve ningún valor."));
		return DataValue::DVError();
	}
	// controlar cantidad de argumentos
	int b=0,ca=argumentos[1]==')'?0:1, l=argumentos.length()-1;
	if (ca==1) while ((b=BuscarComa(argumentos,b+1,l))>0) ca++;
	if (func->cant_arg!=ca) {
		WriteError(279,string("Cantidad de argumentos incorrecta para el subproceso (")+GetNombreFuncion(func)+(")"));
		return DataValue(func->GetTipo(0));
	}
	// parsear argumentos
	ActualArgs args(ca);
	b=0;
	for (int i=0;i<ca;i++) {
		int b2=BuscarComa(argumentos,b+1,l,',');
		if (b2==-1) b2=l;
		int p1=b+1, p2=b2-1; b=b2;
		if (!AplicarTipo(argumentos,p1,p2,func->tipos[i+1])) {
			stringstream ss;
			ss<<"Tipo de dato incorrecto en el argumento "<<i+1<<" ("<<argumentos.substr(p1,p2-p1+1)<<")";
			WriteError(280,ss.str());
			return DataValue(forced_tipo); // deberiamos retorar error?
		} else {
			string arg_actual = argumentos.substr(p1,p2-p1+1);
			if (EsArreglo(arg_actual)) { // los arreglos siempre pasan por referencia
				if (func->pasajes[i+1]==PP_VALOR) { // si la funcion explicita por valor, error
					WriteError(281,string("Los arreglos solo pueden pasarse a subprocesos/funciones por referencia (")+arg_actual+(")"));
					return DataValue(forced_tipo);
				}
				args.pasajes[i] = PP_REFERENCIA;
				args.values[i] = DataValue::MakeString(arg_actual);
			} else if (func->pasajes[i+1]==PP_REFERENCIA) {
				args.pasajes[i] = PP_REFERENCIA;
				if (arg_actual.find('(')!=string::npos) CheckDims(arg_actual); // evalua las expresiones de los indices
				args.values[i] = DataValue::MakeString(arg_actual); /// @todo: definirlo aqui como alias cuando datavalue se lo banque
			} else {
				args.pasajes[i]=PP_VALOR;
				DataValue res = Evaluar(argumentos,p1,p2,func->tipos[i+1]);
				args.values[i] = res;
			}
		}
	}
	// obtener salida
	DataValue ret;
	if (func->func) { // funcion predefinida
		for(int i=0;i<func->cant_arg;i++)
			if (args.values[i].CanBeString() && EsArreglo(args.values[i].GetAsString())) {
				/// @todo: mejorar esto, podria querer funciones predefinidas que reciban arreglos, tipo sizeof
				WriteError(282,string("La función espera un único valor, pero recibe un arreglo (")+args.values[i].GetAsString()+(")"));
				return DataValue(forced_tipo);
			}
		ret = func->func(args.values);
#ifndef _FOR_PSEXPORT
	} else { // subprocesos del usuario
		if (Inter.Running()) {
			Memoria *caller_memoria=memoria;
			memoria=new Memoria(func);
			tipo_var tipo_arg;
			for(int i=0;i<func->cant_arg;i++) { 
				if (args.pasajes[i]==PP_VALOR) { // por valor
					memoria->EscribirValor(func->nombres[i+1],args.values[i]);
					memoria->DefinirTipo(func->nombres[i+1],args.values[i].type);
					if (lang[LS_FORCE_DEFINE_VARS] && Inter.Running()) memoria->DefinirTipo(func->nombres[i+1],args.values[i].type,args.values[i].type.rounded); // para que no genere error con force_var_definition, porque no se deja redefinir argumentos dentro del subproceso
				} else { // por referencia
					memoria->AgregarAlias(func->nombres[i+1],args.values[i].GetAsString(),caller_memoria);
				}
			}
//			Inter.OnFunctionIn(); // ahora se encarga Ejecutar
			Ejecutar(func->line_start);
//			Inter.OnFunctionOut(); // ahora se encarga Ejecutar
			if (func->nombres[0].size()) {
				ret = memoria->LeerValor(func->nombres[0]);
				ret.type = memoria->LeerTipo(func->nombres[0]);
			}
			delete memoria;
			memoria=caller_memoria;
		} 
#endif
	}
	if (!ret.type.set(forced_tipo)) 
		WriteError(283,"No coinciden los tipos.");
	return ret;
}

DataValue Evaluar(const string &expresion, int &p1, int &p2, const tipo_var &forced_tipo) {
#ifdef LOG_EVALUAR
	tabs+=2;
	cerr<<setw(tabs)<<""<<"EVALUAR: {"<<expresion.substr(p1,p2-p1+1)<<"}\n";
#endif
	int pos_op = BuscarOperador(expresion,p1,p2);
	if (pos_op!=-1 && expresion[pos_op]==',') 
		{ WriteError(284,string("Se esperaba solo una expresión")); return DataValue::DVError(); }
	if (pos_op==-1/* || pos_op==p1*/) { // si no hay operador, es constante o variable
		if (p2<p1) ev_return(DataValue(forced_tipo)); //operandos vacios... como el primero en (-3)
		char c = expresion[p1];
		if ( c=='\''||c=='\"') { // si empieza con comillas, es cadena de caracteres
			ev_return( DataValue::MakeString(expresion.substr(p1+1,p2-p1-1)) );
		} else if ( (c>='0'&&c<='9') || c=='.' || c=='-' || c=='+') { // si empieza con un numero o un punto, es nro
			ev_return( DataValue::MakeReal(expresion.substr(p1,p2-p1+1)) );
		} else if (expresion.substr(p1,p2-p1+1)==VERDADERO || expresion.substr(p1,p2-p1+1)==FALSO) { // veradero o falso, logica
			ev_return( DataValue::MakeLogic(expresion.substr(p1,p2-p1+1)) );
		} else { // sino es variable... ver si es comun o arreglo
			size_t pm = expresion.find('(',p1);
			if (int(pm)>p2) pm=string::npos;
			if (pm==string::npos) { // si es una variable comun
				string nombre = expresion.substr(p1,p2-p1+1);
				if (!Inter.Running() && (PalabraReservada(nombre) || nombre==main_process_name)) {
					WriteError(285,string("Identificador no válido (")+nombre+")");
					ev_return(DataValue::DVError());
				}
				const Funcion *func = EsFuncion(nombre);
				if (func) {
					if (func->cant_arg!=0) {
						WriteError(286,string("Faltan parámetros para la función (")+nombre+")");
						ev_return(DataValue::DVError());
					} else {
						DataValue res = EvaluarFuncion(func,"()",forced_tipo);
						ev_return(res);
					}
				}
				if (memoria->LeerDims(nombre)) { // usar leertipo trae problemas cuando la variable es un alias a un elemento de un arreglo
					WriteError(220,string("Faltan subindices para el arreglo (")+nombre+")");
					ev_return(DataValue::DVError());
				}
				DataValue res;
				if (lang[LS_FORCE_DEFINE_VARS] && Inter.Running() && !memoria->EstaDefinida(nombre)) {
					WriteError(210,string("Variable no definida (")+nombre+")");
					ev_return(DataValue::DVError());
				}
				if ((lang[LS_FORCE_INIT_VARS] || Inter.EvaluatingForDebug()) && Inter.Running() && !memoria->EstaInicializada(nombre)) {
					WriteError(215,string("Variable no inicializada (")+nombre+")");
					ev_return(DataValue::DVError());
				}
				res = memoria->Leer(nombre);
				ev_return(res);
			} else { // si es un arreglo o funcion
				string nombre=expresion.substr(p1,pm-p1);
				const Funcion *func=EsFuncion(nombre);
				if (func) { //si es funcion
					DataValue res = EvaluarFuncion(func,expresion.substr(pm,p2-pm+1),forced_tipo);
					ev_return(res);
				} else {
					if (PalabraReservada(nombre)) {
						WriteError(287,string("Identificador no válido (")+nombre+")");
						ev_return(DataValue::DVError());
					}
					if (lang[LS_FORCE_DEFINE_VARS] && Inter.Running() && !memoria->EstaDefinida(nombre)) {
						WriteError(209,string("Variable no definida (")+nombre+")");
						ev_return(DataValue::DVError());
					}
					string aux=expresion.substr(p1,p2-p1+1);
					if (CheckDims(aux)) {
						if (lang[LS_FORCE_INIT_VARS] && Inter.Running() && !memoria->EstaInicializada(aux)) {
							WriteError(288,string("Posición no inicializada (")+aux+")");
							ev_return(DataValue::DVError());
						}
						DataValue res = memoria->Leer(aux);
						ev_return(res);
					} else {
						ev_return(DataValue::DVError());
					}
				}
			}
		}
	} else { // si habia operador, ver cual...
		// cortar operandos y ver sus tipos
		char op = expresion[pos_op], next=expresion[pos_op+1];
		int p1a = p1, p2a = pos_op+1, p1b = pos_op-1, p2b = p2;
		if (next=='='||next=='&'||next=='|'||next=='>') p2a++;
		tipo_var td1 = DeterminarTipo(expresion,p1a,p1b);
		tipo_var td2 = DeterminarTipo(expresion,p2a,p2b);
		if ((op!='~'&&!td1.is_ok()) || !td2.is_ok()) { 
			// DeterminarTipo no informa el error (ni al usuario ni impide la ejecucion)
			// los evaluar que siguen están para que ese error se manifieste
			if (op!='~') Evaluar(expresion,p1a,p1b);
			Evaluar(expresion,p2a,p2b);
			ev_return(DataValue::DVError());
		}
		// analizar segun operador
		switch (op) {
			
		case '|': case '&': {
			// los operandos deben ser logicos
			if ( (td1!=vt_logica && !AplicarTipo(expresion,p1a,p1b,vt_logica)) ||
			     (td2!=vt_logica && !AplicarTipo(expresion,p2a,p2b,vt_logica)) ) 
			{
				WriteError(289,"No coinciden los tipos (|/O, &/Y). Los operandos deben ser logicos.");
				ev_return(DataValue::DVError());
			}
			DataValue s1 = Evaluar(expresion,p1a,p1b,vt_logica);
			if (Inter.Running()) {
				if (op=='|' && s1.GetAsBool()) ev_return(DataValue::MakeLogic(true));
				if (op=='&' && !s1.GetAsBool()) ev_return(DataValue::MakeLogic(false));
			}
			DataValue s2 = Evaluar(expresion,p2a,p2b,vt_logica);
			if (!s1.CanBeLogic() || !s2.CanBeLogic()) { ev_return(DataValue::DVError()); }
			if (op=='|') ev_return(DataValue::MakeLogic(s1.GetAsBool() || s2.GetAsBool()));
			else         ev_return(DataValue::MakeLogic(s1.GetAsBool() && s2.GetAsBool()));
		}
		case '~': case '!':
			if (op!='!'||next!='=') {
				// el operando debe ser logico
				if (p1a!=pos_op) // tal vez nunca se llegue a este error, porque lo detecta en otro lado
				{
					string aux_exp = expresion.substr(0,pos_op)+"("+expresion.substr(pos_op,p2-pos_op+1)+")";
					int aux_p2 = p2+2;
					DataValue ret = Evaluar(aux_exp,p1,aux_p2,forced_tipo);
					p2 = aux_p2-2; 
					return ret;
				}
				if (p2a>p2) // tal vez nunca se llegue a este error, porque lo detecta en otro lado
					WriteError(290,"Falta operando para la negación (~/NO).");
				if (td2!=vt_logica && !AplicarTipo(expresion,p2a,p2,vt_logica)) { 
					WriteError(291,"No coinciden los tipos (~ o NO). El operando deben ser lógico.");
					ev_return(DataValue::DVError());
				}
				DataValue s2 = Evaluar(expresion,p2a,p2b,vt_logica);
				ev_return(DataValue::MakeLogic(!s2.GetAsBool()));
			} else {
				next='>'; op='<';
			}
		case '<': case '>': {
			// los operandos no pueden ser logicos para <, >, <= o >=
			if ((op=='<' || op=='>')&& next!='>') {
				if (td1.cb_log) {
					td1.set(vt_caracter_o_numerica);
					AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica);
				}
				if (td2.cb_log) {
					td2.set(vt_caracter_o_numerica);
					AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica);
				}
				if (!td1.is_ok() || !td2.is_ok()) { 
					WriteError(292,"No coinciden los tipos (<, >, <= o >=). Los operandos no deben ser logicos.");
					ev_return(DataValue::DVError());
				}
			}
		case '=': {
			// los operandos deben tener igual tipo
			tipo_var t = vt_desconocido;
			if (!td1.is_known()) { // no se conoce t1...
				if (td2.is_known()) { // ...pero si t2
					t=td2;
					if (!AplicarTipo(expresion,p1a,p1b,td2)) { ev_return( DataValue::DVError() ); }
				}
			} else { // se conoce t1
				if (!td2.is_known()) { // t2 no
					t=td1; /*int p2b=p2;*/ // ¿por que se redefinia p2b???
					if (!AplicarTipo(expresion,p2a,p2b,td1)) { ev_return(DataValue::DVError()); }
				} else if (td1!=td2) {// t2 no coincide
					WriteError(207,"No coinciden los tipos (<, >, <=, >=, <> o =). No se puede comparar variables o expresiones de distinto tipo.");
					ev_return(DataValue::DVError()); 
				}
			}
			// evaluar operandos y aplicar operador
			DataValue s1 = Evaluar(expresion,p1a,p1b,t);
			DataValue s2 = Evaluar(expresion,p2a,p2b,t);
			if (!s1.type.is_ok() || !s2.type.is_ok()) {
				ev_return(DataValue::DVError());
			} else {
				t.set(s1.type); t.set(s2.type); // esto deberia ser redundante si DeterminarTipo funciona siempre
				if (t.cb_num) { // comparaciones de numeros
#define _epsilon (1e-11)
					if (op=='<') {
						if (next=='>') {
							double diff=s1.GetAsReal()-s2.GetAsReal(); if (diff<0) diff=-diff;
							ev_return(DataValue::MakeLogic(diff>=_epsilon)); // comparacion "difusa" para evitar problemas numericos
						}
						else if (next=='=')
							ev_return(DataValue::MakeLogic((s1.GetAsReal()-_epsilon)<=s2.GetAsReal()));
						else
							ev_return(DataValue::MakeLogic((s1.GetAsReal()+_epsilon)<s2.GetAsReal()));
					} else if (op=='>') {
						if (next=='=')
							ev_return(DataValue::MakeLogic((s1.GetAsReal()+_epsilon)>=s2.GetAsReal()));
						else
							ev_return(DataValue::MakeLogic((s1.GetAsReal()-_epsilon)>s2.GetAsReal()));
					} else {
						double diff=s1.GetAsReal()-s2.GetAsReal(); if (diff<0) diff=-diff;
						ev_return(DataValue::MakeLogic(diff<_epsilon)); // comparacion "difusa" para evitar problemas numericos
					}
				} else if (t==vt_logica) { // comparaciones de logicos
					if (op=='<' && next=='>') {
						ev_return(DataValue::MakeLogic((s1.GetAsBool())!=(s2.GetAsBool())));
					} else if (op=='=') {
						ev_return(DataValue::MakeLogic((s1.GetAsBool())==(s2.GetAsBool())));
					} else {
						/// @todo: ver si todavía es útil que devuelva tipo error pero valor valido
						DataValue res = DataValue::MakeLogic(false);
						res.type = vt_error;
						ev_return(res);
					}
				} else { // comparaciones de cadenas
					if (op=='<') {
						if (next=='>')
							ev_return(DataValue::MakeLogic(s1.GetAsString()!=s2.GetAsString()));
						else if (next=='=')
							ev_return(DataValue::MakeLogic(s1.GetAsString()<=s2.GetAsString()));
						else
							ev_return(DataValue::MakeLogic(s1.GetAsString()<s2.GetAsString()));
					} else if (op=='>') {
						if (next=='=')
							ev_return(DataValue::MakeLogic(s1.GetAsString()>=s2.GetAsString()));
						else
							ev_return(DataValue::MakeLogic(s1.GetAsString()>s2.GetAsString()));
					} else
						ev_return(DataValue::MakeLogic(s1.GetAsString()==s2.GetAsString()));
				}
			}
		} // case '='
		} // case '<': case '>'
		case '+':
			if (lang[LS_ALLOW_CONCATENATION]) {
				if (td1.can_be(vt_logica) && !AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica)) {
					WriteError(293,"No coinciden los tipos (+). Los operandos deben ser numericos o caracter.");
					ev_return(DataValue::DVError());
				} else td1.set(vt_caracter_o_numerica);
				if (td2.can_be(vt_logica) && !AplicarTipo(expresion,p2a,p2b,vt_caracter_o_numerica)) {
					WriteError(294,"No coinciden los tipos (+). Los operandos deben ser numericos o caracter.");
					ev_return(DataValue::DVError());
				} else td2.set(vt_caracter_o_numerica);
				if (td1!=td2) {
					tipo_var ot1=td1, ot2=td2;
					td1.set(td2);
					td2.set(td1);
					if (!td1.is_ok() || !td2.is_ok() || (ot1!=td1 && !AplicarTipo(expresion,p1a,p1b,td1)) || (ot2!=td2 && !AplicarTipo(expresion,p2a,p2b,td2)) || td1!=td2) {
						WriteError(295,"No coinciden los tipos (+). Los operandos deben ser de igual tipo.");
						ev_return(DataValue::DVError());
					}
				}
				tipo_var tipo;
				if (td1==vt_numerica||td2==vt_numerica) tipo=vt_numerica;
				else if (td1==vt_caracter||td2==vt_caracter) tipo=vt_caracter;
				else tipo=vt_caracter_o_numerica;
				DataValue s1 = Evaluar(expresion,p1a,p1b,tipo);
				DataValue s2 = Evaluar(expresion,p2a,p2b,tipo);
				// esta comprobacion parece redundante segun los tests... si no le paso el tipo a los evaluar
				// anteriores, se torna util... hay algun caso en que todavía sirva???
				if (!s1.type.set(s2.type) || !s2.type.set(s1.type)) {
					WriteError(295,"No coinciden los tipos (+). Los operandos deben ser de igual tipo.");
					ev_return(DataValue::DVError());
				}
				if (tipo==vt_caracter) { ev_return( DataValue::MakeString(s1.GetAsString()+s2.GetAsString()) ); }
				else { ev_return( DataValue::MakeReal(s1.GetAsReal()+s2.GetAsReal()) ); }
			}
			
		case '-':case '*':case '/':case '^':case '%': {
			// los operandos deben ser numericos
			if ((td1!=vt_numerica && !AplicarTipo(expresion,p1a,p1b,vt_numerica)) ||
				(td2!=vt_numerica && !AplicarTipo(expresion,p2a,p2b,vt_numerica))) {
					WriteError(213,"No coinciden los tipos (+, -, *, /, ^, % o MOD). Los operandos deben ser numericos.");
					ev_return(DataValue::DVError());
				}
			// evaluar operandos
			DataValue s1 = Evaluar(expresion,p1a,p1b,vt_numerica);
			DataValue s2 = Evaluar(expresion,p2a,p2b,vt_numerica);
			if (!s1.CanBeReal() || !s2.CanBeReal()) { ev_return(DataValue::DVError()); }
			// operar
			double res=0, o1=s1.GetAsReal(), o2=s2.GetAsReal();
			if (op=='+') {
				res=o1+o2;
			} else if (op=='-') { 
				res=o1-o2;
			} else if (op=='/') {
				if (o2==0) {
					/// @todo: ver si tiene sentido.... lo mismo que arriba
					DataValue retval = DataValue::MakeReal(0);
					if (Inter.Running()) {
						WriteError(296,"Division por cero");
						retval.type=vt_error;
					}
					ev_return(retval);
				} else
					res=o1/o2;
			} else if (op=='*') {
				res=o1*o2;
			} else if (op=='%') {
				if (floor(o2)==0) {
					/// @todo: ver.... lo mismo que antes
					DataValue retval = DataValue::MakeReal(0);
					if (Inter.Running()) {
						WriteError(316,"El segundo operando para el operador MOD no puede ser cero");
						retval.type = vt_error;
					}
					ev_return(retval);
				} else {
					if (o1!=floor(o1)||o2!=floor(o2)) {
						WriteError(298,"Los operandos para el operador MOD deben ser enteros");
					} else 
						res=(long long)(o1)%(long long)(o2);
				}
			} else if (op=='^') {
				res=pow(o1,o2);
			}
			ev_return(DataValue::MakeReal(res));
		}
		}
	}
	ev_return(DataValue::DVError());
}

// wrapper para llamar al Evaluar que sigue desde SynCheck, para que verifique que no falten operandos al principio o al final, y aplique los tipos solo si la evaluación es correcta
DataValue EvaluarSC(string expresion, const tipo_var &forced_tipo) {
	if (ignore_logic_errors) { return DataValue(forced_tipo); }
	// primero evalua sin importar de que tipo deberia ser (para que el error de tipo lo dé afuera, mejor contextualizado)
	int p1=0, p2=expresion.size()-1;
	DataValue retval = Evaluar(expresion,p1,p2,forced_tipo);
	// si no habia nada raro, aplica el tipo a las variables
	if (forced_tipo!=vt_desconocido && retval.type!=vt_error) AplicarTipo(expresion,p1,p2,forced_tipo);
	return retval;
}

// recibe la expresion a evaluar y el tipo con la info de que puede llegar a ser
// devuelve el resultado si se pudo evaluar y el tipo en tipo, sino "" y vt_error
DataValue Evaluar(string expresion, const tipo_var &forced_tipo) {
	DataValue d;
	d.type = forced_tipo;
	int p1=0, p2=expresion.size()-1;
	if (forced_tipo!=vt_desconocido && !AplicarTipo(expresion,p1,p2,forced_tipo)) return DataValue::DVError(); // TODO: APLICAR EL TIPO QUE VIENE
//	tipo_var rt = DeterminarTipo(expresion,p1,p2);
	return Evaluar(expresion,p1,p2,forced_tipo);
}

bool CheckDims(string &str) {
	int pp=str.find("(",0), p2=str.size()-1;
	// ver que efectivamente sea un arreglo
	string nombre=str.substr(0,pp);
	const int *adims=memoria->LeerDims(str);
	if (!adims) {
		if (!Inter.Running() && memoria->EsArgumento(nombre)) return true; // si es una funcion, no sabemos si lo que van a pasar sera o no arreglo
#ifdef _FOR_PSEXPORT
		// cuando una expresion con un arreglo se utiliza desde un subproceso, la dimension del mismo
		// no esta en el subproceso y entonces no se sabe, pero es neceria para exportar, por ejemplo
		// para las cabeceras de las funciones en C++, así que al menos con esto detectamos que se trata
		// de un arreglo, y sabemos la cantidad de dimensiones, aunque no sepamos sus tamaños
		int b=pp+1,ca=1; while ((b=BuscarComa(str,b+1,p2))>0) ca++;
		int *dims=new int[ca+1]; for(int i=0;i<ca;i++) dims[i+1]=0; dims[0]=ca;
		memoria->AgregarArreglo(nombre,dims);
#endif
		WriteError(202,string("El identificador ")+str.substr(0,pp)+(" no corresponde a un arreglo o subproceso")); /// @todo: ver que hacer cuando se llama desde psexport, porque genera errores falsos
		return false;
	}
	if (!Inter.Running()) {
		
		// esto es para cuando esta checkeando sintaxis antes de ejecutar
		// en este caso no debe verificar si las expresiones dan en los rangos 
		// del arreglo
		int b=pp+1,ca=1; while ((b=BuscarComa(str,b+1,p2))>0) ca++;
		str[str.size()-1]=','; /*pp;*/
		for (int i=0;i<ca;i++) {
			int np = BuscarComa(str,++pp,p2)-1; // deberia tratar de forzar tipo entero?
			tipo_var t = Evaluar(str,pp,np).type;
			pp=np+1;
			if (!t.is_ok()) return false;
		}
		// controlar cantidad de dimensiones
		if (adims[0]!=ca) {
			WriteError(299,string("Cantidad de indices incorrecta para el arreglo (")+nombre+(")"));
			return false;
		}
		
		// marcar los indices de arreglos como numeros (y enteros para que no sean float al exportar a c++)
#ifdef _FOR_PSEXPORT
		string exp=str.substr(str.find("(")+1)/*+","*/; int par=0;
		for (unsigned int i=0,l=0;i<exp.size();i++) {
			if (exp[i]=='\"') {
				i++;
				while (i<exp.size() && exp[i]!='\"')
					i++;
				i++;
				l=i+1;
			} else if (!EsLetra(exp[i])&&(exp[i]<'0'||exp[i]>'9')&&exp[i]!='_') {
				if (par==0) {
					string nombre=exp.substr(l,i-l);
					if (memoria->Existe(nombre)) 
						memoria->DefinirTipo(nombre,vt_numerica_entera);
				}
				l=i+1;
				if (exp[i]=='['||exp[i]=='(') par++;
				else if (exp[i]==']'||exp[i]==')') par--;
			}
		}
#endif
		return true;
	} 
	
	int b=pp,ca=1; while ((b=BuscarComa(str,b+1,p2))>0) ca++;
	if (adims[0]!=ca) {
		WriteError(300,string("Cantidad de indices incorrecta para el arreglo (")+nombre+(")"));
		return false;
	}
	nombre+="(";
	str[str.size()-1]=',';
	for (int i=0;i<ca;i++) {
		int np=BuscarComa(str,++pp,p2)-1;
		DataValue ret = Evaluar(str,pp,np,vt_numerica);
		if (!ret.IsOk()) return false;
		int idx = ret.GetAsInt();
		if (int(ret.GetAsReal())!=idx) {
			WriteError(301,string("Subindice no entero ("+ret.GetForUser()+")"));
			return false;
		}
		if (lang[LS_BASE_ZERO_ARRAYS]) {
			if (idx<0||idx>adims[i+1]-1) {
				WriteError(302,string("Subindice (")+ret.GetForUser()+") fuera de rango (0..."+IntToStr(adims[i+1]-1)+")");
				return false;
			}
		} else {
			if (idx<1||idx>adims[i+1]) {
				WriteError(303,string("Subindice (")+ret.GetForUser()+") fuera de rango (1..."+IntToStr(adims[i+1])+")");
				return false;
			}
		}
		nombre+=ret.GetForUser()+","; // en nombre vamos armando la referencia a la pos con los indices ya evaluados, se devuelve por str
		pp=np+1;
	}
	str=nombre;
	str[str.size()-1]=')';
	
	return true;
}
