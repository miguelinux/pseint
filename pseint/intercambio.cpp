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
Intercambio::Intercambio(){
	sbuffer="";
#ifdef USE_ZOCKETS
	evaluating_for_debug=false;
	zocket = ZOCKET_ERROR;
#endif
	LineNumber=-1;delay=0; // Inicializar
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
		string res,str="evaluacion ";
		ParseExpression(exp);
		tipo_var tipo;
		if (!is_evaluation_error)
			res = Evaluar(exp,tipo);
		if (is_evaluation_error) 
			str+=evaluation_error+"\n";
		else
			str+=res+"\n";
		zocket_escribir(zocket,str.c_str(),str.size());
		evaluating_for_debug=false;
	} else if (order.substr(0,12)=="autoevaluar ") {
		string exp = order.substr(12);
		evaluating_for_debug=true;
		is_evaluation_error=false;
		ParseExpression(exp);
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
void Intercambio::SetLineNumber(int x){
	LineNumber=x;
#ifdef USE_ZOCKETS
	if (zocket!=ZOCKET_ERROR) { // si estamos depurando, informar la linea y esperar
		string str(string("linea ")+IntToStr(x)+"\n");
		zocket_escribir(zocket,str.c_str(),str.size());
		evaluating_for_debug=true;
		for (unsigned int i=0;i<autoevaluaciones.size();i++) {
			if (autoevaluaciones_valid[i]) {
				stringstream autoevaluacion;
				tipo_var tipo;
				is_evaluation_error=false;
				string res = Evaluar(autoevaluaciones[i],tipo);
//				if (tipo<'c')
//					ExpError(tipo,0,LineNumber);
				if (is_evaluation_error)
					autoevaluacion<<"autoevaluacion "<<i+1<<' '<<evaluation_error<<'\n';
				else
					autoevaluacion<<"autoevaluacion "<<i+1<<' '<<res<<'\n';
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
		if (delay<0)
			zocket_escribir(zocket,"estado pausa\n",13);
		do {
			ProcInput();
		} while (delay<0 && !do_step);
		if (delay>0) Sleep(delay);
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
}

void Intercambio::SetFinished(bool interrupted) {
#ifdef USE_ZOCKETS
	if (zocket!=ZOCKET_ERROR) {
		if (!interrupted)
			zocket_escribir(zocket,"estado finalizado\n",18);
		else
			zocket_escribir(zocket,"estado interrumpido\n",20);
	}
#endif
}
int Intercambio::GetLineNumber() {
	return LineNumber;
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
string Intercambio::GetLine(int x){return Archivo[x];}

// Manejo de errores
int Intercambio::Errores_Size(){return (int)Errores.size();}
void Intercambio::AddError(string s, int n){Errores.push_back(s);Lineas.push_back(n);}
string Intercambio::GetErrorDesc(int x){return Errores[x];}
int Intercambio::GetErrorLine(int x){return Lineas[x];}

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
