#include <iomanip>
#include <stack>
#include <string>
#include <sstream>
#include "../pseint/utils.h"
#include "../pseint/new_evaluar.h"
#include "../pseint/new_funciones.h"
#include "../pseint/global.h"
#include "export_chooser.h"
using namespace std;

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
		if (expr[i]=='='&&expr[i+1]=='=') { expr.replace(i,2,"!=");  return expr; }
		if (expr[i]=='!'&&expr[i+1]=='=') { expr.replace(i,2,"==");  return expr; }
		if (expr[i]=='<'&&expr[i+1]=='=') { expr.replace(i,2,">");  return expr; }
		if (expr[i]=='>'&&expr[i+1]=='=') { expr.replace(i,2,"<");  return expr; }
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
	while (i>=0 && i<exp.size() && (parentesis ||  exp[i]=='.' || (exp[i]>='0' && exp[i]<='9') || (exp[i]>='a' && exp[i]<='z') || exp[i]=='[' || exp[i]=='(' || exp[i]==']' || exp[i]==')')) {
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
		else if (!parentesis && exp[i]!='.' && exp[i]!='+' && exp[i]!='-' && !(exp[i]>='a' && exp[i]<='z') && !(exp[i]>='0' && exp[i]<='9') && exp[i]!=' ') break;
	}
	if (i>final) return exp;
	return string("(")+exp+")";
}

string restarUno(string exp) {
	if (base_zero_arrays) return exp;
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

string expresion(string exp);

string expresion(string exp, tipo_var &tipo){
	for (unsigned int i=0;i<exp.size();i++) if(exp[i]=='\"') exp[i]='\'';
	
//	if (!exp.size()) return exp;
	// pasar todo a mayúsculas y sacar caracteres no validos
	for (unsigned int i=0;i<exp.size();i++) 
		if (exp[i]=='\'') {
			exp[i]='\"';
			i++;
			while (i<exp.size() && exp[i]!='\'') {
				if (exp[i]=='\\') { exp.insert(i,"\\"); i++; }
				i++;
			}
			exp[i]='\"';
		} else if (exp[i]>='a' && exp[i]<='z') exp[i]-=32;
		else if (exp[i]=='ñ' || exp[i]=='Ñ') exp[i]='N';
		else if (exp[i]=='á' || exp[i]=='Á') exp[i]='A';
		else if (exp[i]=='é' || exp[i]=='E') exp[i]='E';
		else if (exp[i]=='í' || exp[i]=='Í') exp[i]='I';
		else if (exp[i]=='ó' || exp[i]=='Ó') exp[i]='O';
		else if (exp[i]=='ú' || exp[i]=='Ú') exp[i]='U';
	
	
	// cambiar verdadero y falso por VERDADERO y FALSO para que evaluar los reconozca
//	exp=exp+",";
//	for (unsigned int i=0,l=0;i<exp.size();i++) {
//		if (exp[i]=='\"') {
//			i++;
//			while (i<exp.size() && exp[i]!='\"')
//				i++;
//			i++;
//			l=i+1;
//		} else if ((exp[i]<'a'||exp[i]>'z')&&(exp[i]<'0'||exp[i]>'9')&&exp[i]!='_') {
//			if (exp.substr(l,i-l)=="verdadero") exp.replace(l,i-l,"VERDADERO");
//			else if (exp.substr(l,i-l)=="falso") exp.replace(l,i-l,"FALSO"); 
//			l=i+1;
//		}
//	}
//	exp=exp.substr(0,exp.size()-1);
	
	Evaluar(string(" ")+exp+" ",tipo); // ¿para que los espacios??? 
	
	// cambiar VERDADERO y FALSO por true y false
//	exp=exp+",";
//	for (unsigned int i=0,l=0;i<exp.size();i++) {
//		if (exp[i]=='\"') {
//			i++;
//			while (i<exp.size() && exp[i]!='\"')
//				i++;
//			i++;
//			l=i+1;
//		} else if ((exp[i]<'a'||exp[i]>'z')&&(exp[i]<'A'||exp[i]>'Z')&&(exp[i]<'0'||exp[i]>'9')&&exp[i]!='_') {
//			if (exp.substr(l,i-l)=="VERDADERO") { exp.replace(l,i-l,"true"); i=i-5; }
//			else if (exp.substr(l,i-l)=="FALSO") { exp.replace(l,i-l,"false"); i=i-5; }
//			l=i+1;
//		}
//	}
//	exp=exp.substr(0,exp.size()-1);
	// reemplazar operadores y funciones matematicas, arreglar indices de arreglos
	stack<bool> esArreglo;
	esArreglo.push(false);
	stack<int> posicion;
	string sub;
	if (exp[0]=='[' || exp[0]=='(') { 
		esArreglo.push(false);
		posicion.push(0);
	}
	for (unsigned int i=0;i<exp.size();i++) {
		if (exp[i]=='\"') {
			i++;
			while (i<exp.size() && exp[i]!='\"')
				i++;
		} else if (exp[i]=='[' or exp[i]=='(') {
			posicion.push(i);
			// ver si es arreglo o funcion, o solo un parentesis de jerarquia
			if  ( i>0 && ((exp[i-1]>='0' && exp[i-1]<='9') || (exp[i-1]>='a' && exp[i-1]<='z')) ) {
				// determinar si es arreglo o funcion
				sub=buscarOperando(exp,i-1,-1);
				if (EsFuncion(sub)) {
					esArreglo.push(false);
					exp[i]='(';
					int parentesis=1,fin=i;
					while(parentesis>0) {
						fin++;
						if (exp[fin]=='[' || exp[fin]=='(') parentesis++;
						else if (exp[fin]==']' || exp[fin]==')') parentesis--;
					}
					exp[fin]=')';
					if (EsFuncionPredefinida(sub)) {
						string args="("+expresion(exp.substr(i+1,fin-i-1))+")";
						string translated=translate_function(sub,args);
						exp.replace(i-sub.size(),fin-(i-sub.size())+1,translated);
						i=i-sub.size()+translated.size()-1;
					}
				} else {
					esArreglo.push(true);
					exp[i]='[';
				}
			} else {
				esArreglo.push(false);
				exp[i]='(';
			}
		}
		else if (exp[i]==',' && esArreglo.top()) {
			sub=restarUno(exp.substr(posicion.top()+1,i-posicion.top()-1));
			exp.replace(posicion.top()+1,i-posicion.top()-1,sub);
			i+=sub.size()-i+posicion.top()+1;
			exp.replace(i++,1,"][");
			posicion.pop();
			posicion.push(i);
		}
		else if (exp[i]==']' or exp[i]==')') {
			if (esArreglo.top()) {
				sub=restarUno(exp.substr(posicion.top()+1,i-posicion.top()-1));
				exp.replace(posicion.top()+1,i-posicion.top()-1,sub);
				i+=sub.size()-i+posicion.top()+1;
				exp[i]=']';
			} else 
				exp[i]=')';
			esArreglo.pop();
			posicion.pop();
		} 
		else if (exp[i]=='^') {
			string s1=buscarOperando(exp,i-1,-1),s2=buscarOperando(exp,i+1,+1);
			exp[i]=',';
			if (s2[0]=='(' && s2[s2.size()-1]==')') {
				exp.erase(i+1,1);
			} else
				exp.insert(i+s2.size()+1,")");
				if (s1[0]=='(' && s1[s1.size()-1]==')') {
					exp.erase(i-1,1);
					exp.insert(i-s1.size(),"powf");
					i-=2;
				} else
					exp.insert(i-s1.size(),"powf(");
				i+=5;
				esArreglo.push(false);
				posicion.push(i);
		} 
		else if (exp[i]=='<' && exp[i+1]=='>'){
			exp[i]='!';
			exp[++i]='=';
		}
		else if (exp[i]=='=' && exp[i-1]!='>' && exp[i-1]!='<'){
			exp.insert(i,"=");
			i++;
		}
		else if (exp[i]=='|'){
			exp.insert(i,"|");
			i++;
		}
		else if (exp[i]=='&'){
			exp.insert(i,"&");
			i++;
		}
		else if (exp[i]=='~'){
			exp[i]='!';
			i++;
		}
	}
	return ToLower(exp);
}

string expresion(string exp) {
	tipo_var t;
	return expresion(exp,t);
}
