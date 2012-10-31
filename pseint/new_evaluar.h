#ifndef NEW_EVALUAR_H
#define NEW_EVALUAR_H
#include "new_memoria.h"

class Funcion;

bool CheckDims(string &str); // recibe una cadena del tipo "lala(1,x)" y verifica que lala sea un arreglo y los indices sean correctos
int BuscarComa(string &expresion, int p1, int p2, char coma=',');
int BuscarOperador(string &expresion, int &p1, int &p2);
tipo_var DeterminarTipo(string &expresion, int p1, int p2);
bool AplicarTipo(string &expresion, int &p1, int &p2, tipo_var tipo);
string EvaluarFuncion(Funcion *func, string argumentos, tipo_var &tipo, bool for_expresion=true);
string Evaluar(string &expresion, int &p1, int &p2, tipo_var &tipo);
string Evaluar(string expresion, tipo_var &tipo, tipo_var forced_tipo=vt_desconocido);
string EvaluarSC(string expresion, tipo_var &tipo, tipo_var forced_tipo=vt_desconocido);

#endif

