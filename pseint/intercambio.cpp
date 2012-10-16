#include "intercambio.h"
#include "common.h"
#include "new_evaluar.h"
#include "utils.h"
#include "SynCheck.h"
using namespace std;

Intercambio Inter;

string IntToStr(int f);

// *********************** Intercambio ****************************

// Linea que se esta ejecutando actualmente
Intercambio::Intercambio() {
	backtraceLevel=0;
	debugLevel=0;
	sbuffer="";
#ifdef USE_ZOCKETS
	evaluating_for_debug=false;
	zocket = ZOCKET_ERROR;
#endif
	instNumber=lineNumber=-1;delay=0; // Inicializar
	port=24377; running=false;
}
Intercambio::~Intercambio() {
#ifdef USE_ZOCKETS
	if (zocket!=ZOCKET_ERROR) 
		zocket_cerrar (zocket);
#endif
}
void Intercambio::UnInit() {
#ifdef USE_ZOCKETS
	if (zocket!=ZOCKET_ERROR) 
		zocket_cerrar(zocket);
	zocket=ZOCKET_ERROR;
#endif
}
#ifdef USE_ZOCKETS
void Intercambio::ProcData(string order) {
	if (order=="paso" || order=="comenzar") {
		if (order=="paso") {
			if (delay>0) delay=-delay;
			zocket_escribir(zocket,"estado paso\n",12);
		} else
			zocket_escribir(zocket,"estado ejecutando\n",18);
		do_step=true;
	} else if (order=="subtitulos 1") { // se mete en todos los subprocesos
		subtitles_on=true;
	} else if (order=="subtitulos 0") { // no se mete más alla del subproceso actual
		subtitles_on=false;
	} else if (order=="subprocesos 1") { // se mete en todos los subprocesos
		debugLevel=0;
	} else if (order=="subprocesos 0") { // no se mete más alla del subproceso actual
		debugLevel=backtraceLevel;
		if (!debugLevel) debugLevel=1;
	} else if (order=="pausa") {
		delay=-delay;
		if (delay>0)
			zocket_escribir(zocket,"estado ejecutando\n",18);
		else
			zocket_escribir(zocket,"estado pausa\n",13);
	} else if (order.substr(0,6)=="delay ") {
		if (delay<0)
			delay=-atoi(order.substr(6).c_str());
		else
			delay=atoi(order.substr(6).c_str());
	} else if (order.substr(0,8)=="evaluar ") {
		is_evaluation_error=false;
		evaluating_for_debug=true;
		string exp = order.substr(8);
		string res,str="evaluacion x ";
		ParseInspection(exp);
		tipo_var tipo;
		if (!is_evaluation_error)
			res = Evaluar(exp,tipo);
		if (is_evaluation_error) 
			str+=evaluation_error+"\n";
		else {
			if (tipo==vt_numerica)
				res=DblToStr(StrToDbl(res),10);
			str+=res+"\n";
		}
		str[11]='0'+(tipo.cb_log?1:0)+(tipo.cb_num?2:0)+(tipo.cb_car?4:0); // reemplaza la x por el tipo
		zocket_escribir(zocket,str.c_str(),str.size());
		evaluating_for_debug=false;
	} else if (order.substr(0,12)=="autoevaluar ") {
		string exp = order.substr(12);
		evaluating_for_debug=true;
		is_evaluation_error=false;
		ParseInspection(exp);
		if (is_evaluation_error) {
			autoevaluaciones_valid.push_back(false);
			autoevaluaciones.push_back(evaluation_error);
		} else {
			autoevaluaciones_valid.push_back(true);
			autoevaluaciones.push_back(exp);
		}
		evaluating_for_debug=false;
	}
}
#endif

void Intercambio::SendSubtitle(string _str) {
#ifdef USE_ZOCKETS
	if (zocket!=ZOCKET_ERROR && (!debugLevel || backtraceLevel<=debugLevel)) {
		string ss="subtitulo "; ss+=_str+"\n";
		zocket_escribir(zocket,ss.c_str(),ss.size());
	}
#endif
}

void Intercambio::SetLineAndInstructionNumber(int _i) {
	lineNumber=programa[_i].num_linea; instNumber=programa[_i].num_instruccion;
#ifdef USE_ZOCKETS
	if (zocket!=ZOCKET_ERROR && (!debugLevel || backtraceLevel<=debugLevel)) { // si estamos depurando, informar la linea y esperar
		string str;
		if (instNumber>0) str=string("linea ")+IntToStr(lineNumber)+":"+IntToStr(instNumber)+"\n";
		else str=string("linea ")+IntToStr(lineNumber)+"\n";
		zocket_escribir(zocket,str.c_str(),str.size());
	}
#endif
}
		
void Intercambio::ChatWithGUI () {
#ifdef USE_ZOCKETS
	if (zocket!=ZOCKET_ERROR  && (!debugLevel || backtraceLevel<=debugLevel)) {
		evaluating_for_debug=true; string str;
		for (unsigned int i=0;i<autoevaluaciones.size();i++) {
			if (autoevaluaciones_valid[i]) {
				stringstream autoevaluacion;
				tipo_var tipo;
				is_evaluation_error=false;
				string res = Evaluar(autoevaluaciones[i],tipo);
				if (is_evaluation_error)
					autoevaluacion<<"autoevaluacion "<<i+1<<' '<<evaluation_error<<'\n';
				else {
					if (tipo==vt_numerica) res=DblToStr(StrToDbl(res),10);
					autoevaluacion<<"autoevaluacion "<<i+1<<' '<<res<<'\n';
				}
				str = autoevaluacion.str();
				zocket_escribir(zocket,str.c_str(),str.size());
			} else {
				stringstream autoevaluacion;
				autoevaluacion<<"autoevaluacion "<<i+1<<' '<<autoevaluaciones[i]<<'\n';
				str = autoevaluacion.str();
				zocket_escribir(zocket,str.c_str(),str.size());
			}
		}
		evaluating_for_debug=false;
		do_step=false;
		if (delay<0) zocket_escribir(zocket,"estado pausa\n",13);
		do { ProcInput(); } while ((subtitles_on || delay<0) && !do_step);
		if (!do_step && !subtitles_on && delay>0) Sleep(delay);
	}
#endif
}

#ifdef USE_ZOCKETS
void Intercambio::ProcInput() {
	static char buf[256];
	int n=255;
	while (zocket_leer(zocket,buf,n)) {
		if (n>0) {
			char *aux=buf;
			buf[n]='\0';
			for (int i=0;i<n;i++)
				if (buf[i]=='\n' || buf[i]=='\r') {
					buf[i]='\0';
					sbuffer+=aux;
					if (sbuffer.size())
						ProcData(sbuffer);
					sbuffer="";
					aux=buf+i+1;
				}
			if (aux<buf+n)
					sbuffer+=aux;
		}
		n=255;
	}
}
#endif

void Intercambio::SetStarted() {
	running=true;
	backtraceLevel=0;
	backtrace.clear();
}

void Intercambio::SetFinished(bool interrupted) {
	backtraceLevel=0;
#ifdef USE_ZOCKETS
	if (zocket!=ZOCKET_ERROR) {
		if (!interrupted)
			zocket_escribir(zocket,"estado finalizado\n",18);
		else
			zocket_escribir(zocket,"estado interrumpido\n",20);
	}
#endif
	running=false;
}
int Intercambio::GetLineNumber() {
	return lineNumber;
}

int Intercambio::GetInstNumber() {
	return instNumber;
}

bool Intercambio::Running() {
	return running;
}

void Intercambio::SetPort(int p) { 
	port = p; 
}

#ifdef USE_ZOCKETS
void Intercambio::SetDelay(int n) { 
	if ( (delay=n) ) {
		//			zocket = zocket_esperar_llamada(port);
		zocket = zocket_llamar(port);
		if (zocket==ZOCKET_ERROR) 
			cerr<<"<<Error al inicializar ejecucion paso a paso>>"<<endl;
		else {
			zocket_escribir(zocket,"estado inicializado\n",20);
			while (!do_step) ProcInput();
			zocket_escribir(zocket,"estado ejecutando\n",18);
		}
	}
}
#endif

// Manejo de las lineas del programa en memoria
int Intercambio::Archivo_Size(){return (int)Archivo.size();}
void Intercambio::AddLine(string s){Archivo.push_back(s);}
void Intercambio::AddLine(char *s){Archivo.push_back(s);}
//string Intercambio::GetLine(int x){return Archivo[x];}

// Manejo de errores
//int Intercambio::Errores_Size(){return (int)Errores.size();}
//void Intercambio::AddError(string s, int n){Errores.push_back(s);Lineas.push_back(n);}
//string Intercambio::GetErrorDesc(int x){return Errores[x];}
//int Intercambio::GetErrorLine(int x){return Lineas[x];}

// *****************************************************************

bool Intercambio::EvaluatingForDebug() {
#ifdef USE_ZOCKETS
	return evaluating_for_debug;
#else
	return false;
#endif
}

void Intercambio::SetError(string error) {
#ifdef USE_ZOCKETS
	is_evaluation_error = true;
	evaluation_error = error;
#endif
}

void Intercambio::OnFunctionIn(string nom) {
	backtrace.push_back(nom);
	backtraceLevel++;
#ifdef USE_ZOCKETS
	if (zocket==ZOCKET_ERROR || backtrace.empty()) return;
	string s="proceso "+IntToStr(backtraceLevel)+":"+backtrace.back()+"\n";
	zocket_escribir(zocket,s.c_str(),s.size());
#endif	
}
void Intercambio::OnFunctionOut() {
	backtrace.pop_back();
	if (backtraceLevel==debugLevel) debugLevel--;
	backtraceLevel--;
#ifdef USE_ZOCKETS
	if (zocket==ZOCKET_ERROR || backtrace.empty()) return;
	string s="proceso "+IntToStr(backtraceLevel)+":"+backtrace.back()+"\n";
	zocket_escribir(zocket,s.c_str(),s.size());
#endif
}

