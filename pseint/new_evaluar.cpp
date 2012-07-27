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
using namespace std;

void WriteError(int num, string s) { 
	if (Inter.Running())
		ExeError(num,s);
	else
		SynError(num,s);
}

// -------------------------------------------------------------------
//    Convierte una cadena a un double
// -------------------------------------------------------------------
double StrToDbl(string s) {
	int Neg=0; // Bandera de Numero negativo
	int i=0;
	if (s[0]=='-') {i++; Neg=1;}
	else if (s[0]=='+') i++;
	double f=0,b,punto=0;
	while ((int)s[i]!=0) {
		if (punto==0)
			if ((int)s[i]==46)
				punto=.1;
			else
				f=(f*10)+((int)s[i]-48);
		else {
			b=(int)s[i]-48;
			f+=b*punto;
			punto=punto/10; }
		i++;};
	if (Neg==1) f=-f;
	return f;
}

// -------------------------------------------------------------------
//    Convierte un double a una cadena - Alta Precision
// -------------------------------------------------------------------
string DblToStr(double f, int pres) {
	stringstream s;
	s<<fixed<<setprecision(pres)<<f;
	string r=s.str();
	if (r.find('.',0)!=string::npos || r.find(',',0)!=string::npos) {
		int p=r.size()-1;
		while (r[p]=='0') p--;
		if (r[p]==','||r[p]=='.') p--;
		r=r.substr(0,p+1);
	}
		
	return r;
}

// -------------------------------------------------------------------
//    Arregla un doble de una cadena... quita precision para evitar
//  cosas como 5+2=6.99999857
// -------------------------------------------------------------------
//string FixDblStr(string dbl) {
//	stringstream s;
//	s<<fixed<<setprecision(10)<<StrToDbl(dbl);
//	string str = s.str();
//	size_t p = str.find('.',0);
//	if (p!=string::npos) {
//		int s = str.size();
//		while (str[s-1]=='0')
//			s--;
//		if (str[s-1]=='.')
//			s--;
//		str.erase(s,str.size()-s);
//	}
//	return str;
// 0}

bool PalabraReservada(string str, bool exclude_functions=false) {
	// Comprobar que no sea palabra reservada
	if (word_operators && (str=="Y" || str=="O" || str=="NO" || str=="MOD"))
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
	if (!exclude_functions && funciones.find(str)!=funciones.end())
		return true;
	return false;
}

int BuscarComa(string &expresion, int p1, int p2, char coma) {
	bool comillas = false;
	int parentesis = 0;
	char c;
	while (p1<=p2) {
		c = expresion[p1];
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

int operadores[]={',','|','&','~','=','<','>','+','-','*','/','^','%',' '};

int BuscarOperador(string &expresion, int &p1, int &p2) {
	bool parentesis_externos=true, comillas;
	int i,j, parentesis, indice_operador=999, posicion_operador=-1; 
	char c;
	while (parentesis_externos) {
		c=expresion[i=p1];
		while (i<=p2 && (c==' '||c=='\t'))
			c=expresion[++i];
		parentesis_externos = i<=p2&&c=='(';
		comillas=false;
		parentesis=0;
		while(i<=p2) {
			if (c=='\"' || c=='\'') {
				comillas = !comillas;
			} else if (!comillas) {
				if (c=='(' || c=='[')
					parentesis++;
				else if (c==')' || c==']') {
					parentesis--;
				} else if (parentesis==0) {
					parentesis_externos=false;
					j=0;
					while (c!=operadores[j] && operadores[j]!=' ')
						j++;
					if (operadores[j]!=' ') {
						if (j<indice_operador) {
							posicion_operador=i;
							indice_operador=j;
							char nc=expresion[i+1];
							if ( (c=='<'||c=='>') && (nc=='=' || nc=='>') ) i++;
						}
//						i++;
					}
				}
			}
			c=expresion[++i];
		}
		if (parentesis_externos) {
			p1++; p2--;
		}
	}
	if (posicion_operador!=-1 && operadores[indice_operador]!=' ') {
		if (operadores[indice_operador]=='-') {
			int np1=posicion_operador+1,np2=p2;
			int aux=BuscarOperador(expresion,np1,np2);
			if (aux!=-1 && (expresion[aux]=='-' || expresion[aux]=='+')) return aux;
		}
		return posicion_operador;
	} else
		return -1;
}

tipo_var DeterminarTipo(string &expresion, int p1, int p2) {
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
				if (funciones.find(expresion.substr(p1,pp-p1))!=funciones.end())
					return vt_numerica;
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
			if (allow_concatenation) {
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

bool AplicarTipo(string &expresion, int &p1, int &p2, tipo_var tipo) {
	if (p2<p1) return true;
	int pos_op = BuscarOperador(expresion,p1,p2);
	if (pos_op==-1) { // si no hay operador, es una variable o constante
//		size_t pp=expresion.find('(');
//		size_t pc=expresion.find('[');
//		if (pp!=string::npos) {
//			if (pc!=string::npos && pc<pp)
//				return memoria->DefinirTipo(expresion.substr(p1,pc-p1+1),tipo);
//			else
//				return memoria->DefinirTipo(expresion.substr(p1,pp-p1+1),tipo);
//		} if (pc!=string::npos)
//			return memoria->DefinirTipo(expresion.substr(p1,pc-p1+1),tipo);
//		else
		
		if (expresion.substr(p1,p2-p1+1)==VERDADERO||expresion.substr(p1,p2-p1+1)==FALSO) return tipo.cb_log;
		if (expresion[p1]=='.'||(expresion[p1]>='0'&&expresion[p1]<='9') ) return tipo.cb_num;
		if (expresion[p1]=='\'') return tipo.cb_car;
		return memoria->DefinirTipo(expresion.substr(p1,p2-p1+1),tipo);
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

//#define LOG_EVALUAR

#ifdef LOG_EVALUAR
string ev_aux(string a,int &tabs)
{ cerr<<setw(tabs)<<""<<"RET: *"<<(a)<<"*\n"; tabs-=2; return a;}
#define ev_return(a) return ev_aux(a,tabs)
#else
#define ev_return(a) return a
#endif

string Evaluar(string &expresion, int &p1, int &p2, tipo_var &tipo) {
	while (p1<p2&&expresion[p1]==' ') p1++;
	while (p1<p2&&expresion[p2]==' ') p2--;
#ifdef LOG_EVALUAR
	static int tabs=0;
	tabs+=2;
	cerr<<setw(tabs)<<""<<"EVALUAR: *"<<expresion.substr(p1,p2-p1+1)<<"*\n";
#endif
	int pos_op = BuscarOperador(expresion,p1,p2);
	if (pos_op!=-1 && expresion[pos_op]==',') { 
		WriteError(999,string("Se esperaba solo una expresión")); tipo=vt_error; return "";
	}
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
				if (PalabraReservada(nombre,true)) {
					WriteError(999,string("Identificador no valido (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				if (EsFuncion(nombre)) {
					WriteError(999,string("Faltan parametros para la funcion (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				tipo = memoria->LeerTipo(nombre);
				if (tipo.dims) {
					WriteError(220,string("Faltan subindices para el arreglo (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				if (force_var_definition && !memoria->EstaDefinida(nombre)) {
					WriteError(210,string("Variable no definida (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				if (!allow_undef_vars && !memoria->EstaInicializada(nombre)) {
					WriteError(215,string("Variable no inicializada (")+nombre+")");
					tipo=vt_error;
					ev_return("");
				}
				ev_return(memoria->LeerValor(nombre));
			} else { // si es un arreglo o funcion
				int pcierra = BuscarComa(expresion,pm+1,p2,')');
				if (pcierra!=p2) {
					while (pcierra<=p2 && expresion[pcierra]==' ') pcierra++;
					if (pcierra<=p2) {
						WriteError(999,string("Falta operador (")+expresion.substr(p1,p2-p1+1)+")");
						tipo=vt_error;
						ev_return("");
					}
				}
				string nombre=expresion.substr(p1,pm-p1);
				funcion *func=EsFuncion(nombre);
				if (func) { //si es funcion
					// controlar cantidad de argumentos/indices
					int b=pm,ca=1;
					while ((b=BuscarComa(expresion,b+1,p2))>0) ca++;
					if (func->cant_arg!=ca) {
						tipo=vt_error;
						WriteError(999,string("Cantidad de argumentos incorrecta para la funcion (")+nombre+(")"));
						ev_return("0");
					}

					// parsear argumentos
					string *args=new string[ca];
					b=pm; int b2;
					for (int i=0;i<ca;i++) {
						b2=BuscarComa(expresion,b+1,p2);
						if (b2==-1) b2=p2;
						int p1=b+1, p2=b2-1;
						args[i]=Evaluar(expresion,p1,p2,tipo);
					}
					if (tipo==vt_error) {
						delete [] args;
						ev_return("0");
					}
					// obtener salida
					string ret=func->func(args,tipo);
					delete [] args;
					ev_return(ret);
				} else {
					if (PalabraReservada(nombre,true)) {
						WriteError(999,string("Identificador no valido (")+nombre+")");
						tipo=vt_error;
						ev_return("");
					}
					if (force_var_definition && !memoria->EstaDefinida(nombre)) {
						WriteError(209,string("Variable no definida (")+nombre+")");
						tipo=vt_error;
						ev_return("");
					}
					string aux=expresion.substr(p1,p2-p1+1);
					if (CheckDims(aux)) {
						if (!allow_undef_vars && !memoria->EstaInicializada(aux)) {
							WriteError(999,string("Posición no inicializada (")+aux+")");
							tipo=vt_error;
							ev_return("");
						}
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
		char op = expresion[pos_op];
		int p1b=pos_op-1, p2a=pos_op+1;
		if ((op=='<'||op=='>') && expresion[pos_op+1]=='=') p2a++;
		if (op=='<'&&expresion[pos_op+1]=='>') p2a++;
		int p1a=p1, p2b=p2;
		t1=DeterminarTipo(expresion,p1a,p1b);
		t2=DeterminarTipo(expresion,p2a,p2b);
		if ((op!='~'&&!t1.is_ok()) || !t2.is_ok()) { tipo=vt_error; ev_return(""); }
		// analizar segun operador
		switch (op) {
			
		case '|': case '&': 
			tipo=vt_logica; // el resultado es logico
			// los operandos deben ser logicos
			if (!t1.is_known()) AplicarTipo(expresion,p1a,p1b,vt_logica);
			if (!t2.is_known()) AplicarTipo(expresion,p2a,p2b,vt_logica);
			if (!t1.is_ok() || !t2.is_ok()) { 
				WriteError(999,"No coinciden los tipos (|, O, & o Y). Los operandos deben ser logicos.");
				tipo=vt_error; 
				ev_return("");
			}
			s1 = Evaluar(expresion,p1a,p1b,t1);
			s2 = Evaluar(expresion,p2a,p2b,t2);
			if (!t1.cb_log || !t2.cb_log) { tipo=vt_error; ev_return(""); }
			if (op=='|')
				ev_return((s1==VERDADERO || s2==VERDADERO)?VERDADERO:FALSO);
			else
				ev_return((s1==VERDADERO && s2==VERDADERO)?VERDADERO:FALSO);
			
		case '~':
			tipo=vt_logica; // el resultado es logico
			// el operando debe ser logico
			if (p2a>=p2)
				WriteError(999,"Falta operando para la negacion (~/NO).");
			if (t2!=vt_logica && !AplicarTipo(expresion,p2a,p2,vt_logica)) { 
				WriteError(999,"No coinciden los tipos (~ o NO). El operando deben ser logicos.");
				tipo=vt_error; 
				ev_return("");
			}
			s2 = Evaluar(expresion,p2a,p2b,t2);
			ev_return(s2==VERDADERO?FALSO:VERDADERO);
			
		case '<': case '>': {
			// los operandos no pueden ser logicos para <, >, <= o >=
			if ((op=='<' || op=='>')&& expresion[pos_op+1]!='>') {
				if (t1.cb_log) {
					t1.set(vt_caracter_o_numerica);
					AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica);
				}
				if (t2.cb_log) {
					t2.set(vt_caracter_o_numerica);
					AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica);
				}
				if (!t1.is_ok() || !t2.is_ok()) { 
					WriteError(999,"No coinciden los tipos (<, >, <= o >=). Los operandos deben ser logicos.");
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
					t=t1; int p2b=p2;
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
					if (op=='<') {
						if (expresion[pos_op+1]=='>')
							ev_return(StrToDbl(s1)!=StrToDbl(s2)?VERDADERO:FALSO);
						else if (expresion[pos_op+1]=='=')
							ev_return(StrToDbl(s1)<=StrToDbl(s2)?VERDADERO:FALSO);
						else
							ev_return(StrToDbl(s1)<StrToDbl(s2)?VERDADERO:FALSO);
					} else if (op=='>') {
						if (expresion[pos_op+1]=='=')
							ev_return(StrToDbl(s1)>=StrToDbl(s2)?VERDADERO:FALSO);
						else
							ev_return(StrToDbl(s1)>StrToDbl(s2)?VERDADERO:FALSO);
					} else {
						double diff=StrToDbl(s1)-StrToDbl(s2); if (diff<0) diff=-diff;
						ev_return(diff<1e-11?VERDADERO:FALSO); // comparacion "difusa" para evitar problemas numericos
					}
				} else if (t==vt_logica) { // comparaciones de logicos
					if (op=='<' && expresion[pos_op+1]=='>') {
						ev_return((s1==VERDADERO)!=(s2==VERDADERO)?VERDADERO:FALSO);
					} else if (op=='=') {
						ev_return((s1==VERDADERO)==(s2==VERDADERO)?VERDADERO:FALSO);
					} else {
						tipo=vt_error;
						ev_return(FALSO);
					}
				} else { // comparaciones de cadenas
					if (op=='<') {
						if (expresion[pos_op+1]=='>')
							ev_return(s1!=s2?VERDADERO:FALSO);
						else if (expresion[pos_op+1]=='=')
							ev_return(s1<=s2?VERDADERO:FALSO);
						else
							ev_return(s1<s2?VERDADERO:FALSO);
					} else if (op=='>') {
						if (expresion[pos_op+1]=='=')
							ev_return(s1>=s2?VERDADERO:FALSO);
						else
							ev_return(s1>s2?VERDADERO:FALSO);
					} else
						ev_return(s1==s2?VERDADERO:FALSO);
				}
			}
		}
			
		case '+':
			if (allow_concatenation) {
				if (t1.can_be(vt_logica) && !AplicarTipo(expresion,p1a,p1b,vt_caracter_o_numerica)) {
					WriteError(999,"No coinciden los tipos (+). Los operandos deben ser numericos o caracter.");
					tipo = vt_error;
					ev_return("");
				} else t1.set(vt_caracter_o_numerica);
				if (t2.can_be(vt_logica) && !AplicarTipo(expresion,p2a,p2b,vt_caracter_o_numerica)) {
					WriteError(999,"No coinciden los tipos (+). Los operandos deben ser numericos o caracter.");
					tipo = vt_error;
					ev_return("");
				} else t2.set(vt_caracter_o_numerica);
				if (t1!=t2) {
					tipo_var ot1=t1, ot2=t2;
					t1.set(t2);
					t2.set(t1);
					if (!t1.is_ok() || !t2.is_ok() || (ot1!=t1 && !AplicarTipo(expresion,p1a,p1b,t1)) || (ot2!=t2 && !AplicarTipo(expresion,p2a,p2b,t2)) || t1!=t2) {
						tipo = vt_error;
						WriteError(999,"No coinciden los tipos (+). Los operandos deben ser de igual tipo.");
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
			
			// ver que pasa si se suman cadenas
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
						WriteError(999,"Division por cero");
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
						WriteError(999,"Division por cero");
						tipo=vt_error;
					}
					ev_return("0");
				} else {
					if (o1!=int(o1)||o2!=int(o2))
						WriteError(999,"Los operandos para el operador MOD deben ser enteros");
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
	// el evaluar comun admite operandos nulos en los extremos, porque pueden ser variables 
	// sin inicializar que han sido reemplazadas, esta version no
	if (expresion.size()) {
		char c0=expresion[0];
		char c1=expresion[expresion.size()-1];
		if (c0=='*'||c0=='='||c0=='<'||c0=='/'||c0=='&'||c0=='%'||c0=='>'||c0=='|'||c0=='^') {
			SynError (999,"Falta un operando al comienzo de la expresión."); /*errores++;*/
		}
		if (c1=='*'||c1=='='||c1=='<'||c1=='/'||c1=='&'||c1=='%'||c1=='>'||c1=='|'||c1=='^') {
			SynError (999,"Falta un operando al comienzo de la expresión."); /*errores++;*/
		}
	}
	// primero evalua sin importar de que tipo deberia ser (para que el error de tipo lo de afuera, mejor contextualizado)
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
	int *adims=memoria->LeerDims(str);
	if (!adims) {
		WriteError(202,string("El identificador ")+str.substr(0,pp)+(" no corresponde a un arreglo"));
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
			string ret = Evaluar(str,pp,np,t);
			pp=np+1;
			if (!t.is_ok()) return false;
		}
		// controlar cantidad de dimensiones
		if (adims[0]!=ca) {
			WriteError(999,string("Cantidad de indices incorrecta para el arreglo (")+nombre+(")"));
			return false;
		}
		
		// parche horrible para marcar los indices de arreglos como enteros para que no sean float al exportar a c++
		if (force_integer_indexes) {
			string exp=str.substr(str.find("(")+1)+","; int par=0;
			for (unsigned int i=0,l=0;i<exp.size();i++) {
				if (exp[i]=='\"') {
					i++;
					while (i<exp.size() && exp[i]!='\"')
						i++;
					i++;
					l=i+1;
				} else if ((exp[i]<'a'||exp[i]>'z')&&(exp[i]<'0'||exp[i]>'9')&&exp[i]!='_') {
					if (par==0) {
						string nombre=exp.substr(l,i-l);
//						cerr<<"***"<<nombre<<"***\n";
						if (memoria->Existe(nombre)) 
							memoria->DefinirTipo(nombre,vt_numerica,true);
					}
					l=i+1;
					if (exp[i]=='['||exp[i]=='(') par++;
					else if (exp[i]==']'||exp[i]==')') par--;
				}
			}
		}
		return true;
	} 
	
	int b=pp,ca=1; while ((b=BuscarComa(str,b+1,p2))>0) ca++;
	if (adims[0]!=ca) {
		WriteError(999,string("Cantidad de indices incorrecta para el arreglo (")+nombre+(")"));
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
			WriteError(999,string("Subindice no entero ("+ret+")"));
			return false;
		}
		int idx=atoi(ret.c_str());
		if (base_zero_arrays) {
			if (idx<0||idx>adims[i+1]-1) {
				WriteError(999,string("Subindice (")+ret+") fuera de rango (0..."+IntToStr(adims[i+1]-1)+")");
				return false;
			}
		} else {
			if (idx<1||idx>adims[i+1]) {
				WriteError(999,string("Subindice (")+ret+") fuera de rango (1..."+IntToStr(adims[i+1])+")");
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
