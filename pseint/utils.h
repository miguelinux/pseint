#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "new_memoria.h"
#include "new_programa.h"
#include "global.h"
using namespace std;

// ***************** Conversiones Numeros/Cadenas **********************

// -------------------------------------------------------------------
//    Convierte un entero a una cadena
// -------------------------------------------------------------------
inline string IntToStr(int f) {
	string s; 
	char stmp[]="0";
	int tmp;
	int a=1,b,c;
	if (f<0) {f=f*(-1);s="-";}
	while (a<=f) a=a*10;
	a=a/10;
	while (a>0) {
		b=a; c=1;
		while (b<=f) {b+=a;c++;}
		stmp[0]=(c-1)+48;
		s+=stmp;
		f-=(b-a);
		a/=10; 
	}
	tmp=s.find(".",0);
	while (s[s.size()-1]==' ' && (int)s.size()>tmp && tmp>=0)
		s.erase(s.size()-1,1);
	if (s=="") s="0";
	return s;
}

// -------------------------------------------------------------------
//    Convierte una cadena a un double
// -------------------------------------------------------------------
inline double StrToDbl(const string &s) {
	bool Neg=0; // Bandera de Numero negativo
	int i=0, c=int(s[0]);
	if (c=='-') {i++; Neg=true;}
	else if (c=='+') i++;
	double f=0,punto=0;
	while ((c=int(s[i]))!=0) {
		if (punto==0) {
			if (c==46) { punto=.1; }
			else { f*=10; f+=c-48; }
		} else {
			f+=(c-48)*punto;
			punto/=10; }
		i++;
	}
	if (Neg) return -f;
	else return f;
}

// -------------------------------------------------------------------
//    Convierte un double a una cadena - Alta Precision
// -------------------------------------------------------------------
inline string DblToStr(double f) {
	static char ret[2048];
	sprintf(ret,"%.50f",f);
	// eliminar ceros que sobren y el punto si era entero
	int i=0; while (ret[i]!=0) i++;
	while (ret[--i]=='0'); 
	if (ret[i]=='.') i--;
	ret[++i]='\0';
	return ret;
}
// -------------------------------------------------------------------
//    Convierte un double a una cadena - Baja Precision
// -------------------------------------------------------------------
inline string DblToStr(double f,bool low_precision) {
	static char ret[2048];
	sprintf(ret,"%.10f",f);
	// eliminar ceros que sobren y el punto si era entero
	int i=0; while (ret[i]!=0) i++;
	while (ret[--i]=='0');
	if (ret[i]=='.') i--;
	ret[++i]='\0';
	return ret;
}

// ***************** Control de Errores y Depuración **********************

// ------------------------------------------------------------
//    Informa un error en tiempo de ejecucion
// ------------------------------------------------------------
void ExeError(int num,string s) ;

// ------------------------------------------------------------
//    Informa un error de syntaxis antes de la ejecucion
// ------------------------------------------------------------
void SynError(int num,string s, InstruccionLoc il) ;
void SynError(int num,string s, int line=-1, int inst=-1) ;

// ------------------------------------------------------------
//    Comprueba que sea un identificador correcto y unico
//    A diferencia del anterior, no tiene en cuenta las
//  funciones predefinidas.
// ------------------------------------------------------------
bool CheckVariable(string str, int errcode=-1) ;


// *********************** Funciones Auxiliares **************************
// ----------------------------------------------------------------------
//    Devuelve el tipo de una variable o una funcion predefinida
// ----------------------------------------------------------------------
tipo_var TipoVar(string str) ;

// ----------------------------------------------------------------------
//    Compara los comienzos de dos cadenas (case sensitive)
// ----------------------------------------------------------------------
bool LeftCompare(string a, string b) ;

// ----------------------------------------------------------------------
//    Compara las terminaciones de dos cadenas (case sensitve)
// ----------------------------------------------------------------------
bool RightCompare(string a, string b) ;

// ----------------------------------------------------------------------
//    Compara parte de una cadena con otra (case insensitve)
// ----------------------------------------------------------------------
bool MidCompareNC(string a, string b, int from) ;

// ----------------------------------------------------------------------
//    Pasa una cadena a mayusculas
// ----------------------------------------------------------------------
string toUpper(string a) ;

// ----------------------------------------------------------------------
//    Pasa una cadena a mayusculas
// ----------------------------------------------------------------------
string toLower(string a) ;

// ----------------------------------------------------------------------
//    Averigua el tipo de variable para un dato
// ----------------------------------------------------------------------
tipo_var GuestTipo(string str) ;

// ----------------------------------------------------------------------
//    Reemplaza una cadena por otra si es que se encuentra
// ----------------------------------------------------------------------
bool ReplaceIfFound(string &str, string str1,string str2,bool saltear_literales=false) ;


// **************************************************************************

// funciones auxiliares para psexport
inline int max(int a,int b);
inline int min(int a,int b);

string CutString(string s, int a, int b=0);

// determina si una letra puede ser parte de una palabra clave o identificador
bool parteDePalabra(char c) ;

// corrige diferencias entre la codificación que usa pseint (ascii pelado) y la de la consola de windows
void fixwincharset(string &s, bool reverse=false);

// determina si un caracter (que debe venir en mayúsculas es letra (incluye acentos y ñs)
inline bool EsLetra(const char &c) {
	return ( (c>='A' && c<='Z') || (lazy_syntax && (c=='Á'||c=='É'||c=='Í'||c=='Ó'||c=='Ú'||c=='Ñ'||c=='Ü') ) || c=='_' );
}

inline char ToUpper(char c) {
	if (c>96 && c<123) c-=32;
	else if (c=='á') c='Á';
	else if (c=='é') c='É';
	else if (c=='í') c='Í';
	else if (c=='ó') c='Ó';
	else if (c=='ú') c='Ú';
	else if (c=='ü') c='Ü';
	else if (c=='ñ') c='Ñ';
	return c;
}

// "extrae" una palabra, una constante, o un operador, desde la pos p, no modifica la cadena, sino que avanza el indice p
string NextToken(string &cadena, int &p);



#endif

