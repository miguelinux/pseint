#ifndef INTERCAMBIO_H
#define INTERCAMBIO_H

#include <string>
#include <vector>
using namespace std;

#ifdef USE_ZOCKETS
#include "zockets.h"
#endif

// *********************** Intercambio ****************************

class Intercambio {
	

	bool running;
	int instNumber;          // Numero de linea que se está ejecutando (base 1)
	int lineNumber;          // Numero de linea que se está ejecutando (base 1)
	vector <string> Archivo; // Archivo original
	vector <string> Errores; // Descripcion de los errores encontrados
	vector <int> Lineas;     // Numeros de lines correspondientes a los errores	

#ifdef USE_ZOCKETS
	
	// para pasar info cuando hay errores al evaluar, pero es una expresion para depuracion
	bool evaluating_for_debug, is_evaluation_error;
	string evaluation_error;
	
	vector <bool> autoevaluaciones_valid; // expresiones para el depurador que se muestran automaticamente en cada paso
	vector <string> autoevaluaciones; // expresiones para el depurador que se muestran automaticamente en cada paso
	ZOCKET zocket;
	
#endif
	
	string sbuffer;
	int delay;
	int port;
	bool do_step;
	
public:
	
	// Linea que se esta ejecutando actualmente
	Intercambio();
	~Intercambio();
	void UnInit();
#ifdef USE_ZOCKETS
	void ProcData(string order);
	void ProcInput();
	void SetDelay(int n);
#endif
	void SetLineNumber(int _l, int _i);
	void SetLineAndInstructionNumber(int _i);
	void SetStarted();
	void SetFinished(bool interrupted=false);
	int GetLineNumber();
	int GetInstNumber();
	bool Running();
	void SetPort(int p);
	
	// Manejo de las lineas del programa en memoria
	int Archivo_Size();
	void AddLine(string s);
	void AddLine(char *s);
	string GetLine(int x);
	
	// Manejo de errores
	int Errores_Size();
	void AddError(string s, int n);
	string GetErrorDesc(int x);
	int GetErrorLine(int x);
	
	bool EvaluatingForDebug();
	void SetError(string error);
};

extern Intercambio Inter;        // clase para enviar informacion de depuración al editor

// *****************************************************************

#endif
