#ifndef NEW_EVALUAR_H
#define NEW_EVALUAR_H
#include "new_memoria.h"

class Funcion;

bool CheckDims(string &str); // recibe una cadena del tipo "lala(1,x)" y verifica que lala sea un arreglo y los indices sean correctos (la modifica y la vuelve a dejar como estaba, por eso no es const, pero a fines practicos si)
int BuscarComa(const string &expresion, int p1, int p2, char coma=',');
int BuscarOperador(const string &expresion, int &p1, int &p2);
tipo_var DeterminarTipo(const string &expresion, int p1, int p2);
bool AplicarTipo(const string &expresion, int &p1, int &p2, tipo_var tipo);
string EvaluarFuncion(const Funcion *func, const string &argumentos, tipo_var &tipo, bool for_expresion=true);
string Evaluar(const string &expresion, int &p1, int &p2, tipo_var &tipo);
string Evaluar(string expresion, tipo_var &tipo, tipo_var forced_tipo=vt_desconocido);
string EvaluarSC(string expresion, tipo_var &tipo, tipo_var forced_tipo=vt_desconocido);
bool PalabraReservada(const string &str, bool exclude_functions=false);

#endif

