#ifndef INTERCAMBIO_H
#define INTERCAMBIO_H

#include <string>
#include <vector>
using namespace std;

#ifdef USE_ZOCKETS
#include "zockets.h"
#endif

// *********************** Intercambio ****************************

// estructura auxiliar para guardar datos del backtrace (llamadas a funciones)
struct FrameInfo {
	string func_name;
	int line;
	int instr;
	FrameInfo(string name, int l, int i):func_name(name),line(l),instr(i){}
};

class Intercambio {
	
	int backtraceLevel; // en que nivel del trazado inverso se encuentra el punto actual de ejecucion (1:proceso principal, >1:alguna funcion)
	int debugLevel; // solo interesa depurar si debugLevel<=backtraceLevel (si es 0 depura todo)
	vector<FrameInfo> backtrace;
	
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
	void SetLineAndInstructionNumber(int _i); // avisa a la gui en que linea va
	void ChatWithGUI(); // espera respuesta de la gui para avanzar
	void SendSubtitle(string _str); // envia el texto para el subtitulo a la gui
	
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
	
	void OnFunctionIn(string nom);
	void OnFunctionOut();
	int GetBacktraceLevel();
	FrameInfo GetFrame(int level);
	
};

#define _sub_msg(i,s) { if (subtitles_on) { Inter.SetLineAndInstructionNumber(i); Inter.SendSubtitle(s); } }
#define _sub_wait() { if (subtitles_on) Inter.ChatWithGUI(); }
#define _sub(i,s) { if (subtitles_on) { Inter.SetLineAndInstructionNumber(i); Inter.SendSubtitle(s); Inter.ChatWithGUI(); } }
#define _pos(i) { if (!subtitles_on) { Inter.SetLineAndInstructionNumber(i); Inter.ChatWithGUI(); } }

extern Intercambio Inter;        // clase para enviar informacion de depuración al editor

// *****************************************************************

#endif
