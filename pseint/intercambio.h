#ifndef INTERCAMBIO_H
#define INTERCAMBIO_H

#include <string>
#include <vector>
using namespace std;

#ifdef USE_ZOCKETS
#include "zockets.h"
#endif

#ifdef __WIN32__
	#include<windows.h>
#else
	#include<unistd.h>
	#define Sleep(x) usleep((x)*1000)
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
	
	int port; // nro de puerto para comunicarse con wxPSeInt
#endif
	
	string sbuffer;
	int delay; // indica el retardo entre instruccion e instruccion para el pasa a paso continuo
	bool do_continue; // indica si debe continuar la ejecucion  o pausarse
	bool do_one_step; // si do_continue, indica si continuar solo un paso (en cuyo caso do_continue volvera a ser falso)
public:
	bool subtitles_on; // si estamos en paso a paso explicado
	
public:
	
	// Linea que se esta ejecutando actualmente
	Intercambio();
	~Intercambio();
	void UnInit();
#ifdef USE_ZOCKETS
	void ProcData(string order);
	void ProcInput();
	void InitDebug(int _delay); // si _delay!=0 inicializa la ejecución paso a paso enviando el hello y esperando la primer instruccion
	void SetPort(int p);
#endif
	void SetLineAndInstructionNumber(int _i); // define cual es la instruccion que se va a ejecutar a continuacion
	void SendPositionToGUI(); // avisa a la gui en que instruccion va
	void SendIOPositionToTerminal(int argNumber); // avisa a la terminal en que instruccion va
//	void SendLoopPositionToTerminal(); // avisa a la terminal en que instruccion va
	void SendErrorPositionToTerminal(); // avisa a la terminal en que instruccion va
	void ChatWithGUI(); // espera respuesta de la gui para avanzar
	void SendSubtitle(string _str); // envia el texto para el subtitulo a la gui
	
	void SetStarted();
	void SetFinished(bool interrupted=false);
	int GetLineNumber() const { return lineNumber; }
	int GetInstNumber() const { return instNumber; }
	bool Running() const { return running; }
	
	// Manejo de las lineas del programa en memoria
	int Archivo_Size();
	void AddLine(string s);
	void AddLine(char *s);
	
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

#define _sub_msg(i,s) { Inter.SetLineAndInstructionNumber(i); if (Inter.subtitles_on) { Inter.SendPositionToGUI(); Inter.SendSubtitle(s); } }
#define _sub_wait() { if (Inter.subtitles_on) Inter.ChatWithGUI(); }
#define _sub(i,s) { Inter.SetLineAndInstructionNumber(i); if (Inter.subtitles_on) { Inter.SendPositionToGUI(); Inter.SendSubtitle(s); Inter.ChatWithGUI(); } }
#define _pos(i) { Inter.SetLineAndInstructionNumber(i); if (!Inter.subtitles_on) { Inter.SendPositionToGUI(); Inter.ChatWithGUI(); } }
#define _sub_raise() { if (Inter.subtitles_on && for_pseint_terminal) { cout<<"\033[zr"; } }

extern Intercambio Inter;        // clase para enviar informacion de depuración al editor

// *****************************************************************

#endif
