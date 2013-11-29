#include<iostream>
#include<iomanip>
#include<stack>
#include "new_memoria.h"

class ExpIntrucciones;

// conversion de expresiones
string modificarConstante(string s,int diff);

string buscarOperando(const string &exp, int comienzo, int direccion);

string colocarParentesis(const string &exp);

string restarUno(string exp);

string expresion(string exp, tipo_var &tipo);

string expresion(string exp);

string invert_expresion(string expr);
