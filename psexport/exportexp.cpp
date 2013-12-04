#include <iomanip>
#include <stack>
#include <string>
#include <sstream>
#include "../pseint/utils.h"
#include "../pseint/new_evaluar.h"
#include "../pseint/new_funciones.h"
#include "../pseint/global.h"
#include "export_common.h"
#include "exportexp.h"
#include "export_cpp.h"
using namespace std;

static void Replace(string &src, int from, int to, string rep, unsigned int &i) {
	int l=to-from+1;
	src.replace(from,l,rep);
	i+=rep.size()-l;
}

string ToLowerExp(string s) {
//	if (s=="")
//		cerr<<"VACIO!";
	bool comillas=false;
	for(unsigned int i=0;i<s.size();i++) { 
		if (s[i]=='\"') comillas=!comillas;
		else if (!comillas) s[i]=tolower(s[i]);
	}
	return s;
}

string invert_expresion(string expr) {
	int i=0,iop=-1,nop=0,l=expr.size();
	bool comillas=false;
	while (i<l) {
		if (expr[i]=='\"') comillas=!comillas;
		else if (!comillas) {
			if (expr[i]=='>' || expr[i]=='<' || expr[i]=='!' || expr[i]=='=') {
				nop++; iop=i;
				if (expr[i+1]=='>' || expr[i+1]=='<' || expr[i+1]=='!' || expr[i+1]=='=') i++;
			}
		}
		i++;
	}
	if (nop==1) {
		i=iop;
		if (expr[i]=='='&&expr[i+1]=='=') { expr.replace(i,2,"!="); return expr; }
		if (expr[i]=='!'&&expr[i+1]=='=') { expr.replace(i,2,"=="); return expr; }
		if (expr[i]=='<'&&expr[i+1]=='=') { expr.replace(i,2,">"); return expr; }
		if (expr[i]=='>'&&expr[i+1]=='=') { expr.replace(i,2,"<"); return expr; }
		if (expr[i]=='<') { expr.replace(i,1,">=");  return expr; }
		if (expr[i]=='>') { expr.replace(i,1,"<=");  return expr; }
	}
	return string("!(")+expr+")";
}

// conversion de expresiones
string modificarConstante(string s,int diff) {
	
	int Neg=0; // Bandera de Numero negativo
	if (s[0]=='-') {s.erase(0,1); Neg=1;}
	double f=0,b,punto=0;
	int i=0;
	while ((int)s[i]!=0) {
		if (punto==0)
			if ((int)s[i]==46)
				punto=.1;
			else
				f=(f*10)+((int)s[i]-48);
		else {
			b=(int)s[i]-48;
			f+=b*punto;
			punto=punto/10; 
		}
		i++;
	};
	if (Neg==1) f=-f;
	
	stringstream r;
	r<<setprecision(10)<<f+diff;
	return r.str();
}

string buscarOperando(const string &exp, int comienzo, int direccion) {
	unsigned int i=comienzo;
	int parentesis=0;
//	if (exp[i]==')'||exp[i]==']') { i--; parentesis++; }
	while (i>=0 && i<exp.size() && (
		parentesis ||  exp[i]=='.' ||  exp[i]=='\'' || exp[i]=='\"' 
		|| (exp[i]>='0' && exp[i]<='9') || (exp[i]>='A' && exp[i]<='Z') || (exp[i]>='a' && exp[i]<='z') 
		|| (direccion>0 && (exp[i]=='[' || exp[i]=='('))
		|| (direccion<0 && (exp[i]==')' || exp[i]==']'))
		)){
//		|| exp[i]=='[' || exp[i]=='(' || (parentesis && (exp[i]==']' || exp[i]==')')))) {
			if (exp[i]=='\''||exp[i]=='\"') { i+=direccion; while (exp[i]!='\''&&exp[i]!='\"') i+=direccion; }
			if (exp[i]=='(' || exp[i]=='[') parentesis++;
			if (exp[i]==')' || exp[i]==']') parentesis--;  
			i+=direccion;
	}
	if (direccion>0) 
		return exp.substr(comienzo,i-comienzo);
	else
		return exp.substr(i+1,comienzo-i);
}

string colocarParentesis(const string &exp) {
	int i,parentesis=0,final=exp.size()-1;
	for (i=0;i<=final;i++) {
		if (exp[i]=='[' || exp[i]=='(') parentesis++;
		else if (exp[i]==']' || exp[i]==')') parentesis++;
		else if (!parentesis && exp[i]!='.' && exp[i]!='+' && exp[i]!='-' && !(exp[i]>='A' && exp[i]<='Z') && !(exp[i]>='0' && exp[i]<='9') && exp[i]!=' ') break;
	}
	if (i>final) return exp;
	return string("(")+exp+")";
}

string restarUno(string exp) {
	int i=exp.size()-1,fin=0,parentesis=0;
	bool numero=false;
	while (i>=0) {
		if (exp[i]=='[' || exp[i]=='(') {
			parentesis++;
		} else if (exp[i]==']' || exp[i]==')') {
			parentesis--;
		} else if (!parentesis) {
			if (numero && (exp[i]<'.' || exp[i]>'9')) {
				if (exp[i]=='+') {
					string res=modificarConstante(exp.substr(i+1,fin-i+1),-1);
					if (res=="0")
						return exp.replace(i,fin-i+2,"");
					else
						return exp.replace(i+1,fin-i+1,res);
				} else  if (exp[i]=='-') {
					string res=modificarConstante(exp.substr(i+1,fin-i+1),+1);
					if (res=="0") 
						return exp.replace(i,fin-i+2,"");
					else
						return exp.replace(i+1,fin-i+1,res);
				}
				else numero=false;
			} else if (!numero && exp[i]>='.' && exp[i]<='9') {
				numero=true;
				fin=i;
			}
		}
		i--;
	}
	if (numero)
		return exp.replace(0,fin+1,DblToStr(StrToDbl(exp.substr(0,fin+1))-1));
	return exp+"-1";
}

static void ReplaceOper(string &exp, unsigned int &i, string oper) {
	// evaluar el segundo operando para ver si es string (algunos operadores son iguales para numeros y cadenas en pseudocódigo pero diferentes en otros lenguajes)
	string oper2=buscarOperando(exp,i+oper.size(),+1);
	tipo_var t; Evaluar(oper2,t); 
	// obtener el operador/funcion que lo reemplaza
	string rep=exporter->get_operator(oper,t==vt_caracter);
	if (rep.size()>5 && rep.substr(0,5)=="func ") { // si el operador es reemplazado por una función, colocar los argumentos y reemplazar todo (operador y operandos)
		rep.erase(0,5);
		string oper1=buscarOperando(exp,i-1,-1); // todo: ojo que este busca en la parte de la cadena que ya está convertida, podría traer problemas
		int i0=i-oper1.size(); i+=oper2.size()+oper.size()-1;
		oper2=expresion(oper2);
		for(unsigned int j=0;j+3<rep.size();j++) { 
			if (rep[j]=='a' && rep[j+1]=='r' && rep[j+2]=='g') {
				if (rep[j+3]=='1') {
					Replace(rep,j,j+3,oper1,j); j+=3;
				} else if (rep[j+3]=='2') {
					Replace(rep,j,j+3,oper2,j); j+=3;
				}
			}
		}
		Replace(exp,i0,i,rep,i);
	} else // si el operador es reemplazado por otro operador, los operandos no se tocan
		Replace(exp,i,i+oper.length()-1,rep,i); 
	i+=oper.size()-1;
}

string expresion(string exp, tipo_var &tipo) {
//cerr<<"IN :"<<exp<<endl;	
	// las cadenas por ahora van con comillas simples (porque Evaluar las quiere así)
	for (unsigned int i=0;i<exp.size();i++) 
		if (exp[i]=='\"') exp[i]='\'';
	
	// pasar todo a mayúsculas, sacar caracteres no validos
	for (unsigned int i=0;i<exp.size();i++) {
		if (exp[i]=='\'') { i++; while (i<exp.size() && exp[i]!='\'') i++; } 
		else if (exp[i]>='a' && exp[i]<='z') exp[i]-=32;
		else if (exp[i]=='ñ' || exp[i]=='Ñ') exp[i]='N';
		else if (exp[i]=='á' || exp[i]=='Á') exp[i]='A';
		else if (exp[i]=='é' || exp[i]=='E') exp[i]='E';
		else if (exp[i]=='í' || exp[i]=='Í') exp[i]='I';
		else if (exp[i]=='ó' || exp[i]=='Ó') exp[i]='O';
		else if (exp[i]=='ú' || exp[i]=='Ú') exp[i]='U';
	}
	Evaluar(string(" ")+exp+" ",tipo); // ¿para qué eran los espacios??? 
	
	// reemplazar operadores y funciones matematicas, arreglar indices de arreglos
	exp+=",";
	stack<bool> esArreglo;
	esArreglo.push(false);
	stack<int> posicion;
	string sub;
	int id_start=0;
	esArreglo.push(false); posicion.push(0); // por si encontramos una coma y preguntamos por el esArreglo.top() sin haber metido nada antes
	for (unsigned int i=0;i<exp.size();i++) {
		
		// corregir identificadores de constantes
		if (id_start<=i && (exp[i]<'A'||exp[i]>'Z')&&(exp[i]<'0'||exp[i]>'9')&&exp[i]!='_') { // corregir identificadores de constantes
			string word=exp.substr(id_start,i-id_start);
			if (word=="VERDADERO"||word=="FALSO"||word=="PI") {
				Replace(exp,id_start,i-1,exporter->get_constante(word),i);
			} else {
				exp.replace(id_start,i-id_start,ToLowerExp(word));
			}
			id_start=i+1;
		}
		
		
		if (exp[i]=='\'') { // corregir cadenas de caracteres constantes
			int l=i++; while (i<exp.size() && exp[i]!='\'') i++;
			string cont=exp.substr(l+1,i-l-1);
			Replace(exp,l,i,exporter->make_string(cont),i);
			id_start=i+1;
		} else if (exp[i]=='[' or exp[i]=='(') { // arreglos, llamadas a funciones, o simplemente parentesis para denotar orden de operaciones
			// ver si es arreglo o funcion, o solo un parentesis de jerarquia
			if  ( i>0 && ((exp[i-1]>='0' && exp[i-1]<='9') || (exp[i-1]>='A' && exp[i-1]<='Z') || (exp[i-1]>='a' && exp[i-1]<='z')) ) { // si lo que hay antes parece identificador, sera arreglo o funcion, pero no parentesis por jerarquia de operaciones
				// determinar si es arreglo o funcion
				sub=buscarOperando(exp,i-1,-1);
				sub=ToUpper(sub); // para que EsFuncion lo reconozca
				if (EsFuncion(sub)) { // funcion, puede ser subproceso del usuario o funcion predefinida
					// buscar donde terminan los argumentos
					int parentesis=1; unsigned int fin=i;
					while(parentesis>0) { 
						fin++;
						if (exp[fin]=='[' || exp[fin]=='(') parentesis++;
						else if (exp[fin]==']' || exp[fin]==')') parentesis--;
					}
					string args=exporter->get_operator("{")+expresion(exp.substr(i+1,fin-i-1))+exporter->get_operator("}"); // argumentos, con parentesis incluidos
					int ini=i-sub.size(); i=fin;
					if (EsFuncionPredefinida(sub)) { // funcion predefinida del lenguaje
						Replace(exp,ini,fin,exporter->function(sub,args),i); // traduccion de la llamada con argumentos y todo
					} else { // subproceso del usuario
						Replace(exp,ini,fin,ToLower(sub)+args,i); // traduccion de la llamada con argumentos y todo
					}
					id_start=i+1; 
				} else { // aca empiezan los indices de arreglo
					Replace(exp,i,i,exporter->get_operator("["),i);
					posicion.push(i); esArreglo.push(true);
				}
			} else { // parentesis que solo indica orden de operaciones
				Replace(exp,i,i,exporter->get_operator("("),i);
				posicion.push(i); esArreglo.push(false);
			}
		}
		
		else if (exp[i]==',' && esArreglo.top()) { // la coma puede separar argumentos de una llamada a función o instrucción, o indices de arreglo... en el segundo caso...
			sub=exp.substr(posicion.top()+1,i-posicion.top()-1);
			if (base_zero_arrays) {
				sub=restarUno(sub);
				Replace(exp,posicion.top()+1,i-1,sub,i);
			}
			Replace(exp,i,i,exporter->get_operator(","),i);
			posicion.pop();	posicion.push(i);
		}
		
		else if (exp[i]==']' or exp[i]==')') { // se cierra un arreglo o un paréntesis por orden de operaciones (nunca deberia llegar con llamadas a funciones)
			if (esArreglo.top()) {
				sub=exp.substr(posicion.top()+1,i-posicion.top()-1);
				if (base_zero_arrays) {
					sub=restarUno(sub);
					Replace(exp,posicion.top()+1,i-1,sub,i);
				}
				Replace(exp,i,i,exporter->get_operator("]"),i);
			} else {
				Replace(exp,i,i,exporter->get_operator(")"),i);
			}
			esArreglo.pop(); posicion.pop();
		}
		
		// no recuerdo si estos operadores pueden aparecer o ya fueron reemplazados antes por SyntaxCheck
		else if (exp[i]=='&' && exp[i+1]=='&') exp.erase(i--,1);
		else if (exp[i]=='|' && exp[i+1]=='|') exp.erase(i--,1);
		else if (exp[i]=='=' && exp[i+1]=='=') exp.erase(i--,1);
		// operadores
		else if (exp[i]=='<' && exp[i+1]=='>') { ReplaceOper(exp,i,"<>"); id_start=i+1; }
		else if (exp[i]=='>' && exp[i+1]=='=') { ReplaceOper(exp,i,">="); id_start=i+1; }
		else if (exp[i]=='<' && exp[i+1]=='=') { ReplaceOper(exp,i,"<="); id_start=i+1; }
		else if (exp[i]=='=') { ReplaceOper(exp,i,"="); id_start=i+1; }
		else if (exp[i]=='&') { ReplaceOper(exp,i,"&"); id_start=i+1; }
		else if (exp[i]=='|') { ReplaceOper(exp,i,"|"); id_start=i+1; }
		else if (exp[i]=='+') { ReplaceOper(exp,i,"+"); id_start=i+1; }
		else if (exp[i]=='-') { ReplaceOper(exp,i,"-"); id_start=i+1; } 
		else if (exp[i]=='/') { ReplaceOper(exp,i,"/"); id_start=i+1; }
		else if (exp[i]=='*') { ReplaceOper(exp,i,"*"); id_start=i+1; }
		else if (exp[i]=='^') { ReplaceOper(exp,i,"^"); id_start=i+1; }
		else if (exp[i]=='%') { ReplaceOper(exp,i,"%"); id_start=i+1; }
		else if (exp[i]=='<') { ReplaceOper(exp,i,"<"); id_start=i+1; }
		else if (exp[i]=='>') { ReplaceOper(exp,i,">"); id_start=i+1; }
		else if (exp[i]=='~') { ReplaceOper(exp,i,"~"); id_start=i+1; }
		
	}
	exp=exp.substr(0,exp.size()-1);
//cerr<<"OUt:"<<exp<<endl;	
	return exp;
}

string expresion(string exp) {
	tipo_var t;
	return expresion(exp,t);
}
