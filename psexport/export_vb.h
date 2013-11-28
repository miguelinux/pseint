#ifndef EXPORT_VB_H
#define EXPORT_VB_H
#include <string>
#include "defines.h"
using namespace std;

/**
* @brief traduce la llamada a una función predefinida a VB .NET
*
* @param name    nombre de la función en pseudocódigo
* @param args    string con la lista de argumentos (incluye los paréntesis)
**/
string vb_function(string name, string args);



/**
* @brief traduce el algoritmo a VB .NET
*
* @param prog    argumento de salida, donde colocar las instrucciones traducidas
* @param alg     argumento de entrada, con el algoritmo ya parseado
**/
void translate_vb(t_output &prog, t_programa &alg);

#endif

