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

#define t_proceso list <t_instruccion>
#define t_proceso_it list <t_instruccion>::iterator
#define t_programa list<t_proceso>
#define t_programa_it list<t_proceso>::iterator
#define t_output list<string>
#define t_output_it list<string>::iterator
#define t_arglist list<string>
#define t_arglist_it list<string>::iterator


//#ifndef _USE_COUT
#define insertar(prog,text) prog.insert((prog).end(),text)
#define insertar_out(prog,out) { t_output_it it=(out).begin(); while (it!=(out).end()) { (prog).insert((prog).end(),*it); ++it; } }
#define mem_iterator map<string,tipo_var>::iterator
//#else
//#define insertar(prog,text) cerr<<text<<endl
//#endif

#endif
