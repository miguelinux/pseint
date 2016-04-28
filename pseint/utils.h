#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "new_memoria.h"
#include "new_programa.h"
#include "global.h"
using namespace std;

void show_user_info(string msg);
void show_user_info(string msg1, int num, string msg2);

// ***************** Control de Errores y Depuración **********************

// ------------------------------------------------------------
//    Informa un error en tiempo de ejecucion
// ------------------------------------------------------------
void ExeError(int num,string s) ;
void ExeError(int num,string s, bool use_syn_if_not_running);

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
string ToUpper(string a) ;

// ----------------------------------------------------------------------
//    Pasa una cadena a mayusculas
// ----------------------------------------------------------------------
string ToLower(string a) ;

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

inline char ToUpper(const char c) {
	if (c>96 && c<123) return c-32;
	if (c=='á') return 'Á';
	if (c=='é') return 'É';
	if (c=='í') return 'Í';
	if (c=='ó') return 'Ó';
	if (c=='ú') return 'Ú';
	if (c=='ü') return 'Ü';
	if (c=='ñ') return 'Ñ';
	return c;
}

// determina si un caracter (que debe venir en mayúsculas es letra (incluye acentos y ñs)
inline bool EsLetra(const char &_c, bool incluir_numeros=false) {
	return (_c>='A' && _c<='Z') || (lang[LS_ALLOW_ACCENTS] && (_c=='Á'||_c=='É'||_c=='Í'||_c=='Ó'||_c=='Ú'||_c=='Ñ'||_c=='Ü') ) || _c=='_'  || (incluir_numeros && _c>='0' && _c<='9');
}

// "extrae" una palabra, una constante, o un operador, desde la pos p, no modifica la cadena, sino que avanza el indice p
string NextToken(string &cadena, int &p);


Funcion *ParsearCabeceraDeSubProceso(string cadena, bool es_proceso, int &errores);
string ExtraerNombreDeSubProceso(string cadena);

string FirstWord(const string &s);

#endif

