#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "new_memoria.h"
using namespace std;

// -------------------------------------------------------------------
//    Convierte un entero a una cadena
// -------------------------------------------------------------------
string IntToStr(int f) ;

// -------------------------------------------------------------------
//    Convierte una cadena a un double
// -------------------------------------------------------------------
double StrToDbl(string s) ;

// -------------------------------------------------------------------
//    Convierte un double a una cadena - Alta Precision
// -------------------------------------------------------------------
string DblToStr(double f,int pres=50) ;
// -------------------------------------------------------------------
//    Convierte un double a una cadena
// -------------------------------------------------------------------
//string DblToStr(double f) ;


// ***************** Control de Errores y Depuración **********************

// ------------------------------------------------------------
//    Informa un error en tiempo de ejecucion
// ------------------------------------------------------------
void ExeError(int num,string s) ;

// ------------------------------------------------------------
//    Informa un error de syntaxis antes de la ejecucion
// ------------------------------------------------------------
void SynError(int num,string s, int line=-1) ;

// ------------------------------------------------------------
//    Analiza el tipo de error devuelo por Evaluar(expresion)
//  e informa segun corresponda.
// ------------------------------------------------------------
//void ExpError(char tipo, int level, int line) ;

// ------------------------------------------------------------
//    Comprueba que sea un identificador correcto y unico
//    A diferencia del anterior, no tiene en cuenta las
//  funciones predefinidas.
// ------------------------------------------------------------
bool CheckVariable(string str) ;


// ------------------------------------------------------------
//    Comprueba que sea un identificador correcto y unico
// ------------------------------------------------------------
bool CheckVariableForW(string str) ;


// *********************** Funciones Auxiliares **************************

// ----------------------------------------------------------------------
//    Escribe un variable en Memoria realizando algunas verificaciones
// ----------------------------------------------------------------------
//void EscribirVar(string nombre,tipo_var tipo,string contenido) ;

// ----------------------------------------------------------------------
//    Lee una variable de Memoria o evalua una funcion predefinida
// ----------------------------------------------------------------------
//string LeerVar(string str) ;

// ----------------------------------------------------------------------
//    Devuelve el tipo de una variable o una funcion predefinida
// ----------------------------------------------------------------------
tipo_var TipoVar(string str) ;

// ----------------------------------------------------------------------
//    Escribe una expresion en el ambiente
// ----------------------------------------------------------------------
void Escribir(string aux1) ;

// ----------------------------------------------------------------------
//    Compara los comienzos de dos cadenas (case sensitive)
// ----------------------------------------------------------------------
bool LeftCompare(string a, string b) ;

// ----------------------------------------------------------------------
//    Compara los comienzos de dos cadenas (case insensitve)
// ----------------------------------------------------------------------
bool LeftCompareNC(string a, string b);

// ----------------------------------------------------------------------
//    Compara las terminaciones de dos cadenas (case sensitve)
// ----------------------------------------------------------------------
bool RightCompare(string a, string b) ;

// ----------------------------------------------------------------------
//    Compara las terminaciones de dos cadenas (case insensitve)
// ----------------------------------------------------------------------
bool RightCompareNC(string a, string b) ;

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

#endif
