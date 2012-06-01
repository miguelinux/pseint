#include<iostream>
#include<iomanip>
#include<stack>

#include "../pseint/new_memoria.h"
#include "../pseint/intercambio.h"
#include "../pseint/utils.h"
#include "../pseint/new_evaluar.h"
using namespace std;

bool include_cmath=false;
bool include_cstdlib=false;

// resolucion de tipos
void declarar_variables(t_programa &prog, bool &use_sin_tipo, bool &use_string) {
	
	// saltear comentarios??
	t_programa::iterator it = prog.begin();
	while (*it!=MAIN_LINE_TEXT) it++; it++; it++;
	
	map<string,tipo_var>::iterator mit=memoria->var_info.begin(), mit2=memoria->var_info.end();
	string tab("\t"),dims, c1="[", c2="]", stipo;
	
	use_sin_tipo=false;
	use_string=false;
	
	while (mit!=mit2) {
		tipo_var &t=mit->second;
		stipo="SIN_TIPO ";
		if (t==vt_caracter) { stipo="string "; use_string=true; }
		else if (t==vt_numerica) stipo=t.rounded?"int ":"float ";
		else if (t==vt_logica) stipo="bool ";
		else use_sin_tipo=true;
		if (t.dims) {
			dims="";
			for (int j=1;j<=t.dims[0];j++)
				dims+=c1+IntToStr(t.dims[j])+c2;
			prog.insert(it,tab+stipo+toLower(mit->first)+dims+";");
		} else {
			prog.insert(it,tab+stipo+toLower(mit->first)+";");
		}
		mit++;
	}
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
	return r	.str();
}

string buscarOperando(const string &exp, int comienzo, int direccion){
	unsigned int i=comienzo;
	int parentesis=0;
	if (exp[i]=='(' || exp[i]=='[') parentesis++;
	if (exp[i]==')' || exp[i]==']') parentesis--;
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
bool colocarParentesis(string &exp, int comienzo, int final){
	int i,parentesis=0;
	for (i=0;i<=final;i++) {
		if (exp[i]=='[' || exp[i]=='(') parentesis++;
		else if (exp[i]==']' || exp[i]==')') parentesis++;
		else if (!parentesis && exp[i]!='.' && exp[i]!='+' && exp[i]!='-' && !(exp[i]>='a' && exp[i]<='z') && !(exp[i]>='0' && exp[i]<='9') && exp[i]!=' ') break;
	}
	if (i>final) return false;
	exp.insert(comienzo,"(");
	exp.insert(final+2,")");
	return true;
}

string restarUno(string exp){
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

string expresion(string exp, tipo_var &tipo){
//	cerr<<exp<<endl;
	for (unsigned int i=0;i<exp.size();i++) if(exp[i]=='\"') exp[i]='\'';
//	cerr<<
//	<<endl;

//	if (!exp.size()) return exp;
	// pasar todo a minusculas sacar caracteres no validos
	for (unsigned int i=0;i<exp.size();i++) 
		if (exp[i]=='\'') {
			exp[i]='\"';
			i++;
			while (i<exp.size() && exp[i]!='\'')
				i++;
			exp[i]='\"';
		} else if (exp[i]>='A' && exp[i]<='Z') exp[i]+=32;
		else if (exp[i]=='Ñ' || exp[i]=='ñ') exp[i]='n';
		else if (exp[i]=='Á' || exp[i]=='á') exp[i]='a';
		else if (exp[i]=='É' || exp[i]=='é') exp[i]='e';
		else if (exp[i]=='I' || exp[i]=='í') exp[i]='i';
		else if (exp[i]=='Ó' || exp[i]=='ó') exp[i]='o';
		else if (exp[i]=='Ú' || exp[i]=='ú') exp[i]='u';
	
	Evaluar(string(" ")+exp+" ",tipo);
	
	// cambiar VERDADERO y FALSO por true y false
	exp=exp+",";
	for (unsigned int i=0,l=0;i<exp.size();i++) {
		if (exp[i]=='\"') {
			i++;
			while (i<exp.size() && exp[i]!='\"')
				i++;
			i++;
			l=i+1;
		} else if ((exp[i]<'a'||exp[i]>'z')&&(exp[i]<'0'||exp[i]>'9')&&exp[i]!='_') {
			if (exp.substr(l,i-l)=="verdadero") { exp.replace(l,i-l,"true"); i=i-5; }
			else if (exp.substr(l,i-l)=="falso") { exp.replace(l,i-l,"false"); i=i-5; }
			l=i+1;
		}
	}
	exp=exp.substr(0,exp.size()-1);
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
			i++;
		} else if (exp[i]=='[' or exp[i]=='(') {
			posicion.push(i);
			// ver si es arreglo o funcion, o solo un parentesis de jerarquia
			if  ( i>0 && ((exp[i-1]>='0' && exp[i-1]<='9') || (exp[i-1]>='a' && exp[i-1]<='z')) ) {
				// determinar si es arreglo o funcion
				sub=buscarOperando(exp,i-1,-1);
				if (sub=="sen") {
					esArreglo.push(false);
					exp.replace(i-sub.size(),sub.size()+1,"sin(");
					include_cmath=true;
				} else if (sub=="tan") {
					esArreglo.push(false);
					exp[i]='(';
					include_cmath=true;
				} else if (sub=="asen") {
					esArreglo.push(false);
					exp.replace(i-sub.size(),sub.size()+1,"asin(");
					include_cmath=true;
				} else if (sub=="acos") {
					esArreglo.push(false);
					exp[i]='(';
					include_cmath=true;
				} else if (sub=="cos") {
					esArreglo.push(false);
					exp[i]='(';
					include_cmath=true;
				} else if (sub=="rc") {
					esArreglo.push(false);
					exp.replace(i-sub.size(),sub.size()+1,"sqrtf(");
					i+=3; // diff entre rc y sqrtf
					include_cmath=true;
				} else if (sub=="raiz") {
					esArreglo.push(false);
					exp.replace(i-sub.size(),sub.size()+1,"sqrtf(");
					i+=1; // diff entre rc y sqrtf
					include_cmath=true;
				} else if (sub=="abs") {
					esArreglo.push(false);
					exp[i]='(';
					include_cmath=true;
				} else if (sub=="ln") {
					esArreglo.push(false);
					exp.replace(i-sub.size(),sub.size()+1,"log(");
					i++;
					include_cmath=true;
				} else if (sub=="exp") {
					esArreglo.push(false);
					exp[i]='(';
					include_cmath=true;
				} else if (sub=="azar") {
					esArreglo.push(false);
					exp.replace(i-sub.size(),sub.size()+1,"(rand()%");
					i+=3;
					include_cstdlib=true;
				} else if (sub=="atan") {
					esArreglo.push(false);
					exp[i]='(';
					include_cmath=true;
				} else if (sub=="trunc") {
					esArreglo.push(false);
					exp.replace(i-sub.size(),sub.size()+1,"floor(");
					include_cmath=true;
				} else if (sub=="redon") {
					esArreglo.push(false);
					exp.replace(i-sub.size(),sub.size()+1,"ceil(");
					int parentesis=1,fin=i;
					while(parentesis>0) {
						fin++;
						if (exp[fin]=='[' || exp[fin]=='(') parentesis++;
						else if (exp[fin]==']' || exp[fin]==')') parentesis--;
					}
					if (colocarParentesis(exp,i+1,fin-1))
						exp.insert(fin+1,"+.5");
					else
						exp.insert(fin,"+.5");
					include_cmath=true;
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
	}
	return exp;
}

string expresion(string exp) {
	tipo_var t;
	return expresion(exp,t);
}

//void SetTipo(string exp, char t) {
//	if (exp.find("[",0)!=string::npos) // para que sepa el tipo
//		memoria->a_escribir(exp.substr(0,exp.find("[",0)),t,NULL,"");
//	else
//		memoria->escribir(exp,t,"");
//}
