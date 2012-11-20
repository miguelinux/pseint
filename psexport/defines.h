#ifndef DEFINES_H
#define DEFINES_H
#include <string>
#include <list>
using namespace std;

// tipos auxiliares
struct t_instruccion{
	string nombre;
	string par1,par2,par3,par4;
};

#define t_algoritmo list <t_instruccion>
#define t_algoritmo_it list <t_instruccion>::iterator
#define t_programa list<string>

#ifndef _USE_COUT
#define insertar(prog,text) prog.insert(prog.end(),text)
#else
#define insertar(prog,text) cerr<<text<<endl
#endif

#endif
