#include "SynCheck.h"
#include "global.h"
#include <stack>
#include <string>
#include "utils.h"
#include "new_evaluar.h"
#include "intercambio.h"
#include "new_programa.h"
#include "new_funciones.h"
using namespace std;

static int PSeudoFind(const string &s, char x, int from=0, int to=-1) {
	if (to==-1) to=s.size();
	if (x=='\"') x='\'';
	else if (x=='[') x='(';
	else if (x==']') x=')';
	int par=0; bool com=false;
	for (int i=from;i<to;i++) {
		char c=s[i];
		if (c=='\''||c=='\"') { 
			if (x=='\'') return i;
			com=!com;
		} else if (!com) {
			if (c=='('||c=='[') {
				if (par==0 && x=='(') return i;
				par++;
			} else if (c==')'||c==']') {
				if (par==0 && x==')') return i;
				par--;
			} else if (par==0 && c==x) 
				return i; 
		}
	}
	return -1;
}

//static int PSeudoFind(const string &s, string x, int from=0, int to=-1) {
//	if (to==-1) to=x.size();
//	to-=x.size()-1;
//	int p=PSeudoFind(s,x[0],from,to);
//	while (p!=-1) {
//		if (s.substr(p,x.size())==x) return p;
//		p=PSeudoFind(s,x[0],p+1,to);
//	}
//	return -1;
//}



// para checkear que las dimensiones de los arreglos no involucren variables
static bool IsNumericConstant(string &str) {
	for (unsigned int i=0;i<str.size();i++)
		if (EsLetra(str[i])) return false;
	return true;
}

// pasar todo a mayusculas, reemplazar tabs, comillas, word_operators, corchetes, y trimear
static void SynCheckAux1(string &cadena) {
	bool comillas=false;
	int len = cadena.size();
	// primero, todo a mayúsculas y cambio de comillas y paréntesis
	for (int tmp=0;tmp<len;tmp++) {
		char &c=cadena[tmp];
		if (!comillas && tmp>0 && c=='/' && cadena[tmp-1]=='/')
			{ cadena=cadena.substr(0,tmp-1); len=tmp-1; break; }
		if (c=='\"' || c=='\'') { 
			c='\''; comillas=!comillas;
		} else if (!comillas) {
			if (c=='[') c='(';
			else if (c==']') c=')';
			else if (c==9) c=' ';
			// pasar a mayusculas (solo importa la ú por el según, las otras importarán cuando permita variables con ñs y acentos
			else if (c>96 && c<123) c-=32;
			else if (c=='á') c='Á';
			else if (c=='é') c='É';
			else if (c=='í') c='Í';
			else if (c=='ó') c='Ó';
			else if (c=='ú') c='Ú';
			else if (c=='ü') c='Ü';
			else if (c=='ñ') c='Ñ';
		}
	}
	// desues, word_operators
	if (word_operators) {
		comillas=false;
		for (int tmp=0;tmp<len;tmp++) {
			char &c=cadena[tmp];
			if (c=='\'') comillas=!comillas;
			else if (!comillas) {
				if (c=='Y' && (tmp==0 || !parteDePalabra(cadena[tmp-1])) && (tmp==len-1 || !parteDePalabra(cadena[tmp+1])) )
					c='&';
				if (c=='O' && (tmp==0 || !parteDePalabra(cadena[tmp-1])) && (tmp==len-1 || !parteDePalabra(cadena[tmp+1])) )
					c='|';
				if (c=='O' && tmp>0 && cadena[tmp-1]=='N' && (tmp-1==0 || !parteDePalabra(cadena[tmp-2])) && (tmp+1==len || !parteDePalabra(cadena[tmp+1])) )
				{ cadena[tmp-1]=' '; c='~'; }
				if (c=='D' && tmp>1 && cadena[tmp-1]=='O' && cadena[tmp-2]=='M' && (tmp-2==0 || !parteDePalabra(cadena[tmp-3])) && (tmp+1==len || !parteDePalabra(cadena[tmp+1])) )
				{ c='%'; cadena[tmp-1]=' '; cadena[tmp-2]=' '; }
			}
		}
	}
	// Borrar espacios en blanco al principio y al final
	while (cadena.size() && cadena[0]==' ') {cadena.erase(0,1); len--;}
	while (cadena.size() && cadena[cadena.size()-1]==' ') {cadena.erase(cadena.size()-1,1);; len--;}
	// agregar espacios para evitar que cosas como "SI(x<2)ENTONCES" generen un error
//	comillas=false;
//	for(int i=0;i<len;i++) {
//		if (cadena[i]=='\'') comillas=!comillas;
//		else if (!comillas) {
//			if (cadena[i]=='(' && i>0 && EsLetra(cadena[i-1])) { cadena.insert(i," "); len++; }
//			if (cadena[i]==')' && i+2<len && EsLetra(cadena[i+1])) { cadena.insert(i+1," "); len++; }
//		}
//	}
}

// reescribir condiciones coloquiales
static void SynCheckAux2(string &cadena) {
	if (!cadena.size() || !coloquial_conditions) return;
	if (cadena[cadena.size()-1]!=' ') cadena+=" ";
	int comillas=-1;
	for (int y=0;y<int(cadena.size());y++) {
		if (cadena[y]=='\'' || cadena[y]=='\"') comillas=-comillas;
		else if (comillas<0) {
			if (y>0&&(cadena[y]==','||/*cadena[y]=='('||*/cadena[y]==';')&&cadena[y-1]!=' ')
				cadena.insert(y++," ");
		}
	}
	comillas=-1;
	for (int y=0;y<int(cadena.size());y++) {
		if (cadena[y]=='\'' || cadena[y]=='\"') comillas=-comillas;
		else if (comillas<0) {
			string pre, post; int nlen=0;
			if (cadena.substr(y,11)==" ES ENTERO ") {
				cadena.replace(y,11,")=TRUNC(");
				pre="("; post="<PRE>)"; nlen=1;
			} else 
			if (cadena.substr(y,14)==" ES MENOR QUE ") {
				cadena.replace(y,14,"<");
				pre="("; post=")"; nlen=1;
			} else 
			if (cadena.substr(y,14)==" ES MAYOR QUE ") {
				cadena.replace(y,14,">");
				pre="("; post=")"; nlen=1;
			} else 
			if (cadena.substr(y,12)==" ES MENOR A ") {
				cadena.replace(y,12,"<");
				pre="("; post=")"; nlen=1;
			} else 
			if (cadena.substr(y,12)==" ES MAYOR A ") {
				cadena.replace(y,12,">");
				pre="("; post=")"; nlen=1;
			} else 
			if (cadena.substr(y,20)==" ES IGUAL O MAYOR A ") {
				cadena.replace(y,20,">=");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,20)==" ES IGUAL O MENOR A ") {
				cadena.replace(y,20,"<=");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,22)==" ES IGUAL O MAYOR QUE ") {
				cadena.replace(y,22,">=");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,22)==" ES IGUAL O MENOR QUE ") {
				cadena.replace(y,22,"<=");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,20)==" ES MAYOR O IGUAL A ") {
				cadena.replace(y,20,">=");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,20)==" ES MENOR O IGUAL A ") {
				cadena.replace(y,20,"<=");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,12)==" ES IGUAL A ") {
				cadena.replace(y,12,"=");
				pre="("; post=")"; nlen=1;
			} else 
			if (cadena.substr(y,14)==" ES IGUAL QUE ") {
				cadena.replace(y,14,"=");
				pre="("; post=")"; nlen=1;
			} else 
			if (cadena.substr(y,8)==" ES PAR ") {
				cadena.replace(y,8,")%2=0 ");
				pre="("; nlen=6;
			} else 
			if (cadena.substr(y,10)==" ES IMPAR ") {
				cadena.replace(y,10,")%2=1 ");
				pre="("; nlen=6;
			} else 
			if (cadena.substr(y,13)==" ES POSITIVO ") {
				cadena.replace(y,13,")>0 ");
				pre="("; nlen=4;
			} else 
			if (cadena.substr(y,13)==" ES NEGATIVO ") {
				cadena.replace(y,13,")<0 ");
				pre="("; nlen=4;
			} else 
			if (cadena.substr(y,13)==" ES POSITIVA ") {
				cadena.replace(y,13,")>0 ");
				pre="("; nlen=4;
			} else 
			if (cadena.substr(y,13)==" ES NEGATIVA ") {
				cadena.replace(y,13,")<0 ");
				pre="("; nlen=4;
			} else 
			if (cadena.substr(y,9)==" ES CERO ") {
				cadena.replace(y,9,")=0 ");
				pre="("; nlen=4;
			} else 
			if (cadena.substr(y,16)==" ES DISTINTO DE ") {
				cadena.replace(y,16,"<>");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,15)==" ES DISTINTO A ") {
				cadena.replace(y,15,"<>");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,16)==" ES DISTINTA DE ") {
				cadena.replace(y,16,"<>");
				pre="("; post=")"; nlen=2;
			} else 
			if (cadena.substr(y,18)==" ES DIVISIBLE POR ") {
				cadena.replace(y,18,") % (");
				pre="("; post=")=0"; nlen=5;
			} else 
			if (cadena.substr(y,16)==" ES MULTIPLO DE ") {
				cadena.replace(y,16,") % (");
				pre="("; post=")=0"; nlen=5;
			} else 
//			if (cadena.substr(y,7)==" NO ES ") {
//				cadena.replace(y,7," ES ");
//				pre="~("; post=")"; nlen=4;
//			} else 
			if (cadena.substr(y,6)==" ~ ES ") {
				cadena.replace(y,6," ES ");
				pre="~("; post=")"; nlen=4;
			}
			else
			if (cadena.substr(y,4)==" ES ") {
				cadena.replace(y,4,"=");
				pre="("; post=")"; nlen=1;
			}
			if (pre.size()) {
				int par=0, com=-1, yold=y--;
				while ( y>=0 && cadena[y]!='&' && cadena[y]!=',' && cadena[y]!='|' && cadena[y]!='~' && (par>0||cadena[y]!='(')) {
					if (cadena[y]=='\''||cadena[y]=='\"') com=-com;
					else if (com>0) {
						if (cadena[y]==')') par++;
						else if (cadena[y]=='(') par--;
					}
					y--;
				}
				cadena.insert(y+1,pre);
				if (post=="<PRE>") post=cadena.substr(y+1,yold-y+1);
				else if (post=="<PRE>)") post=cadena.substr(y+2,yold-y);
				y=yold;
			}
			if (post.size()) {
				int par=0, com=-1, yold=y,l=cadena.size(); y+=nlen+1;
				while ( y<l && cadena[y]!='&' && cadena[y]!=',' && cadena[y]!='|' && cadena[y]!='~' && (par>0||cadena[y]!=')') && (comillas>0||(cadena.substr(y,2)!="//"&&cadena.substr(y,10)!=" ENTONCES "&&cadena.substr(y,7)!=" HACER ")) ) {
					if (cadena[y]=='\''||cadena[y]=='\"') com=-com;
					else if (com>0) {
						if (cadena[y]==')') par--;
						else if (cadena[y]=='(') par++;
					}
					y++;
				}
				cadena.insert(y,post);
				y=yold;
			}
		}
	}
}

// verificar operadores
static void SynCheckAux3(const int &x, string &cadena, int &errores,const  string &instruccion, int &flag_pyc) {
	char last='.',lastb='.';
	int comillas=-1;
	int parentesis=0;
	int Numero=0; // Para controlar los correctos nombres de variables
	// Borrar espacios en blanco al principio y al final
	while (cadena[0]==' ' && !cadena.size()==0) cadena.erase(0,1);
	while (!cadena.size()==0 && cadena[cadena.size()-1]==' ') cadena.erase(cadena.size()-1,1);
	if (cadena[0]==',') { SynError (3,"Parametro nulo."); errores++; }
	for (int y=0;y<(int)cadena.size();y++) {
		char act=cadena[y];
		// Checkear correctos nombres de variables
		if (comillas<0) {
			if (act==' ' || act==',' || act=='+' || act=='-' || act=='*' || act=='/' || act=='|' || act=='%'
				|| act=='&' || act=='=' || act=='^' || act=='~' || act==')' || act=='(' ||act=='<' || act=='>' ) Numero=0;
			if (Numero==0 && EsLetra(act)) Numero=-1;
			if (Numero==0 && act>='.' && act<='9' && act!='/') Numero=1;
			if (Numero==2 && act=='.')
			{SynError (4,"Constante o Identificador no valido."); errores++; Numero=-1;}
			if (Numero==1 && act=='.') Numero=2;
			if (Numero>0 && (EsLetra(act) || act=='(' || act=='\''))
			{SynError (5,"Constante o Identificador no valido."); errores++; Numero=-1;}
		}
		// Contar comillas
		if (act=='\"') {cadena[y]='\'';act='\'';}
		if (act=='\'') {
			comillas=comillas*(-1);
			if (comillas==1) { // Si se abren comillas
				// if (cadena[y-1]==' ') {cadena.erase(y-1,1);y--;} // Borrar espacio innecesario
				if (last==')' || last=='/' || last=='*' || last=='^' || last=='&' || last=='%' ||
					last=='|' || last=='~' || (last=='.' && y!=0) || (last=='+'&&!allow_concatenation) || (last=='-' && lastb!='<'))
					{SynError (6,"Operador incorrecto."); errores++;}
					if (EsLetra(last))
					{SynError (7,"Se esperaba espacio o coma."); errores++;}
					if (last>='.' && last<='9' && last!=47 && y!=0)
					{SynError (8,"Se esperaba espacio o coma."); errores++;}
					;;
			} else { // Si se cierran comillas
				if ((int)cadena.size()>y+1)
					if (cadena[y+1]=='(' || cadena[y+1]=='/' || cadena[y+1]=='*' || cadena[y+1]=='^' || cadena[y+1]=='&' ||
						cadena[y+1]=='|' || cadena[y+1]=='~' || cadena[y+1]=='.' || cadena[y+1]=='%' ||
						(cadena[y+1]=='+'&&!allow_concatenation) || cadena[y+1]=='-' )
					{SynError (9,"Operador incorrecto."); errores++;}
				if (EsLetra(cadena[y+1]))
				{SynError (10,"Se esperaba espacio o coma."); errores++;}
				if (cadena[y+1]>='.' && cadena[y+1]<='9' && cadena[y+1]!=47)
				{SynError (11,"Se esperaba espacio o coma."); errores++;}
				;;
			}
		} else {
			if (comillas==(-1)) { // si estamos fuera de comillas
				if (act==':' && instruccion!=":") { SynError(999,"Operador no valido (:)."); errores++; }
				// Corroborar caracteres validos
				if (act=='{' || act=='}') 
				{SynError (68,"Caracter no valido."); errores++;}
				else if (!EsLetra(act) && act!=' ' && act!='^'
					&& act!='+' && act!='(' && act!='\'' && act!='~' && act!='-' && act!=')' 
					&& act!='|' && act!='&' && act!='*' && act!='=' && act!='<' && act!='>'
					&& act!='/' && act!='0' && act!='1' && act!='2' && act!='3' && act!='4'
					&& act!='5' && act!='6' && act!='7' && act!='8' && act!='9' && act!='.'
					&& act!=',' && act!='[' && act!=']' && act!=':' && act!=';' && act!='_' && act!='%') 
				{SynError (12,string("Caracter no valido (")+string(1,act)+")."); errores++;}
				// Separar lineas compuestas por más de una instrucción
				if (act==';' && y!=(int)cadena.size()-1) {
					string str=cadena;
					cadena.erase(y+1,cadena.size()-y-1);
					str.erase(0,y+1);
					programa.Insert(x+1,str);
					flag_pyc+=1;
				}
				// Contar parentesis
				if (act=='(') 
					parentesis+=1;
				if (act==')') 
					parentesis-=1;
				// Borrar espacios innecesarios
				/// @todo: ver porque no queria sacarle los espacios a & y |
				if (last=='\'' && act==32 && !EsLetra(cadena[y+1]) /*&& cadena[y+1]!='&' &&  cadena[y+1]!='|'*/) {cadena.erase(y,1);y--;}
				if (last==32 && act==32) {cadena.erase(y,1);y--;} // <<<-
				if (last=='&' && act==32) {cadena.erase(y,1);y--;} // <<<-
				if (last==32 && act=='&') {cadena.erase(y-1,1);y--;} // <<<-
				if (last=='|' && act==32) {cadena.erase(y,1);y--;} // <<<-
				if (last==32 && act=='|') {cadena.erase(y-1,1);y--;}
				if (last=='(' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act=='(') {cadena.erase(y-1,1);y--;}
				//if (last==')' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act==')') {cadena.erase(y-1,1);y--;}
				if (last==32 && act=='+') {cadena.erase(y-1,1);y--;}
				if (last==32 && act=='-') {cadena.erase(y-1,1);y--;}
				if (last=='+' && act==32) {cadena.erase(y,1);y--;}
				if (last=='-' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act=='/') {cadena.erase(y-1,1);y--;}
				if (last=='/' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act=='*') {cadena.erase(y-1,1);y--;}
				if (last=='*' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act=='%') {cadena.erase(y-1,1);y--;}
				if (last=='%' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act=='^') {cadena.erase(y-1,1);y--;}
				if (last=='^' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act==',') {cadena.erase(y-1,1);y--;}
				if (last==',' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act=='<') {cadena.erase(y-1,1);y--;}
				if (last==32 && act=='>') {cadena.erase(y-1,1);y--;}
				if (last=='>' && act==32) {cadena.erase(y,1);y--;}
				if (last=='<' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act=='=') {cadena.erase(y-1,1);y--;}
				if (last=='=' && act==32) {cadena.erase(y,1);y--;}
				if (last=='~' && act==32) {cadena.erase(y,1);y--;}
				if (last==32 && act==';') {cadena.erase(y-1,1);y--;}
				if (last==32 && act==':') {cadena.erase(y-1,1);y--;}
				// Buscar operadores incorrectos
				if (EsLetra(act) && last=='.' && y!=0) {SynError (13,"Operador incorrecto."); errores++;}
				if (EsLetra(last) && act=='.') {SynError (14,"Operador incorrecto."); errores++;}
				if (last=='.' && y!=0)
					if (act=='<' || act=='>' || act=='(' || act==')' || 
						act=='/' || act=='*' || act=='^' || act=='&' || 
						act=='|' || act=='~' || act=='.' || act==',' ||
						act=='%' || act=='+' || act=='-' || act=='=') 
							{SynError (15,"Operador incorrecto."); errores++;}
				if (last==',')
					if (act=='<' || act=='>' || act==')' || act==',' || act==';' ||
						act=='/' || act=='*' || act=='^' || act=='&' || act=='%' ||
						act=='|' || act=='=') 
							{SynError (16,"Parametro nulo."); errores++;}
				if (last=='>')
					if (act=='<' || act=='>' || act=='/' || act=='*' || act=='^' || act=='&' || act==',' || act==';' || act=='%' ||
						act=='|' || act=='~' || act==')') 
							{SynError (17,"Operador incorrecto."); errores++;}
				if (last=='<')
					if (act=='<' || act=='/' || act=='*' || act=='^' || act=='&' || act==',' || act==';' || act=='%' ||
						act=='|' || act=='~' || act==')')
							{SynError (18,"Operador incorrecto."); errores++;}
				if (last=='=')
					if (act=='<' || act=='>' || act=='/' || act=='*' || act=='^' || act=='&' || act==',' || act==';' || act=='%' ||
						act=='|' || act=='~' || act=='=' || act==')') 
							{SynError (19,"Operador incorrecto."); errores++;}
				if (last=='|')
					if (act=='<' || act=='>' || act=='-' || act=='+' || act=='%' ||
						act=='/' || act=='*' || act=='^' || act=='&' ||
						act=='|' || act=='=' || act==',' || act==';' || act==')') 
							{SynError (20,"Operador incorrecto."); errores++;}
				if (last=='&')
					if (act=='<' || act=='>' || act=='-' || act=='+' || act=='%' ||
						act=='/' || act=='*' || act=='^' || act=='&' ||
						act=='|' || act=='=' || act==',' || act==';' || act==')')
							{SynError (21,"Operador incorrecto."); errores++;}
				if (last=='~')
					if (act=='<' || act=='>' || act=='/' || act=='*' || act=='^' || act=='&' || act==',' || act==';' || act=='%' ||
						act=='|' || act=='~' || act=='=' || act==')') 
							{SynError (22,"Operador incorrecto."); errores++;}
				if (last=='+')
					if (act=='<' || act=='>' || act=='-' || act=='+' || act==')' ||
						act=='/' || act=='*' || act=='^' || act=='&' || act==',' || act==';' ||
						act=='|' || act=='~' || act=='=' ) 
							{SynError (23,"Operador incorrecto."); errores++;}
				if (last=='-')
					if (act=='<' || act=='>' || act=='-' || act=='+' || act==')' ||
						act=='/' || act=='*' || act=='^' || act=='&' || act==',' || act==';' ||
						act=='|' || act=='~' || act=='=' ) 
							{SynError (24,"Operador incorrecto."); errores++;}
				if (last=='*')
					if (act=='<' || act=='>' || act=='+' || act=='%' ||
						act=='/' || act=='*' || act=='^' || act=='&' || act==',' || act==';' ||
						act=='|' || act=='~' || act=='=' || act==')') 
							{SynError (25,"Operador incorrecto."); errores++;}
				if (last=='/')
					if (act=='<' || act=='>' || act=='+' || act=='%' ||
						act=='*' || act=='^' || act=='&' || act==',' || act==';' ||
						act=='|' || act=='~' || act=='=' || act==')') 
							{SynError (26,"Operador incorrecto."); errores++;}
				if (last=='^')
					if (act=='<' || act=='>' || act=='+' ||
						act=='/' || act=='*' || act=='^' || act=='&' || act=='%' ||
						act=='|' || act=='~' || act=='=' || act==')') 
							{SynError (27,"Operador incorrecto."); errores++;}
				if (last=='(')
					if (act=='<' || act=='>' || act=='/' || act=='*' || act=='^' || act=='&' || act=='%' ||
						act=='|' || act=='=') 
							{SynError (28,"Operador incorrecto."); errores++;}
				if (last=='(')
					if (act==',') 
						{SynError (999,"Parametro nulo."); errores++;}
				if (last==')') {
					if (act=='(') 
						{SynError (29,"Operador incorrecto."); errores++;}
				}
				if (last=='%') {
					if (act=='<' || act=='>' || act==')' || act=='%' ||
						act=='/' || act=='*' || act=='^' || act=='&' || act==',' || act==';' ||
						act=='|' || act=='~' || act=='=' ) {SynError (30,"Operador incorrecto."); errores++;}								
				}
				if (last=='(' && act==')')  {SynError (152,"Parametro nulo."); errores++;}
				// Borrar comentarios
				if (last=='/' && act=='/')
					cadena.erase(y-1,cadena.size()-y+1);
			}
		}
		if (y!=0) lastb=cadena[y-1];
		last=cadena[y];
	}
	// Posibles errores encontrados
	if (parentesis<0) { SynError (35,"Se cerraron parentesis o corchetes demás."); errores++; }
	if (parentesis>0) { 
		SynError (36,"Falta cerrar parentesis o corchete."); errores++;
	}
	if (comillas==1) { SynError (37,"Falta cerrar comillas."); errores++; }
}

static bool RightCompareFix(string &s, string e) {
	int le=e.size(); int ls=s.size();
	bool fix=e[0]==' ';
	if (fix) { le--; e=e.substr(1); }
	if (ls<le) return false;
	else if (le==ls) return s==e;
	else if (s.substr(ls-le,le)!=e) return false;
	else if (s[ls-le-1]==')') { s.insert(ls-le," "); ls++; };
	return s[ls-le-1]==' ';
}

static bool LeftCompareFix(string &s, string e) {
	int ls=s.size(), le=e.size();
	bool fix=e[le-1]==' ';
	if (fix) { le--; e=e.substr(0,le); }
	if (ls<le) return false;
	else if (ls==le) return s==e;
	else if (s.substr(0,le)!=e) return false;
	else if (s[le]=='(') s.insert(le," ");
	return s[le]==' ';
}

void InformUnclosedLoops(stack<Instruccion> &bucles, int &errores) {
	// Controlar Cierre de Bucles
	while (!bucles.empty())	{
		if (bucles.top()=="PARA") {SynError (114,"Falta cerrar PARA.",bucles.top().num_linea,bucles.top().num_instruccion); errores++;}
		else if (bucles.top()=="REPETIR") {SynError (115,"Falta cerrar REPETIR.",bucles.top().num_linea,bucles.top().num_instruccion); errores++;}
		else if (bucles.top()=="MIENTRAS") {SynError (116,"Falta cerrar MIENTRAS.",bucles.top().num_linea,bucles.top().num_instruccion); errores++;}
		else if (bucles.top()=="SI") {SynError (117,"Falta cerrar SI.",bucles.top().num_linea,bucles.top().num_instruccion); errores++;}
		else if (bucles.top()=="SEGUN") {SynError (118,"Falta cerrar SEGUN.",bucles.top().num_linea,bucles.top().num_instruccion); errores++;}
		else if (bucles.top()=="PROCESO"/* && Proceso<2*/) {SynError (119,"Falta cerrar PROCESO.",bucles.top().num_linea,bucles.top().num_instruccion); errores++;}
		else if (bucles.top()=="SUBPROCESO"/* && Proceso<2*/) {SynError (119,"Falta cerrar SUBPROCESO.",bucles.top().num_linea,bucles.top().num_instruccion); errores++;}
		bucles.pop();
	}
}

//-------------------------------------------------------------------------------------------
// ********************* Checkear la Correcta Sintaxis del Archivo **************************
//-------------------------------------------------------------------------------------------
int SynCheck() {
	queue<string> arguments; // para guardar los nombres de argumentos de funciones 
	int untitled_functions_count=0; // para numerar las funciones sin nombre
	SynErrores=0;
	programa.PushBack("");
	programa.Insert(0,"");
	stack <Instruccion> bucles; // Para controlar los bucles que se abren y cierran
	bucles.push(Instruccion("CHECK",1,1));
	int errores=0, Lerrores; // Total de errores , y cant hasta la instruccion anterior
	int flag_pyc=0, tmp;
	bool in_process=false, process_seen=false;
	tipo_var tipo;
	string cadena, instruccion, str;
	
	// Checkear sintaxis y reorganizar el codigo
	for (int x=1;x<programa.GetSize();x++){
		Inter.SetLineAndInstructionNumber(x);
		cadena=programa[x];
		Lerrores=errores;
		
		// Corregir formato de fin de linea
		if (cadena[cadena.size()-1]==13) cadena[cadena.size()-1]=' ';
		if (cadena[cadena.size()-1]==10) cadena[cadena.size()-1]=' ';
		if (cadena[cadena.size()-2]==13) cadena[cadena.size()-2]=' ';
		if (cadena[cadena.size()-2]==10) cadena[cadena.size()-2]=' ';
		
		
		// Ignorar lineas de comentarios
		{
			int comillas=-1, parentesis=0;
			instruccion="";
			// Pasar todo a mayusculas, cambiar comillas y corchetes
			comillas=-1;

			// pasar todo a mayusculas, reemplazar tabs, comillas, word_operators, corchetes, y trimear
			SynCheckAux1(cadena);
			
			int len = cadena.size();
			if (lazy_syntax && LeftCompare(cadena,"FIN ")) { cadena="FIN"+cadena.substr(4); len--; }
			if (LeftCompare(cadena,"BORRAR ")) { cadena="BORRAR"+cadena.substr(7); len--; }
			// si hay un ; pegado a finalgo puede traer problema
			comillas=-1;
			for (tmp=0;tmp<len;tmp++) {
				if (cadena[tmp]=='\'') comillas=-comillas;
				else if (comillas<0 && cadena[tmp]==';' && tmp && cadena[tmp-1]!=' ')
					cadena.insert(tmp," ");
			}
			
			
			// Separar si es sino o entonces
			if (cadena=="ENTONCES" && bucles.top()!="SI" )
				{SynError (51,"ENTONCES mal colocado."); errores++;}
			if (cadena=="SINO" && bucles.top()!="SI" )
				{SynError (55,"SINO mal colocado."); errores++;}
			if (LeftCompare(cadena,"ENTONCES ")) {
				if (bucles.top()!="SI" && programa[x-1]!="SI")
					{SynError (1,"ENTONCES mal colocado."); errores++;}
				str=cadena;
				str.erase(0,9);
				cadena="ENTONCES";
				programa.Insert(x+1,str);
				flag_pyc+=1;
			}
			if (LeftCompare(cadena,"SINO ")) {
				if (bucles.top()!="SI")
					{SynError (2,"SINO mal colocado."); errores++;}
				str=cadena;
				cadena="SINO";
				str.erase(0,5);
				programa.Insert(x+1,str);
				flag_pyc+=1;
			}
			if (cadena=="FINSEGÚN") cadena="FINSEGUN";
			
			// Cortar la instrucción
			cadena=cadena+" ";
			if (LeftCompare(cadena,"ESCRIBIR ") || (lazy_syntax && (LeftCompare(cadena,"IMPRIMIR ") || LeftCompare(cadena,"MOSTRAR ") || LeftCompare(cadena,"INFORMAR "))) ) {
				instruccion="ESCRIBIR "; cadena.erase(0,cadena.find(" ")+1);
				if (ReplaceIfFound(cadena,"SIN SALTAR","",true)||ReplaceIfFound(cadena,"SIN BAJAR","",true)||ReplaceIfFound(cadena,"SINBAJAR","",true)||ReplaceIfFound(cadena,"SINSALTAR","",true))
					instruccion="ESCRIBNL ";
			} else if (LeftCompare(cadena,"LEER ")) {
				instruccion="LEER "; cadena.erase(0,5);
			} else if (LeftCompareFix(cadena,"SI ") && !LeftCompare(cadena,"SI ES ")) {
				instruccion="SI "; cadena.erase(0,3);
				bucles.push(programa.GetLoc(x,"SI"));
				// cortar el entonces si esta en la misma linea
				comillas=-1;
				if (RightCompareFix(cadena," ENTONCES ")) {
					cadena.erase(cadena.size()-10,10);
					programa.Insert(x+1,"ENTONCES");
					flag_pyc+=1;
				} else
					for (int y=0; y<(int)cadena.size()-10;y++) {
						if(cadena[y]=='\"' || cadena[y]=='\'') comillas=-comillas;
						if (comillas<1 && MidCompareNC(" ENTONCES ",cadena,y)) {
							str=cadena;
							cadena.erase(y+1,cadena.size()-y);
							// borrar los espacios en medio
							while (cadena[cadena.size()-1]==' ' && cadena.size()!=0) cadena.erase(cadena.size()-1,1);
							str.erase(0,y+1);
							programa.Insert(x+1,str);
							flag_pyc+=1;
							break;
						}
					}
			} else if (LeftCompareFix(cadena,"MIENTRAS ")&&(!lazy_syntax||!LeftCompareFix(cadena,"MIENTRAS QUE "))) { 
				instruccion="MIENTRAS "; cadena.erase(0,9);
				bucles.push(programa.GetLoc(x,"MIENTRAS"));
			} else if (LeftCompareFix(cadena,"SEGUN ")) {
				instruccion="SEGUN "; cadena.erase(0,6);
				bucles.push(programa.GetLoc(x,"SEGUN"));
			} else if (LeftCompare(cadena,"DE OTRO MODO: ") || LeftCompare(cadena,"DE OTRO MODO ")) {
				cadena.erase(0,13); programa.Insert(x+1,cadena); flag_pyc+=1;
				instruccion="DE OTRO MODO: "; cadena="";
			} else if (LeftCompare(cadena,"DIMENSION ")) {
				instruccion="DIMENSION "; cadena.erase(0,10);
			} else if (LeftCompareFix(cadena,"HASTA QUE ")) {
				instruccion="HASTA QUE "; cadena.erase(0,10);
			} else if (lazy_syntax&&LeftCompareFix(cadena,"MIENTRAS QUE ")) {
				instruccion="MIENTRAS QUE "; cadena.erase(0,13);
			} else if (LeftCompare(cadena,"FINSI ")) {
				instruccion="FINSI "; cadena.erase(0,6);
			} else if (LeftCompare(cadena,"FINPARA ")) {
				instruccion="FINPARA "; cadena.erase(0,8);
			} else if (LeftCompare(cadena,"FINMIENTRAS ")) {
				instruccion="FINMIENTRAS "; cadena.erase(0,12);
			} else if (LeftCompare(cadena,"FINSEGUN ")) {
				instruccion="FINSEGUN "; cadena.erase(0,9);
			} else if (LeftCompare(cadena,"ESPERARTECLA ")) {
				instruccion="ESPERARTECLA"; cadena.erase(0,13);
			} else if (LeftCompare(cadena,"ESPERAR UNA TECLA ")) {
				instruccion="ESPERARTECLA"; cadena.erase(0,18);
			} else if (LeftCompare(cadena,"ESPERAR TECLA ")) {
				instruccion="ESPERARTECLA"; cadena.erase(0,14);
			} else if (LeftCompare(cadena,"ESPERAR ")) {
				instruccion="ESPERAR "; cadena.erase(0,8);
			} else if (LeftCompare(cadena,"LIMPIARPANTALLA ")) {
				instruccion="BORRARPANTALLA"; cadena.erase(0,16);
			} else if (LeftCompare(cadena,"BORRAR PANTALLA ")) {
				instruccion="BORRARPANTALLA"; cadena.erase(0,16);
			} else if (LeftCompare(cadena,"LIMPIAR PANTALLA ")) {
				instruccion="BORRARPANTALLA"; cadena.erase(0,17);
			} else if (LeftCompare(cadena,"BORRARPANTALLA ")) {
				instruccion="BORRARPANTALLA"; cadena.erase(0,15);
			} else if (LeftCompare(cadena,"PROCESO ")) {
				instruccion="PROCESO "; cadena.erase(0,8);
//				if (!in_process) bucles.push(programa.GetLoc(x,"PROCESO"));
			} else if (enable_user_functions && LeftCompare(cadena,"SUBPROCESO ")) {
				instruccion="SUBPROCESO "; cadena.erase(0,11);
//				if (!in_process) bucles.push(programa.GetLoc(x,"SUBPROCESO"));
			} else if (LeftCompare(cadena,"ENTONCES ")) {
				instruccion="ENTONCES "; cadena.erase(0,9);
			} else if (LeftCompare(cadena,"SINO ")) {
				instruccion="SINO "; cadena.erase(0,5);
			} else if (lazy_syntax && LeftCompare(cadena,"PARACADA ")) {
				instruccion="PARACADA "; cadena.erase(0,9);
				bucles.push(programa.GetLoc(x,"PARACADA"));
			} else if (lazy_syntax && LeftCompare(cadena,"PARA CADA ")) {
				instruccion="PARACADA "; cadena.erase(0,10);
				bucles.push(programa.GetLoc(x,"PARACADA"));
			} else if (LeftCompare(cadena,"PARA ")) {
				instruccion="PARA "; cadena.erase(0,5);
				bucles.push(programa.GetLoc(x,"PARA"));
				// si se puede asignar con igual, reemplazar aca
				if (overload_equal) {
					int i=0, l=cadena.size();
					while (i<l && EsLetra(cadena[i])) i++;
					if (i<l&& cadena[i]=='=') cadena.replace(i,1,"<-");
				}
				// evitar problema de operador incorrecto al poner el signo al numero
				comillas=-1;
				// si dice "i desde 1" en lugar de "i<-1" se reemplaza " desde " por "<-"
				if (lazy_syntax && cadena.find("<-",0)==string::npos && cadena.find(" DESDE ")!=string::npos) 
					cadena.replace(cadena.find(" DESDE "),7,"<-");
				if (cadena.find("<-",0)!=string::npos) {
					// se agregan parentesis al valor inicial para evitar problemas mas adelante (porque si el valor es negativo, con la flecha de asignacion queda un --)
					for (int y=cadena.find("<-",0)+3; y<(int)cadena.size();y++) {
						if(cadena[y]=='\"' || cadena[y]=='\'') comillas=-comillas;
						if (comillas<1 && (MidCompareNC(" HASTA ",cadena,y) || MidCompareNC(" CON PASO ",cadena,y))) {
							cadena.insert(y,")");
							cadena.insert(cadena.find("<-",0)+2,"(");
							break;
						}
					}
				}
			} else if (LeftCompareFix(cadena,"SEGÚN ")) {
				instruccion="SEGUN "; cadena.erase(0,6);
				bucles.push(programa.GetLoc(x,"SEGUN"));
			} else if (LeftCompare(cadena,"FINSEGÚN ")) {
				instruccion="FINSEGUN "; cadena.erase(0,9);
			} else if (LeftCompare(cadena,"FINPROCESO ")) {
				instruccion="FINPROCESO "; cadena.erase(0,11);
			} else if (enable_user_functions && LeftCompare(cadena,"FINSUBPROCESO ")) {
				instruccion="FINSUBPROCESO "; cadena.erase(0,14);
			} else if (LeftCompare(cadena,"REPETIR ")) {
				instruccion="REPETIR "; cadena.erase(0,8);
			} else if (lazy_syntax && LeftCompare(cadena,"HACER ")) {
				instruccion="REPETIR "; cadena.erase(0,6);
			} else if (LeftCompare(cadena,"DEFINIR ")) {
				instruccion="DEFINIR "; cadena.erase(0,8);
			} else {
				int flag_segun=0;
				if (!bucles.empty()) {
					if (bucles.top()=="SEGUN") { // si esta en segun comprobar la condicion
						int pos_dp=PSeudoFind(cadena,':');
						if (pos_dp!=-1 && cadena[pos_dp+1]!='=') {
							// ver ademas si dise "OPCION o CASO al principio"
							if (lazy_syntax) {
								if (cadena.size()>6 && cadena.substr(0,5)=="CASO ") {
									cadena=cadena.substr(5); pos_dp-=5;
								} else if (cadena.size()>6 && cadena.substr(0,5)=="SIES ") {
									cadena=cadena.substr(5); pos_dp-=5;
								} else if (cadena.size()>7 && cadena.substr(0,6)=="SI ES ") {
									cadena=cadena.substr(6); pos_dp-=6;
								} else if (cadena.size()>8 && cadena.substr(0,7)=="OPCION ") {
									cadena=cadena.substr(7); pos_dp-=7;
								} else if (cadena.size()>8 && cadena.substr(0,7)=="OPCIÓN ") {
									cadena=cadena.substr(7); pos_dp-=7;
								}
							}
							instruccion=":";
							programa.Insert(x+1,cadena);
							programa[x+1].instruccion.erase(0,pos_dp+1);
							cadena.erase(pos_dp+1,cadena.size()-pos_dp-1);
							flag_pyc+=1; flag_segun=1;
						}
					}
				}
				if (flag_segun==0) {
					instruccion="Error?";
					// Ver si es asignacion
					int i=0, l=cadena.size();
					while (i<l && (cadena[i]=='\t'||cadena[i]==' ')) i++;
					int par=0;
					while (i<l && (par||
//						(cadena[i]>='a'&&cadena[i]<='z') || // no deberia hacer falta a esta altura
						EsLetra(cadena[i]) ||
						(cadena[i]>='0'&&cadena[i]<='9') ||
						cadena[i]=='_'||cadena[i]=='(')) {
							if (cadena[i]=='(') par++;
							else if (cadena[i]==')') par--;
							i++;
						}
					while (i<l && (cadena[i]=='\t'||cadena[i]==' ')) i++;
					if (i>0&&i<l) {
						if (i+1<l && cadena[i]==':' && cadena[i+1]=='=') {cadena[i]='<';cadena[i+1]='-';}
						else if (overload_equal && cadena[i]=='=') cadena.replace(i,1,"<-");
						if (i+1<l&&cadena[i]=='<'&&cadena[i+1]=='-') {
							comillas=-1;
							for (int y=0; y<(int)cadena.find("<-",0);y++)
								if(cadena[y]=='\"' || cadena[y]=='\'') comillas=-comillas;
							if (comillas<1) {
								instruccion="<-";
								// evitar problema de operador incorrecto al poner el signo al numero
								cadena.insert(i+2,"(");
								cadena.insert(cadena.size(),") ");
								for (int y=i+3; y<(int)cadena.size();y++) {
									if(cadena[y]=='\"' || cadena[y]=='\'') comillas=-comillas;
									if (comillas<1 && cadena[y]==';') {
										cadena.insert(y,")");
										cadena.erase(cadena.size()-2,2);
										break;
									}
								}
							} 
						}
					}
					if (instruccion!="<-") {
						int p=0, l=cadena.length();
						while (p<l&&((cadena[p]>='A'&&cadena[p]<='Z')||cadena[p]=='_'||(cadena[p]>='0'&&cadena[p]<='9'))) p++;
						funcion *func=EsFuncion(cadena.substr(0,p));
						if (func) instruccion=string("INVOCAR ");
					}
				}
			}
			
			// reescribir condiciones coloquiales
			SynCheckAux2(cadena);

			// verificar operadores
			SynCheckAux3(x,cadena,errores,instruccion,flag_pyc);
			
			// y si hay algo a continuacion del hacer tambien, asi que despues del hacer se corta como si fuera hacer; para que se pueda escribir por ejemplo un mientras en una sola linea
			comillas=-1; len=cadena.size();
			for (tmp=0;tmp<len;tmp++) {
				if (cadena[tmp]=='\'') comillas=-comillas;
				else if (comillas<0 && tmp+5<len && cadena.substr(tmp,5)=="HACER" && cadena.substr(tmp,6)!="HACER;") {
					programa.Insert(x+1,cadena.substr(tmp+5));
					cadena.erase(tmp+5); break;
				}
			}
			
			if (cadena.size()&&cadena[cadena.size()-1]==',')
			{ SynError (31,"Parametro nulo."); errores++; }
			while (cadena[0]==';' && cadena.size()>1) cadena.erase(0,1); // para que caso esta esto?
			// Controlar que el si siempre tenga un entonces
			if (x&&LeftCompare(programa[x-1],"SI "))
				if (instruccion!="ENTONCES " && cadena!="") {
					if (lazy_syntax) {
						programa.Insert(x,"ENTONCES "); x++;
					} else 
						{SynError (32,"Se esperaba ENTONCES"); errores++;}
				}
			// si entro en segun comprobar que haya condicion
			if (!bucles.empty()) {
				if (bucles.top()=="SEGUN" && LeftCompare(programa[x-1],"SEGUN") && cadena!="") {
					if (instruccion!=":") SynError (33,"Se esperaba <opcion>:."); errores++;
				}
			}
			
			// Controlar el punto y coma
			bool lleva_pyc=instruccion=="DIMENSION " || instruccion=="DEFINIR " || instruccion=="ESCRIBIR "|| instruccion=="ESCRIBNL " || instruccion=="<-"  || instruccion=="LEER " || instruccion=="ESPERAR " || instruccion=="ESPERARTECLA" || instruccion=="BORRARPANTALLA";
			if (lleva_pyc) {
				if (cadena[cadena.size()-1]!=';') {
					if (force_dot_and_comma)
					{ SynError (38,"Falta punto y coma."); errores++; }
					cadena=cadena+';';
				}
//			} else {
//				if (instruccion!="Error?" && cadena[cadena.size()-1]==';') {
//					SynError (39,"No lleva punto y coma."); errores++;
//					cadena.erase(cadena.size()-1,1);
//				}
			}
			// Pegar la instrucción
			if (instruccion!="." && instruccion!="Error?" && instruccion!="<-" && instruccion!=":")
				cadena=instruccion+cadena;
			if (cadena.size()&&cadena[cadena.size()-1]==' ') cadena.erase(cadena.size()-1,1); // Borrar espacio en blanco al final
			// Cortar instrucciones despues de sino o entonces
			while (cadena.size()&&cadena[cadena.size()-1]==' ') // Borrar espacios en blanco al final
				cadena.erase(cadena.size()-1,1);
			// arreglar problemas con valores negativos en para y mientras
			ReplaceIfFound(cadena," HASTA-"," HASTA -");
			ReplaceIfFound(cadena," PASO-"," PASO -");
			ReplaceIfFound(cadena," QUE-"," QUE -");
			// Comprobar parametros
			if (instruccion=="SUBPROCESO "|| instruccion=="PROCESO ") {
				funcion the_func(x+1);
				int p=0; NextToken(cadena,p);
				if (in_process) InformUnclosedLoops(bucles,errores);
				bool sub=instruccion[0]=='S'; in_process=true;
				bucles.push(programa.GetLoc(x,sub?"SUBPROCESO":"PROCESO"));
				// parsear nombre y valor de retorno
				string fname=NextToken(cadena,p); string tok=NextToken(cadena,p); // extraer el nombre y el "=" si esta
				if (tok=="="||tok=="<-") { // si estaba el igual, lo que se extrajo es el valor de retorno
					if (!sub) { SynError (999,"El proceso principal no puede retornar ningun valor."); errores++; }
					the_func.nombres[0]=fname; fname=NextToken(cadena,p); tok=NextToken(cadena,p); 
					if (!CheckVariable(the_func.nombres[0])) errores++;
				} //...en tok2 deberia quedar siempre el parentesis si hay argumentos, o en nada si termina sin argumentos
				if (fname=="") { 
					SynError (40,sub?"Falta nombre de subproceso.":"Falta nombre de proceso."); errores++; 
					fname=string("<sin_nombre>")+IntToStr(++untitled_functions_count);
				}
				else if (!CheckVariable(fname)) errores++;
				if (!sub) { // si es el proceso principal, verificar que sea el unico, y guardar el nombre en main_process_name para despues saber a cual llamar
					if (process_seen) { SynError (999,"Solo puede haber un proceso."); errores++; }
					else process_seen=true;
					main_process_name=fname;
				}
				// argumentos
				if (tok=="(") {
					if (!sub) { SynError (999,"El proceso principal no puede recibir argumentos."); errores++; }
					bool closed=false;
					tok=NextToken(cadena,p);
					while (tok!="") {
						if (tok==")") { closed=true; break; }
						else if (tok==",") { 
							SynError (999,"Falta nombre de argumento."); errores++;
							tok=NextToken(cadena,p);
						} else {
							if (!CheckVariable(tok)) errores++;
							the_func.AddArg(tok);
							tok=NextToken(cadena,p);
							if (tok!="," && tok!=")" && tok!="") { SynError (999,"Se esperaba coma(,) o parentesis ())."); errores++; }
							else if (tok==",") tok=NextToken(cadena,p);
						}
					}
					if (!closed) {
						{ SynError (999,"Falta cerrar lista de argumentos."); errores++; }
					} else if (NextToken(cadena,p).size()) {
						{ SynError (999,"Se esperaba fin de linea."); errores++; }
					}
				} else if (tok!="") { // si no habia argumentos no tiene que haber nada
					if (!sub) { SynError (999,"Se esperaba el fin de linea."); errores++; } else {
						if (the_func.nombres[0].size()) { SynError (999,"Se esperaba la lista de argumentos, o el fin de linea."); errores++; }
						else { SynError (999,"Se esperaba la lista de argumentos, el signo de asignación, o el fin de linea."); errores++; }
					}
				}
				subprocesos[fname]=the_func;

				
				
//				else if (enable_user_functions && cadena=="SUBPROCESO") {SynError (999,"Falta nombre de subproceso."); errores++; Proceso=1;}
//				else if (LeftCompare(cadena,"PROCESO ") || (enable_user_functions && LeftCompare(cadena,"SUBPROCESO "))) {
//					
//					if (!CheckVariable(cadena.substr(8,cadena.size()-8),41)) errores++;
//					scope=sub?SC_FUNC:SC_MAIN; // Indica el comienzo
//				} else if (enable_user_functions && LeftCompare(cadena,"SUBPROCESO ")) {
////					if (!CheckVariable(cadena.substr(8,cadena.size()-8),41)) errores++;
////					Proceso=1; // Indica el comienzo
//				} else if (cadena!="") {
//					SynError (42,"Se esperaba PROCESO <nombre>."); errores++; Proceso=1;
//					bucles.push(programa.GetLoc(x,"PROCESO"));
//				}
			}
			if (!in_process && cadena!="") {SynError (43,enable_user_functions?"Instruccion fuera de proceso/subproceso.":"Instruccion fuera de proceso."); errores++;}
			if (cadena=="FINPROCESO" || cadena=="FINSUBPROCESO") {
				bool sub=cadena!="FINPROCESO";
				if (!bucles.empty() && ( (!sub&&bucles.top()=="PROCESO")||(sub&&bucles.top()=="SUBPROCESO") ) ) {
					bucles.pop();
				} else {
					SynError (108,sub?"FINSUBPROCESO mal colocado.":"FINPROCESO mal colocado."); errores++;
				}
			}
			// Controlar correcta y completa sintaxis de cada instruccion
			if (instruccion=="DEFINIR "){  // ------------ DEFINIR -----------//
				if (cadena=="DEFINIR" || cadena=="DEFINIR ;")
				{SynError (44,"Faltan parametros."); errores++;}
				else {
					if (cadena[cadena.size()-1]!=';') {
						cadena=cadena+";";
						SynError (45,"Falta punto y coma."); errores++;
					}
					
					if (RightCompare(cadena," COMO ENTEROS;")||RightCompare(cadena," COMO ENTERAS;")) cadena.replace(cadena.size()-8,8,"ENTERO;");
					if (RightCompare(cadena," COMO ENTERA;")) cadena.replace(cadena.size()-7,7,"ENTERO;");
					if (RightCompare(cadena," COMO REALES;")) cadena.replace(cadena.size()-7,7,"REAL;");
					if (RightCompare(cadena," COMO NUMEROS;")) cadena.replace(cadena.size()-8,8,"REAL;");
					if (RightCompare(cadena," COMO NUMERO;")) cadena.replace(cadena.size()-7,7,"REAL;");
					if (RightCompare(cadena," COMO NUMERICA;")||RightCompare(cadena," COMO NUMERICO;")) cadena.replace(cadena.size()-9,9,"REAL;");
					if (RightCompare(cadena," COMO NUMERICAS;")||RightCompare(cadena," COMO NUMERICOS;")) cadena.replace(cadena.size()-10,10,"REAL;");
					if (RightCompare(cadena," COMO TEXTO;")) cadena.replace(cadena.size()-6,6,"CARACTER;");
					if (RightCompare(cadena," COMO CARACTERES;")) cadena.replace(cadena.size()-11,11,"CARACTER;");
					if (RightCompare(cadena," COMO TEXTOS;")) cadena.replace(cadena.size()-7,7,"CARACTER;");
					if (RightCompare(cadena," COMO CADENA;")) cadena.replace(cadena.size()-7,7,"CARACTER;");
					if (RightCompare(cadena," COMO CADENAS;")) cadena.replace(cadena.size()-8,8,"CARACTER;");
					if (RightCompare(cadena," COMO LOGICOS;")||RightCompare(cadena," COMO LOGICAS;")) cadena.replace(cadena.size()-8,8,"LOGICO;");
					if (RightCompare(cadena," COMO LOGICA;")) cadena.replace(cadena.size()-7,7,"LOGICO;");
					
					if (!RightCompare(cadena," COMO REAL;") && !RightCompare(cadena," COMO ENTERO;") && !RightCompare(cadena," COMO CARACTER;") && !RightCompare(cadena," COMO LOGICO;") ) {
						{SynError (46,"Falta tipo de dato o tipo no valido."); errores++;}
					} else {
						int largotipo=0; tipo_var tipo_def=vt_desconocido;;
						if (RightCompare(cadena," COMO REAL;")) { largotipo=11; tipo_def=vt_numerica; }
						if (RightCompare(cadena," COMO ENTERO;")) { largotipo=13; tipo_def=vt_numerica; }
						else if (RightCompare(cadena," COMO LOGICO;")) { largotipo=13; tipo_def=vt_logica; }
						else if (RightCompare(cadena," COMO CARACTER;")) { largotipo=15; tipo_def=vt_caracter; }
						cadena[cadena.size()-largotipo]=',';
						// evaluar los nombre de variables
						int tmp1=0,tmp2=0,tmp3=8;
						for (tmp1=8;tmp1<(int)cadena.size()-largotipo+1;tmp1++) {
							if (cadena[tmp1]=='(') tmp2++;
							if (cadena[tmp1]==')') tmp2--;
							if (tmp1>0 && tmp1<(int)cadena.size()-1)
								if (comillas<0 && cadena[tmp1]==' ' && cadena[tmp1-1]!='&' && cadena[tmp1-1]!='|'  && cadena[tmp1+1]!='&'  && cadena[tmp1+1]!='|')
								{SynError (47,"Se esperaba fin de expresion."); errores++;}
							if (tmp2==0 && cadena[tmp1]==',') { // comprobar validez
								str=cadena;
								str.erase(tmp1,str.size()-tmp1);
								str.erase(0,tmp3);
								if (str.find("(",0)==string::npos) {
									if (!CheckVariable(str,48)) errores++;
									else memoria->DefinirTipo(str,tipo_def);
								} else {
									str.erase(str.find("(",0),str.size()-str.find("(",0));
									SynError (212,string("No debe utilizar subindices (")+str+")."); errores++;
								}
								tmp3=tmp1+1;
							}
						}
						cadena[cadena.size()-largotipo]=' ';
					}
				}
			}
			if (instruccion=="ESCRIBIR " || instruccion=="ESCRIBNL "){  // ------------ ESCRIBIR -----------//
				if (cadena=="ESCRIBIR" || cadena=="ESCRIBIR ;"||cadena=="ESCRIBNL" || cadena=="ESCRIBNL ;")
				{SynError (53,"Faltan parametros."); errores++;}
				else {
					if (cadena[cadena.size()-1]==';')
						cadena[cadena.size()-1]=',';
					else
						cadena=cadena+",";
					int tmp1=0,tmp2=0, tmp3=9;
					comillas=-1; // cortar parametros
					for (tmp1=9;tmp1<(int)cadena.size();tmp1++) {
						if (cadena[tmp1]=='(') tmp2++;
						if (cadena[tmp1]==')') tmp2--;
						if (cadena[tmp1]=='\'') comillas=-comillas;
						if (comillas>0) continue;
						if (tmp1>0 && tmp1<(int)cadena.size()-1) {
							if (lazy_syntax && cadena[tmp1]==' ') cadena[tmp1]=',';
							if (cadena[tmp1]==' ' && cadena[tmp1-1]!='&' && cadena[tmp1-1]!='|'  && cadena[tmp1+1]!='&'  && cadena[tmp1+1]!='|')
								{SynError (54,"Se esperaba fin de expresion."); errores++;}
						}
						if (tmp2==0 && cadena[tmp1]==',') { // comprobar validez
							str=cadena;
							str.erase(tmp1,str.size()-tmp1);
							str.erase(0,tmp3);
							if (Lerrores==errores) EvaluarSC(str,tipo);
							tmp3=tmp1+1;
						}
					}
					cadena[cadena.size()-1]=';';
				}
			}
			if (instruccion=="ESPERAR "){  // ------------ ESCRIBIR -----------//
				if (cadena=="ESPERAR" || cadena=="ESPERAR ;")
					{SynError (217,"Faltan parametros."); errores++;}
				else {
					string str=cadena.substr(8);
					if (RightCompare(cadena," SEGUNDOS;")) str.erase(str.size()-10);
					else if (RightCompare(cadena," SEGUNDO;")) str.erase(str.size()-9);
					else if (RightCompare(cadena," MILISEGUNDOS;")) str.erase(str.size()-14);
					else if (RightCompare(cadena," MILISEGUNDO;")) str.erase(str.size()-13);
					else {SynError (218,"Falta unidad o unidad desconocida."); errores++;}
					EvaluarSC(str,tipo,vt_numerica);
					if (!tipo.cb_num) {SynError (219,"La longitud del intervalo debe ser numérica."); errores++;} else {
						for (int tmp1=0;tmp1<(int)str.size();tmp1++) if (str[tmp1]==' ') {SynError (120,"Se esperaba una sola expresión."); errores++;}
					}
				}
			}
			if (instruccion=="DIMENSION "){  // ------------ DIMENSION -----------//
				if (cadena=="DIMENSION" || cadena=="DIMENSION ;")
				{SynError (56,"Faltan parametros."); errores++;}
				else {
					if (cadena[cadena.size()-1]==';')
						cadena[cadena.size()-1]=',';
					else
						cadena=cadena+",";
					int tmp1=0,tmp2=0,tmp3=10;
					for (tmp1=10;tmp1<(int)cadena.size();tmp1++) {
						if (cadena[tmp1]=='(') tmp2++;
						if (cadena[tmp1]==')') tmp2--;
						if (tmp1>0 && tmp1<(int)cadena.size()-1)
							if (comillas<0 && cadena[tmp1]==' ' && cadena[tmp1-1]!='&' && cadena[tmp1-1]!='|'  && cadena[tmp1+1]!='&'  && cadena[tmp1+1]!='|')
							{SynError (57,"Se esperaba fin de expresion."); errores++;}
						if (tmp2==0 && cadena[tmp1]==',') { // comprobar validez
							str=cadena;
							str.erase(tmp1,str.size()-tmp1);
							str.erase(0,tmp3);
							if (str.find("(",0)<0 || str.find("(",0)>=str.size()){ 
								SynError (58,"Faltan subindices."); errores++;
								if (!CheckVariable(str,59)) errores++;
								else if (!memoria->EstaDefinida(str)) memoria->DefinirTipo(str,vt_desconocido); // para que aparezca en la lista de variables
							} else {
								string aname;
								str.erase(str.find("(",0),str.size()-str.find("(",0));
								if (!CheckVariable(str,60)) errores++;
								else aname=str; // para que aparezca en la lista de variables
								str=cadena;
								str.erase(tmp1,str.size()-tmp1);
								str.erase(0,str.find("(",tmp3)+1);
								if (str[str.size()-1]==')')
									str.erase(str.size()-1,1);
								str=str+",";
								
								// contar dimensiones y reservar espacio para el arreglo dims
								int parentesis=0, len=str.size(), ndims=0; bool comillas=false;
								for(int i=0;i<len;i++) { 
									if (str[i]=='\'') comillas=!comillas;
									else if (!comillas) {
										if (str[i]==',') ndims++;
										else if (str[i]=='(') parentesis++;
										else if (str[i]==')') parentesis--;
									}
								}
								int *dims=new int[ndims+1], idims=1; dims[0]=ndims;
								
								// comprobar los indices
								string str2,res_eval;
								while (str.find(",",0)>=0 && str.find(",",0)<str.size()){
									str2=str;
									str2.erase(str.find(",",0),str.size()-str.find(",",0));
									if (str2=="")
									{SynError (61,"Parametro nulo."); errores++;}
									if (Lerrores==errores) res_eval=EvaluarSC(str2,tipo,vt_numerica);
									if (tipo!=vt_error&&!tipo.cb_num) {
										{SynError (62,"No coinciden los tipos."); errores++;}
										dims[idims]=-1;
									} else {
										if (!IsNumericConstant(str2)) {
											dims[idims]=-1;
											if (!allow_dinamyc_dimensions)
												{SynError (153,"Las dimensiones deben ser constantes."); errores++;}
										} else {
											dims[idims]=(int)StrToDbl(res_eval);
										}
									}
									str.erase(0,str2.size()+1);
									idims++;
								}
								if (aname.size()) memoria->AgregarArreglo(aname,dims);
							}
							tmp3=tmp1+1;
						}
					}
				}
				cadena[cadena.size()-1]=';';
			}
			if (instruccion=="LEER "){  // ------------ LEER -----------//
				if (cadena=="LEER" || cadena=="LEER ;")
				{SynError (63,"Faltan parametros."); errores++;}
				else {
					if (cadena[cadena.size()-1]==';')
						cadena[cadena.size()-1]=',';
					else
						cadena=cadena+",";
					int tmp1=0,tmp2=0,tmp3=5;
					comillas=-1;
					for (tmp1=5;tmp1<(int)cadena.size();tmp1++) {
						if (cadena[tmp1]=='(') tmp2++;
						if (cadena[tmp1]==')') tmp2--;
						if (cadena[tmp1]=='\'') { comillas=-comillas; continue; }
						else if (comillas>0) continue;
						if (tmp1>0 && tmp1<(int)cadena.size()-1) {
							if (lazy_syntax && cadena[tmp1]==' ') cadena[tmp1]=',';
							if (comillas<0 && cadena[tmp1]==' ' && cadena[tmp1-1]!='&' && cadena[tmp1-1]!='|'  && cadena[tmp1+1]!='&'  && cadena[tmp1+1]!='|')
								{SynError (64,"Se esperaba fin de expresion."); errores++;}
						}
						if (tmp2==0 && cadena[tmp1]==',') { // comprobar validez
							str=cadena;
							str.erase(tmp1,str.size()-tmp1);
							str.erase(0,tmp3);
							if (str.find("(",0)==string::npos) {
								if (!CheckVariable(str,65)) errores++;
								else {
									if (!memoria->EstaDefinida(str)) memoria->DefinirTipo(str,vt_desconocido); // para que aparezca en la lista de variables
									if (memoria->LeerTipo(str).dims) SynError(999,"Faltan subindices para el arreglo ("+str+").");
								}
							} else {
								bool name_ok=true;
								string aname=str.substr(0,str.find("(",0));
								if (!CheckVariable(aname,66)) { errores++; name_ok=false; }
								else if (!memoria->EstaDefinida(aname)) memoria->DefinirTipo(aname,vt_desconocido); // para que aparezca en la lista de variables
								if (!memoria->LeerTipo(aname).dims) { SynError(999,"La variable ("+aname+") no es un arreglo."); name_ok=false; }
								str=cadena;
								str.erase(tmp1,str.size()-tmp1);
								str.erase(0,tmp3);
								str.erase(0,str.find("(",0));
								if (str[str.size()-1]==')')
									str.erase(str.size()-1,1);
								str.erase(0,1);
								str=str+",";
								string str2;
								// comprobar los indices
								int ca=0;
								while (str.find(",",0)>=0 && str.find(",",0)<str.size()){
									str2=str;
									str2.erase(str.find(",",0),str.size()-str.find(",",0));
									// if (str2=="") {SynError (67,"Parametro nulo."); errores++;}
									if (Lerrores==errores) EvaluarSC(str2,tipo,vt_numerica);
									if (tipo!=vt_error&&!tipo.cb_num)
										{SynError (154,"No coinciden los tipos."); errores++;}
									str.erase(0,str2.size()+1);
									ca++;
								}
								if (name_ok && memoria->LeerTipo(aname).dims[0]!=ca) {
									SynError(999,string("Cantidad de indices incorrecta para el arreglo (")+aname+(")"));
									return false;
								}
							}
							tmp3=tmp1+1;
						}
					}
				}
				cadena[cadena.size()-1]=';';
			}
			if (instruccion=="PARA "){  // ------------ PARA -----------//
				str=cadena; // cortar instruccion
				str.erase(0,5);
				if ((str.find(" ",0)<0 || str.find(" ",0)>=str.size()))
				{SynError (70,"Faltan parametros."); errores++;}
				if (!RightCompareFix(str," HACER")) {
					if (lazy_syntax) { str+=" HACER"; cadena+=" HACER";}
					else {SynError (71,"Falta HACER."); errores++;}
				}
				if (RightCompareFix(str," HACER")) { // comprobar asignacion
					str.erase(str.find(" ",0),str.size()-str.find(" ",0));
					if (str.find("<-",0)<0 || str.find("<-",0)>=str.size()) // Comprobar asignacion
					{SynError (72,"Se esperaba asignacion."); errores++;}
					else
						if (RightCompare(str,"<-HASTA") || RightCompare(str,"<-CON PASO") || LeftCompare(str,"<-"))
						{SynError (73,"Asignacion incompleta."); errores++;}
						else {
							str.erase(str.find("<-",0),str.size()-str.find("<-",0));
							if (!CheckVariable(str,74)) errores++; else {
								memoria->DefinirTipo(str,vt_numerica); // para que aparezca en la lista de variables
								if (Lerrores==errores) EvaluarSC(str,tipo,vt_numerica);
								if (!tipo.cb_num)
									{SynError (76,"No coinciden los tipos."); errores++;}
								str=cadena;
								str.erase(0,str.find("<-")+2);
								str.erase(str.find(" "),str.size()-str.find(" ",0));
								if (Lerrores==errores) EvaluarSC(str,tipo,vt_numerica);
								if (!tipo.cb_num)
									{SynError (77,"No coinciden los tipos."); errores++;}
								else { // comprobar hasta y variable final
									str=cadena;
									size_t pos_hasta=str.find(" ",8);
									str.erase(0,pos_hasta);
									if (lazy_syntax && LeftCompare(str," CON PASO ")) { // si esta el "CON PASO" antes del "HASTA", dar vuelta
										size_t e=str.find(" ",10);
										if (e!=string::npos) { // si hay algo despues del "CON PASO"
											str.erase(e); // corta la parte de "CON PASO X"
											cadena=cadena.substr(0,pos_hasta)+cadena.substr(pos_hasta+e); // rearma la cadena sin el paso
											cadena.insert(cadena.size()-6,str); // inserta el paso
											str=cadena; // pone str como si no hubiese pasado nada
											str.erase(0,pos_hasta);
										}
									}
									if (!LeftCompare(str," HASTA ")) {
										if (LeftCompare(str," CON PASO ")) { 
											SynError (216,"CON PASO va despues de HASTA."); errores++;
										} else {
											SynError (78,"Falta HASTA."); errores++;
										}
									} else if (LeftCompare(str," HASTA HACER"))
										{SynError (79,"Falta el valor final del PARA."); errores++;}
									else {
										str.erase(0,7); str.erase(str.size()-6,6);
										if (str.find(" ",0)<0 || str.find(" ",0)>str.size()) {
											if (Lerrores==errores) EvaluarSC(str,tipo,vt_numerica);
											if (!tipo.cb_num) {SynError (80,"No coinciden los tipos."); errores++;}
										} else {
											str.erase(str.find(" ",0),str.size()-str.find(" ",0));
											if (Lerrores==errores) EvaluarSC(str,tipo,vt_numerica);
											if (!tipo.cb_num) {SynError (81,"No coinciden los tipos."); errores++;}
											str=cadena; // comprobar con paso
											str.erase(0,str.find("HASTA",6)+6);
											str.erase(0,str.find(" ",0)+1);
											str.erase(str.size()-6,6);
											if (!LeftCompare(str,"CON PASO "))
											{SynError (82,"Se esparaba CON PASO o fin de instruccion."); errores++;}
											else {
												str.erase(0,9);
												EvaluarSC(str,tipo,vt_numerica);
												if (!tipo.cb_num) {SynError (84,"No coinciden los tipos."); errores++;}
											}
										}
									}
								}
							}
						}
				}
			}
			
			if (instruccion=="PARACADA "){  // ------------ PARA CADA -----------//
				str=cadena; // cortar instruccion
				str.erase(0,9);
				if ((str.find(" ",0)<0 || str.find(" ",0)>=str.size()))
				{SynError (70,"Faltan parametros."); errores++;} /// 999
				if (!RightCompareFix(str," HACER")) {
					if (lazy_syntax) { str+=" HACER"; cadena+=" HACER";}
					else {SynError (71,"Falta HACER."); str+=" HACER"; errores++;} /// 999
				}
				if (RightCompareFix(str," HACER")) {
					int i=0; while (str[i]!=' ') i++;
					if (!CheckVariable(str.substr(0,i),999)) errores++;
					if (str.substr(i,4)==" EN ") cadena.replace(9+i,4," DE ");
					else if (str.substr(i,4)!=" DE ") {SynError (999,"Se esperaba DE o EN."); errores++;} 
					else if (!CheckVariable(str.substr(i+4,str.size()-i-10),999)) errores++;
				}
			}
			
			if (instruccion==":") {  // ------------ opcion del SEGUN -----------//
				int p;
				// permitir utiliza O para separar la posibles opciones
				if (lazy_syntax) {
					while ((p=cadena.find(" O "))!=-1) cadena.replace(p,3,",");
					while ((p=cadena.find("|"))!=-1) cadena.replace(p,1,",");
				}
				cadena[cadena.size()]=',';
				int i=0;
				while ((p=PSeudoFind(cadena,',',p+1))!=-1) {
					tipo=vt_caracter_o_numerica;
					EvaluarSC(cadena.substr(i,p-i-1),tipo,lazy_syntax?vt_caracter_o_numerica:vt_numerica);
					if (!tipo.cb_num&&(!tipo.cb_car||!lazy_syntax))
						SynError (203,"Las opciones deben ser de tipo numerico."); errores++;
					i=p+1;
				}
				cadena[cadena.size()]=':';
			}
			
			if (instruccion=="<-") {  // ------------ ASIGNACION -----------//
				str=cadena;
				str.erase(str.find("<-",0),str.size()-str.find("<-",0));
				if (str.size()==0)
				{SynError (85,"Asignacion incompleta."); errores++;}
				else {
					if (!CheckVariable(str,86)) errores++;
					string vname=str;
					str=cadena;
					str.erase(0,str.find("<-",0)+2);
					comillas=-1;parentesis=0;
					for (int y=0;y<(int)str.size();y++){ // comprobar que se un solo parametro
						if (str[y]=='(') parentesis++;
						if (str[y]==')') parentesis--;
						if (str[y]=='\'') comillas=-comillas;
						if (y>0 && y<(int)str.size()-1)
							if (comillas<0 && str[y]==' ' && str[y-1]!='&' && str[y-1]!='|'  && str[y+1]!='&'  && str[y+1]!='|')
							{SynError (87,"Se esperaba fin de expresion."); errores++;}
						if (comillas<0 && parentesis==1 && str[y]==',')
						{SynError (88,"Demasiados parametros."); errores++;}
					}
					if (str.size()==3)
					{SynError (89,"Asignacion incompleta."); errores++;}
					else {
						str.erase(str.size()-1,1);
						if (Lerrores==errores) EvaluarSC(str,tipo);
						if (!memoria->LeerTipo(vname).can_be(tipo)) {
							SynError(125,"No coinciden los tipos."); errores++; 
							if (!memoria->EstaDefinida(str)) memoria->DefinirTipo(str,vt_desconocido); // para que aparezca en la lista de variables
						}
						else memoria->DefinirTipo(vname,tipo);
					}
				}
			}
			if (instruccion=="SI "){  // ------------ SI -----------//
				if (cadena=="SI")
				{ SynError (90,"Falta condicion."); errores++; }
				else
					str=cadena; // Comprobar la condicion
				str.erase(0,3);
				// comprobar que no halla espacios
				comillas=-1;
				for (int tmp1=0;tmp1<(int)str.size();tmp1++) {
					if (str[tmp1]=='\'') comillas=-comillas;
					if (tmp1>0 && tmp1<(int)str.size()-1)
						if (comillas<0 && str[tmp1]==' ' && str[tmp1-1]!='&' && str[tmp1-1]!='|'  && str[tmp1+1]!='&'  && str[tmp1+1]!='|') {
							if (lazy_syntax) {
								string aux=str.substr(tmp1); flag_pyc++;
								programa.Insert(x+1,aux);
								break;
							} else
								SynError (91,"Se esperaba ENTONCES o fin de expresion."); errores++;
						}
				}
				tipo=vt_logica;
				if (Lerrores==errores) EvaluarSC(str,tipo,vt_logica);
				if (!tipo.cb_log) { SynError (92,"No coinciden los tipos."); errores++; }
			}
			if (instruccion=="HASTA QUE "||instruccion=="MIENTRAS QUE "){  // ------------ HASTA QUE -----------//
				if (cadena=="HASTA QUE"||cadena=="MIENTRAS QUE")
				{ SynError (93,"Falta condicion."); errores++; }
				else {
					str=cadena; // Comprobar la condicion
					str.erase(0,instruccion=="HASTA QUE "?10:13);
					// comprobar que no halla espacios
					comillas=-1;
					for (int tmp1=0;tmp1<(int)str.size();tmp1++) {
						if (str[tmp1]=='\'') comillas=-comillas;
						if (tmp1>0 && tmp1<(int)str.size()-1)
							if (comillas<0 && str[tmp1]==' ' && str[tmp1-1]!='&' && str[tmp1-1]!='|'  && str[tmp1+1]!='&'  && str[tmp1+1]!='|')
							{SynError (94,"Se esperaba fin de expresion."); errores++;}
					}
					if (str[str.size()-1]==';') {
						str=str.substr(0,str.size()-1);
						cadena=cadena.substr(0,cadena.size()-1);
					}
					if (Lerrores==errores) EvaluarSC(str,tipo,vt_logica);
					if (!tipo.cb_log) { SynError (95,"No coinciden los tipos."); errores++; }
				}
			}
			if (instruccion=="SEGUN "){  // ------------ SEGUN -----------//
//				last_was_segun=true;
				if (cadena=="SEGUN HACER" || cadena=="SEGUN")
				{ SynError (96,"Falta condicion."); errores++; }
				else
					if (!RightCompareFix(cadena," HACER")) {
						if (lazy_syntax) cadena+=" HACER";
						else { SynError (97,"Falta HACER."); errores++; }
					}
					if (RightCompareFix(str," HACER")) {
						str=cadena; // Comprobar la condicion
						str.erase(str.size()-6,6);
						str.erase(0,6);
						// comprobar que no halla espacios
						comillas=-1;
						for (int tmp1=0;tmp1<(int)str.size();tmp1++) {
							if (str[tmp1]=='\'') comillas=-comillas;
							if (tmp1>0 && tmp1<(int)str.size()-1)
								if (comillas<0 && str[tmp1]==' ' && str[tmp1-1]!='&' && str[tmp1-1]!='|'  && str[tmp1+1]!='&'  && str[tmp1+1]!='|')
								{SynError (98,"Se esperaba fin de expresion."); errores++;}
						}
						if (Lerrores==errores) EvaluarSC(str,tipo,lazy_syntax?vt_caracter_o_numerica:vt_numerica);
						if (!tipo.cb_num&&!lazy_syntax) { SynError (100,"No coinciden los tipos."); errores++; }
					}
			}
			if (instruccion=="MIENTRAS ") { // ------------ MIENTRAS -----------//
				if (cadena=="MIENTRAS ")
				{ SynError (101,"Falta condicion."); errores++; }
				else
					if (RightCompare(cadena,";")) {
						SynError (999,"MIENTRAS no lleva punto y coma luego de la condicion."); errores++;
						cadena.erase(cadena.size()-1,1);
					}
					if (!RightCompareFix(cadena," HACER")) {
						if (lazy_syntax) cadena+=" HACER";
						else { SynError (102,"Falta HACER."); errores++; }
					}
					if (RightCompareFix(cadena," HACER")) {
						str=cadena; // Comprobar la condicion
						str.erase(str.size()-6,6);
						str.erase(0,9);
						// comprobar que no halla espacios
						comillas=-1;
						for (int tmp1=0;tmp1<(int)str.size();tmp1++) {
							if (str[tmp1]=='\'') comillas=-comillas;
							if (tmp1>0 && tmp1<(int)str.size()-1)
								if (comillas<0 && str[tmp1]==' ' && str[tmp1-1]!='&' && str[tmp1-1]!='|'  && str[tmp1+1]!='&'  && str[tmp1+1]!='|') {
									if (lazy_syntax) {
										string aux=str.substr(tmp1); flag_pyc++;
										programa.Insert(x+1,aux);
										break;
									} else {
										SynError (103,"Se esperaba fin de expresion."); errores++;
									}
								}
						}
						if (Lerrores==errores) {
							EvaluarSC(str,tipo,vt_logica);
							if (tipo!=vt_error && !tipo.cb_log) { SynError (104,"No coinciden los tipos."); errores++; }
						}
					}
			}
			if (LeftCompare(instruccion,"FIN") || instruccion=="REPETIR "||instruccion=="BORRARPANTALLA"||instruccion=="ESPERARTECLA")
				if (cadena.find(" ",0)>=0 && cadena.find(" ",0)<cadena.size() && cadena.substr(cadena.find(" ",0))!=" ;") {
					SynError (105,"La instruccion no debe tener parametros."); errores++;
					cadena.erase(cadena.find(" ",0),cadena.size()-cadena.find(" ",0));
				}
			if (instruccion=="Error?" && cadena!="" && cadena!=";") {
				if (LeftCompare(cadena,"FIN "))
				{SynError (99,"Instruccion no valida."); errores++;}
				else
					{SynError (106,"Instruccion no valida."); errores++;}
			}
			// llama directa a un subproceso
			if (instruccion=="INVOCAR ") {
				int p=8;
				string fname=NextToken(cadena,p);
				funcion *func=EsFuncion(fname);
				string args=cadena.substr(p);
				if (args=="") args="()"; // para que siempre aparezcan las llaves y se eviten así problemas
				if (args=="()" && func->cant_arg!=0) {SynError (999,string("Se esperaban argumentos para el subproceso (")+fname+")."); errores++;}
				else if (args!="()" && func->cant_arg==0) {SynError (999,string("El subproceso (")+fname+") no debe recibir argumentos."); errores++;}
				else if (args[0]!='(') {SynError (999,"Los argumentos para invocar a un subproceso deben ir entre paréntesis."); errores++;}
				else { // entonces tiene argumentos, y requiere argumentos, ver que la cantidad esté bien
					int args_last_pos=BuscarComa(args,1,args.length()-1,')');
					if (args_last_pos!=-1) { // si faltaban cerrar parentesis, el error salto antes
						int pos_coma=0, last_pos_coma=0, cant_args=0; tipo_var tipo;
						do {
							pos_coma=BuscarComa(args,pos_coma+1,args_last_pos,',');
							if (pos_coma==-1) pos_coma=args_last_pos;
							EvaluarSC(args.substr(last_pos_coma+1,pos_coma-last_pos_coma-1),tipo,func->tipos[cant_args+1]);
							cant_args++; last_pos_coma=pos_coma;
						} while (pos_coma!=args_last_pos);
						cerr<<int(args.length())-1<<endl;
						if (args_last_pos!=int(args.length())-2) {SynError (999,"Se esperaba fin de instrucción."); errores++;} // el -2 de la condicion es por el punto y coma
					}
				}
			}
			// Controlar Cierre de Bucles
			if (cadena=="REPETIR") 
			{bucles.push(programa.GetLoc(x,"REPETIR"));}
			if (cadena=="FINSEGUN") {
				if (!bucles.empty() && bucles.top()=="SEGUN") {
					bucles.pop();
				} else {
					SynError (107,"FINSEGUN mal colocado."); errores++;}
			}
			if (cadena=="FINPARA") {
				if (!bucles.empty() && (bucles.top()=="PARA"||bucles.top()=="PARACADA")) {
					bucles.pop();
				} else {
					SynError (108,"FINPARA mal colocado."); errores++;}
			}
			if (cadena=="FINMIENTRAS") {
				if (!bucles.empty() && (bucles.top()=="MIENTRAS")) {
					bucles.pop();
				} else {
					SynError (109,"FINMIENTRAS mal colocado."); errores++;}
			}
			if (cadena=="FINSI") {
				if (!bucles.empty() && (bucles.top()=="SI")) {
					bucles.pop();
				} else {
					SynError (110,"FINSI mal colocado."); errores++;}
			}
			if (LeftCompare(cadena,"HASTA QUE ")) {
				if (!bucles.empty() && bucles.top()=="REPETIR") {
					bucles.pop();
				} else {
					SynError (111,"HASTA QUE mal colocado."); errores++;}
			}
			if (lazy_syntax && LeftCompare(cadena,"MIENTRAS QUE ")) {
				if (!bucles.empty() && bucles.top()=="REPETIR") {
					bucles.pop();
				} else {
					SynError (999,"MIENTRAS QUE mal colocado."); errores++;}
			}
			if ( (x>0 && cadena=="SINO" && LeftCompare(programa[x-1],"SI "))
				|| (x>0 && cadena=="SINO" && LeftCompare(programa[x-1],"ENTONCES")) )
			{ SynError (113,"Debe haber acciones en la salida por verdadero."); errores++;}
			
			// Actualiza los vectores
			programa[x]=cadena;
			
			if (flag_pyc==0) /*LineNumber++*/; else flag_pyc-=1;
			// Borra cadenas vacias
			if (cadena.size()==0 || cadena==";") {programa.Erase(x);x--;}
		}
	}
	
	InformUnclosedLoops(bucles,errores);
	
	return SynErrores;
}

int ParseInspection(string &cadena) {
	SynCheckAux1(cadena); // acomodar caracteres
	if (cadena.size() && cadena[cadena.size()-1]==';') cadena.erase(cadena.size()-1,1);
	SynCheckAux2(cadena); // word_operators
	int errores=0, flag_pyc=0;
	SynCheckAux3(-1,cadena,errores,"ESCRIBIR",flag_pyc); // verificar operadores
	if (flag_pyc) { SynError (999,"No puede haber más de una expresión."); errores++; }
	return errores+flag_pyc;
}

