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
	if (Inter.Running())
		ExeError(num,s);
	else
		SynError(num,s);
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
	bool comillas = false;
	int parentesis = 0;
	while (p1<=p2) {
		char c = expresion[p1];
		if (c=='\'' || c=='\"')
			comillas = !comillas;
		else if (!comillas) {
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

static int operadores[]={',','|','&','~','!','=','<','>','+','-','*','/','^','%',' '};
static int oper_lev[]=  { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 8 , 10, 10, 12, 13, 14};

int BuscarOperador(const string &expresion, int &p1, int &p2) {
	bool parentesis_externos=true;
	static int max_ind=sizeof(operadores)/sizeof(int)-1;
	int j, indice_operador=max_ind, posicion_operador=-1;
	while (parentesis_externos) {
		char c=expresion[p1]; int i=p1;
		// comentado el 19-2-2013, creo que ya no es necesario
//		while (i<=p2 && (c==' '||c=='\t'))
//			c=expresion[++i];
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
						while (j<max_ind && c!=operadores[j]) j++;
						if (j!=max_ind && oper_lev[j]<=oper_lev[indice_operador]) {
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
			if (pp!=string::npos) {
				const Funcion *func=EsFuncion(expresion.substr(p1,pp-p1));
				if (func) return func->tipos[0];
			}
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
		if (t2==vt_desconocido && !AplicarTipo(expresion,p2a,p2b,tipo)) return false;
		return true;
	}
	return false;  // nunca llega aca
}

#ifdef LOG_EVALUAR
static int tabs=0;
string ev_aux(string a,int &tabs)
{ cerr<<setw(tabs)<<""<<"RET: *"<<(a)<<"*\n"; tabs-=2; return a;}
#define ev_return(a) return ev_aux(a,tabs)
#else
#define ev_return(a) return a
#endif

// estructura auxiliar para la funcion EvaluarFuncion, para que el destructor del objeto libere la memoria si la función aborta de forma temprana, entre otras cosas
// cppcheck-suppress noCopyConstructor
struct info_de_llamada { 
	string *values;
	tipo_var *tipos;
	PASAJE *pasajes;
	info_de_llamada(int n) {
		values=new string[n];
		tipos=new tipo_var[n];
		pasajes=new PASAJE[n];
	}
	~info_de_llamada() {
		delete []values;
		delete []tipos;
		delete []pasajes;
	}
};

static bool EsArreglo(const string &nombre) {
	return nombre.find('(')==string::npos && memoria->Existe(nombre) && memoria->LeerDims(nombre);
}

string EvaluarFuncion(const Funcion *func, const string &argumentos, tipo_var &tipo, bool for_expresion) {
	if (for_expresion && func->GetTipo(0)==vt_error) {
		WriteError(278,string("El subproceso (")+GetNombreFuncion(func)+(") no devuelve ningún valor."));
		tipo=vt_error; return "";
	}
	// controlar cantidad de argumentos
	int b=0,ca=argumentos[1]==')'?0:1, l=argumentos.length()-1;
	if (ca==1) while ((b=BuscarComa(argumentos,b+1,l))>0) ca++;
	if (func->cant_arg!=ca) {
		tipo=vt_error;
		WriteError(279,string("Cantidad de argumentos incorrecta para el subproceso (")+GetNombreFuncion(func)+(")"));
		return("");
	}
	// parsear argumentos
	info_de_llamada args(ca);
	b=0;
	for (int i=0;i<ca;i++) {
		int b2=BuscarComa(argumentos,b+1,l,',');
		if (b2==-1) b2=l;
		int p1=b+1, p2=b2-1; b=b2;
		if (!AplicarTipo(argumentos,p1,p2,func->tipos[i+1])) {
			stringstream ss;
			ss<<"Tipo de dato incorrecto en el argumento "<<i+1<<" ("<<argumentos.substr(p1,p2-p1+1)<<")";
			WriteError(280,ss.str());
			return("");
		} else {
			string arg_actual = argumentos.substr(p1,p2-p1+1);
			if (EsArreglo(arg_actual)) { // los arreglos siempre pasan por referencia
				if (func->pasajes[i+1]==PP_VALOR) { // si la funcion explicita por valor, error
					WriteError(281,string("Los arreglos solo pueden pasarse a subprocesos/funciones por referencia (")+arg_actual+(")"));
					return "";
				}
				args.pasajes[i]=PP_REFERENCIA;
				args.values[i]=arg_actual;
			} else if (func->pasajes[i+1]==PP_REFERENCIA) {
				args.pasajes[i]=PP_REFERENCIA;
				if (arg_actual.find('(')!=string::npos) CheckDims(arg_actual); // evalua las expresiones de los indices
				args.values[i]=arg_actual;
			} else {
				args.pasajes[i]=PP_VALOR;
				args.values[i]=Evaluar(argumentos,p1,p2,args.tipos[i]);
			}
		}
	}
	if (tipo==vt_error) {
		ev_return("");
	}
	// obtener salida
	string ret; tipo_var rettipo;
	if (func->func) {
		for(int i=0;i<func->cant_arg;i++)
			if (EsArreglo(args.values[i])) {
				WriteError(282,string("La función espera un único valor, pero recibe un arreglo (")+args.values[i]+(")"));
				return "";
			}
		ret=func->func(args.values);
		rettipo=func->tipos[0];
#ifndef _FOR_PSEXPORT
	} else {
		if (Inter.Running()) {
			Memoria *caller_memoria=memoria;
			memoria=new Memoria(func);
			tipo_var tipo_arg;
			for(int i=0;i<func->cant_arg;i++) { 
				if (args.pasajes[i]==PP_VALOR) { // por valor
					memoria->EscribirValor(func->nombres[i+1],args.values[i]);
					memoria->DefinirTipo(func->nombres[i+1],args.tipos[i]);
					if (lang[LS_FORCE_DEFINE_VARS] && Inter.Running()) memoria->DefinirTipo(func->nombres[i+1],args.tipos[i],args.tipos[i].rounded); // para que no genere error con force_var_definition, porque no se deja redefinir argumentos dentro del subproceso
				} else { // por referencia
					memoria->AgregarAlias(func->nombres[i+1],args.values[i],caller_memoria);
				}
			}
//			Inter.OnFunctionIn(); // ahora se encarga Ejecutarº
			Ejecutar(func->line_start);
//			Inter.OnFunctionOut(); // ahora se encarga Ejecutar
			if (func->nombres[0].size()) {
				ret=memoria->LeerValor(func->nombres[0]);
				rettipo=memoria->LeerTipo(func->nombres[0]);
			}
			delete memoria;
			memoria=caller_memoria;
		} 
#endif
	}
	if (!tipo.can_be(rettipo)) 
		WriteError(283,"No coinciden los tipos.");
	tipo=rettipo;
	return ret;
}

string Evaluar(const string &expresion, int &p1, int &p2, tipo_var &tipo) {
	// es probable que estos 2 whiles ya no sean necesarios
	while (p1<p2&&expresion[p1]==' ') p1++;
	while (p1<p2&&expresion[p2]==' ') p2--;
#ifdef LOG_EVALUAR
	tabs+=2;
	cerr<<setw(tabs)<<""<<"EVALUAR: {"<<expresion.substr(p1,p2-p1+1)<<"}\n";
#endif
	int pos_op = BuscarOperador(expresion,p1,p2);
	if (pos_op!=-1 && expresion[pos_op]==',') 
		{ WriteError(284,string("Se esperaba solo una expresión")); tipo=vt_error; return ""; }
	if (pos_op==-1/* || pos_op==p1*/) { // si no hay operador, es constante o variable
		if (p2<p1) ev_return("");
		char c = expresion[p1];
		if ( c=='\''||c=='\"') { // si empieza con comillas, es cadena de caracteres
			tipo = vt_caracter;
			ev_return(expresion.substr(p1+1,p2-p1-1));
		} else if ( (c>='0'&&c<='9') || c=='.' || c=='-' || c=='+') { // si empieza con un numero o un punto, es nro
			tipo = vt_numerica;
			ev_return(expresion.substr(p1,p2-p1+1));
		} else if (expresion.substr(p1,p2-p1+1)==VERDADERO || expresion.substr(p1,p2-p1+1)==FALSO) { // veradero o falso, logica
			tipo = vt_logica;
			ev_return(expresion.substr(p1,p2-p1+1));
		} else { // sino es variable... ver si es comun o arreglo
			size_t pm=expresion.find('(',p1);
			if (int(pm)>p2) pm=string::npos;
//			size_t pp=expresion.find('(',p1);
//			if (int(pp)>p2) pp=string::npos;
//			size_t pc=expresion.find('[',p1);
//			if (int(pc)>p2) pc=string::npos;
//			size_t pm=string::npos;
//			if (pp!=string::npos) {
//				if (pc!=string::npos && pc<pp)
//					pm=pc;
//				else
//					pm=pp;
//			} if (pc!=string::npos)
//				pm=pc;
			if (pm==string::npos) { // si es una variable comun
				string nombre = expresion.substr(p1,p2-p1+1);
				if (PalabraReservada(nombre) || nombre==main_process_name) {
					WriteError(285,string("Identificador no valido (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				const Funcion *func=EsFuncion(nombre);
				if (func) {
					if (func->cant_arg!=0) {
						WriteError(286,string("Faltan parametros para la funcion (")+nombre+")");
						tipo=vt_error;
						ev_return("");
					} else {
						ev_return(EvaluarFuncion(func,"()",tipo));
					}
				}
				if (memoria->LeerDims(nombre)) { // usar leertipo trae problemas cuando la variable es un alias a un elemento de un arreglo
					WriteError(220,string("Faltan subindices para el arreglo (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				tipo = memoria->LeerTipo(nombre);
				if (lang[LS_FORCE_DEFINE_VARS] && Inter.Running() && !memoria->EstaDefinida(nombre)) {
					WriteError(210,string("Variable no definida (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				if ((lang[LS_FORCE_INIT_VARS] || Inter.EvaluatingForDebug()) && Inter.Running() && !memoria->EstaInicializada(nombre)) {
					WriteError(215,string("Variable no inicializada (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				ev_return(memoria->LeerValor(nombre));
			} else { // si es un arreglo o funcion
				string nombre=expresion.substr(p1,pm-p1);
				const Funcion *func=EsFuncion(nombre);
				if (func) { //si es funcion
					ev_return(EvaluarFuncion(func,expresion.substr(pm,p2-pm+1),tipo));
				} else {
					if (PalabraReservada(nombre)) {
						WriteError(287,string("Identificador no valido (")+nombre+")");
						tipo=vt_error;
						ev_return("");
					}
					if (lang[LS_FORCE_DEFINE_VARS] && Inter.Running() && !memoria->EstaDefinida(nombre)) {
						WriteError(209,string("Variable no definida (")+nombre+")");
						tipo=vt_error;
						ev_return("");
					}
					string aux=expresion.substr(p1,p2-p1+1);
					if (CheckDims(aux)) {
						if (lang[LS_FORCE_INIT_VARS] && Inter.Running() && !memoria->EstaInicializada(aux)) {
							WriteError(288,string("Posición no inicializada (")+aux+")");
							tipo=vt_error;
							ev_return("");
						}
						tipo=memoria->LeerTipo(aux); // correccion 20130213
						ev_return(memoria->LeerValor(aux));
					} else {
						tipo=vt_error;
						ev_return("");
					}
				}
			}
		}
	} else { // si habia operador, ver cual...
		// cortar operandos y ver sus tipos
		string s1,s2;
		tipo_var t1,t2;
		char op = expresion[pos_op], next=expresion[pos_op+1];
		int p1b=pos_op-1, p2a=pos_op+1;
		if (next=='='||next=='&'||next=='|'||next=='>') p2a++;
		int p1a=p1, p2b=p2;
		t1=DeterminarTipo(expresion,p1a,p1b);
		t2=DeterminarTipo(expresion,p2a,p2b);
		if ((op!='~'&&!t1.is_ok()) || !t2.is_ok()) { 
			// DeterminarTipo no informa el error (ni al usuario ni impide la ejecucion)
			// los evaluar que siguen están para que ese error se manifieste
			tipo_var t1; if (op!='~') Evaluar(expresion,p1a,p1b,t1);
			tipo_var t2; Evaluar(expresion,p2a,p2b,t2);
			tipo=vt_error; ev_return("");
		}
		// analizar segun operador
		switch (op) {
			
		case '|': case '&': 
			tipo=vt_logica; // el resultado es logico
			// los operandos deben ser logicos
			if (!t1.is_known()) AplicarTipo(expresion,p1a,p1b,vt_logica);
			if (!t2.is_known()) AplicarTipo(expresion,p2a,p2b,vt_logica);
			if (!t1.is_ok() || !t2.is_ok()) { 
				WriteError(289,"No coinciden los tipos (|, O, & o Y). Los operandos deben ser logicos.");
				tipo=vt_error; 
				ev_return("");
			}
			s1 = Evaluar(expresion,p1a,p1b,t1);
			if (Inter.Running()) {
				if (op=='|' && s1==VERDADERO) ev_return(VERDADERO);
				if (op=='&' && s1==FALSO) ev_return(FALSO);
			}
			s2 = Evaluar(expresion,p2a,p2b,t2);
			if (!t1.cb_log || !t2.cb_log) { tipo=vt_error; ev_return(""); }
			if (op=='|')
				ev_return((s1==VERDADERO || s2==VERDADERO)?VERDADERO:FALSO);
			else
				ev_return((s1==VERDADERO && s2==VERDADERO)?VERDADERO:FALSO);
			
		case '~': case '!':
			if (op!='!'||next!='=') {
				tipo=vt_logica; // el resultado es logico
				// el operando debe ser logico
				if (p2a>p2) // tal vez nunca se llegue a este error, porque lo detecta en otro lado
					WriteError(290,"Falta operando para la negacion (~/NO).");
				if (t2!=vt_logica && !AplicarTipo(expresion,p2a,p2,vt_logica)) { 
					WriteError(291,"No coinciden los tipos (~ o NO). El operando deben ser logicos.");
					tipo=vt_error; 
					ev_return("");
				}
				s2 = Evaluar(expresion,p2a,p2b,t2);
				ev_return(s2==VERDADERO?FALSO:VERDADERO);
			} else {
				next='>'; op='<';
			}
		case '<': case '>': {
			// los operandos no pueden ser logicos para <, >, <= o >=
			if ((op=='<' || op=='>')&& next!='>') {
				if (t1.cb_log) {
					t1.set(vt_caracter_o_numerica);
					AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica);
				}
				if (t2.cb_log) {
					t2.set(vt_caracter_o_numerica);
					AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica);
				}
				if (!t1.is_ok() || !t2.is_ok()) { 
					WriteError(292,"No coinciden los tipos (<, >, <= o >=). Los operandos deben ser logicos.");
					tipo=vt_error; 
					ev_return("");
				}
			}
		case '=': 
			tipo=vt_logica; // el resultado es logico
			// los operandos deben tener igual tipo
			tipo_var t=vt_desconocido;
			if (!t1.is_known()) { // no se conoce t1...
				if (t2.is_known()) { // ...pero si t2
					t=t2;
					if (!AplicarTipo(expresion,p1a,p1b,t2))
					{ tipo=vt_error; ev_return(""); }
				}
			} else { // se conoce t1
				if (!t2.is_known()) { // t2 no
					t=t1; /*int p2b=p2;*/ // ¿por que se redefinia p2b???
					if (!AplicarTipo(expresion,p2a,p2b,t1))
					{ tipo=vt_error; ev_return(""); }
				} else if (t1!=t2) {// t2 no coincide
					WriteError(207,"No coinciden los tipos (<, >, <=, >=, <> o =). No se puede comparar variables o expresiones de distinto tipo.");
					tipo=vt_error; 
					ev_return(""); 
				}
			}
			// evaluar operandos y aplicar operador
			s1 = Evaluar(expresion,p1a,p1b,t1);
			s2 = Evaluar(expresion,p2a,p2b,t2);
			if (!t1.is_ok() || !t2.is_ok()) {
				tipo = vt_error;
				ev_return("");
			} else {
				t.set(t1); t.set(t2);
				tipo = vt_logica;
				if (t.cb_num) { // comparaciones de numeros
#define _epsilon (1e-11)
					if (op=='<') {
						if (next=='>') {
							double diff=StrToDbl(s1)-StrToDbl(s2); if (diff<0) diff=-diff;
							ev_return(diff<_epsilon?FALSO:VERDADERO); // comparacion "difusa" para evitar problemas numericos
						}
						else if (next=='=')
							ev_return((StrToDbl(s1)-_epsilon)<=StrToDbl(s2)?VERDADERO:FALSO);
						else
							ev_return((StrToDbl(s1)+_epsilon)<StrToDbl(s2)?VERDADERO:FALSO);
					} else if (op=='>') {
						if (next=='=')
							ev_return((StrToDbl(s1)+_epsilon)>=StrToDbl(s2)?VERDADERO:FALSO);
						else
							ev_return((StrToDbl(s1)-_epsilon)>StrToDbl(s2)?VERDADERO:FALSO);
					} else {
						double diff=StrToDbl(s1)-StrToDbl(s2); if (diff<0) diff=-diff;
						ev_return(diff<_epsilon?VERDADERO:FALSO); // comparacion "difusa" para evitar problemas numericos
					}
				} else if (t==vt_logica) { // comparaciones de logicos
					if (op=='<' && next=='>') {
						ev_return((s1==VERDADERO)!=(s2==VERDADERO)?VERDADERO:FALSO);
					} else if (op=='=') {
						ev_return((s1==VERDADERO)==(s2==VERDADERO)?VERDADERO:FALSO);
					} else {
						tipo=vt_error;
						ev_return(FALSO);
					}
				} else { // comparaciones de cadenas
					if (op=='<') {
						if (next=='>')
							ev_return(s1!=s2?VERDADERO:FALSO);
						else if (next=='=')
							ev_return(s1<=s2?VERDADERO:FALSO);
						else
							ev_return(s1<s2?VERDADERO:FALSO);
					} else if (op=='>') {
						if (next=='=')
							ev_return(s1>=s2?VERDADERO:FALSO);
						else
							ev_return(s1>s2?VERDADERO:FALSO);
					} else
						ev_return(s1==s2?VERDADERO:FALSO);
				}
			}
		}
			
		case '+':
			if (lang[LS_ALLOW_CONCATENATION]) {
				if (t1.can_be(vt_logica) && !AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica)) {
					WriteError(293,"No coinciden los tipos (+). Los operandos deben ser numericos o caracter.");
					tipo = vt_error;
					ev_return("");
				} else t1.set(vt_caracter_o_numerica);
				if (t2.can_be(vt_logica) && !AplicarTipo(expresion,p2a,p2b,vt_caracter_o_numerica)) {
					WriteError(294,"No coinciden los tipos (+). Los operandos deben ser numericos o caracter.");
					tipo = vt_error;
					ev_return("");
				} else t2.set(vt_caracter_o_numerica);
				if (t1!=t2) {
					tipo_var ot1=t1, ot2=t2;
					t1.set(t2);
					t2.set(t1);
					if (!t1.is_ok() || !t2.is_ok() || (ot1!=t1 && !AplicarTipo(expresion,p1a,p1b,t1)) || (ot2!=t2 && !AplicarTipo(expresion,p2a,p2b,t2)) || t1!=t2) {
						tipo = vt_error;
						WriteError(295,"No coinciden los tipos (+). Los operandos deben ser de igual tipo.");
						ev_return("");
					}
				}
				if (t1==vt_numerica||t2==vt_numerica) tipo=vt_numerica;
				else if (t1==vt_caracter||t2==vt_caracter) tipo=vt_caracter;
				else tipo=vt_caracter_o_numerica;
				s1 = Evaluar(expresion,p1a,p1b,t1);
				s2 = Evaluar(expresion,p2a,p2b,t2);
				if (tipo==vt_caracter) 
					ev_return(s1+s2);
				else 
					ev_return(DblToStr(StrToDbl(s1)+StrToDbl(s2)));
			}
			
		case '-':case '*':case '/':case '^':case '%': {
			tipo=vt_numerica; // el resultado es numerico
			// los operandos deben ser numericos
			if ((t1!=vt_numerica && !AplicarTipo(expresion,p1a,p1b,vt_numerica)) ||
				(t2!=vt_numerica && !AplicarTipo(expresion,p2a,p2b,vt_numerica))) {
					WriteError(213,"No coinciden los tipos (+, -, *, /, ^, % o MOD). Los operandos deben ser numericos.");
					tipo=vt_error;
					ev_return("");
				}
			// evaluar operandos
			s1 = Evaluar(expresion,p1a,p1b,t1);
			s2 = Evaluar(expresion,p2a,p2b,t2);
			if (!t1.cb_num || !t2.cb_num) { tipo=vt_error; ev_return(""); }
			// operar
			double res=0, o1=StrToDbl(s1), o2=StrToDbl(s2);
			if (op=='+')
				res=o1+o2;
			else if (op=='-')
				res=o1-o2;
			else if (op=='/')
				if (o2==0) {
					if (Inter.Running()) {
						WriteError(296,"Division por cero");
						tipo=vt_error;
					}
					ev_return("0");
				} else
					res=o1/o2;
			else if (op=='*')
				res=o1*o2;
			else if (op=='%')
				if (o2==0) {
					if (Inter.Running()) {
						WriteError(297,"Division por cero");
						tipo=vt_error;
					}
					ev_return("0");
				} else {
					if (o1!=int(o1)||o2!=int(o2))
						WriteError(298,"Los operandos para el operador MOD deben ser enteros");
					res=int(o1)%int(o2);
				}
			else if (op=='^')
				res=pow(o1,o2);
			ev_return(DblToStr(res));
		}
		}
	}
	tipo=vt_error;
	ev_return("");
}

// wrapper para llamar al Evaluar que sigue desde SynCheck, para que verifique que no falten operandos al principio o al final, y aplique los tipos solo si la evaluación es correcta
string EvaluarSC(string expresion, tipo_var &tipo, tipo_var forced_tipo) {
	if (ignore_logic_errors) { tipo=forced_tipo; return ""; }
	// <<<el comentario que sigue ya no aplica porque eso ahora se verifica en SynCheckAux3>>
	// el evaluar comun admite operandos nulos en los extremos, porque pueden ser variables 
	// sin inicializar que han sido reemplazadas, esta version no 
//	if (expresion.size()) { ... }
	// primero evalua sin importar de que tipo deberia ser (para que el error de tipo lo dé afuera, mejor contextualizado)
	tipo=vt_desconocido; int p1=0, p2=expresion.size()-1;
	string retval=Evaluar(expresion,p1,p2,tipo);
	// si no habia nada raro, aplica el tipo a las variables
	if (forced_tipo!=vt_desconocido && tipo!=vt_error) AplicarTipo(expresion,p1,p2,forced_tipo);
	return retval;
}

// recibe la expresion a evaluar y el tipo con la info de que puede llegar a ser
// devuelve el resultado si se pudo evaluar y el tipo en tipo, sino "" y vt_error
string Evaluar(string expresion, tipo_var &tipo, tipo_var forced_tipo) {
	tipo=forced_tipo;
	int p1=0, p2=expresion.size()-1;
	if (forced_tipo!=vt_desconocido && !AplicarTipo(expresion,p1,p2,forced_tipo)) {tipo=vt_error; return "";}; // TODO: APLICAR EL TIPO QUE VIENE
//	tipo_var rt = DeterminarTipo(expresion,p1,p2);
	return Evaluar(expresion,p1,p2,tipo);
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
			int np=BuscarComa(str,++pp,p2)-1;
			tipo_var t=vt_numerica;
			Evaluar(str,pp,np,t);
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
		tipo_var t=vt_numerica;
		string ret = Evaluar(str,pp,np,t);
		if (!t.is_ok()) return false;
		if (ret.find(".",0)!=string::npos) {
			WriteError(301,string("Subindice no entero ("+ret+")"));
			return false;
		}
		int idx=atoi(ret.c_str());
		if (lang[LS_BASE_ZERO_ARRAYS]) {
			if (idx<0||idx>adims[i+1]-1) {
				WriteError(302,string("Subindice (")+ret+") fuera de rango (0..."+IntToStr(adims[i+1]-1)+")");
				return false;
			}
		} else {
			if (idx<1||idx>adims[i+1]) {
				WriteError(303,string("Subindice (")+ret+") fuera de rango (1..."+IntToStr(adims[i+1])+")");
				return false;
			}
		}
		nombre+=ret+",";
		pp=np+1;
	}
	str=nombre;
	str[str.size()-1]=')';
	
	return true;
}
