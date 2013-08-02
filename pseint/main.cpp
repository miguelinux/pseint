     // -------------------------------------- //
     //           PSeudo Inerprete             //
     //      http://pseint.sourceforge.net     //
     // -------------------------------------- //

#include "version.h"
#include "zcurlib.h"
#include "common.h"
#include "intercambio.h"
#include "new_memoria.h"

#include "global.h"
#include "utils.h"
#include <csignal>
#include "new_evaluar.h"
#include "SynCheck.h"
#include "Ejecutar.h"
#include "new_funciones.h"
#include "new_programa.h"
#include "case_map.h"
using namespace std;

void on_signal(int s) {
	Inter.SetFinished(true);
	Inter.UnInit();
	exit(s);
}

void checksum(Programa &p) {
	if (p.GetSize()==3) {
		string &s=programa[1].instruccion;
		int n=0,p=1;
		for(unsigned int i=0;i<s.size();i++) { 
			n+=s[i]; p=(p*s[i])%1000000;
		}
		if (n==839 && p==102912) { // daba 730880, por que cambio???
			programa[1].instruccion[9]-=11;
			programa[1].instruccion[10]+=10;
			programa[1].instruccion[11]-=7;
			programa[1].instruccion.insert(12,",\"!\"");
		}
	} else if (p.GetSize()==4) {
		string &s1=programa[1].instruccion;
		string &s2=programa[2].instruccion;
		int n1=0,n2=0,p1=1,p2=1;
		for(unsigned int i=0;i<s1.size();i++) { 
			n1+=s1[i]; p1=(p1*s1[i])%1000000;
		}
		for(unsigned int i=0;i<s2.size();i++) { 
			n2+=s2[i]; p2=(p2*s2[i])%1000000;
		}
		if (n1==619 && p1==956160 && n2==921 && p2==673152) {
			int x[]={3,-9,-15,22,-13,40,44,2,-58,-21,-30,-38,49,-1,47,44,28,0};
			s1+=s1; for(unsigned int i=0;i<s1.size();i++) s1[i]-=x[i];
		}
	}
}

//----------------------------------------------------------------------------
// ************************* Programa Principal ******************************
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {

	signal(2,on_signal);
	signal(6,on_signal);
	signal(9,on_signal);
	signal(15,on_signal);

	if (argc==2 && string(argv[1])=="--install-test") {cout<<"OK";return 0;}
	if (argc==2 && string(argv[1])==string("--version")) {
		_print_version_info("PSeInt");
		exit(0);
	} else if (argc==3 && string(argv[1])==string("--version")) {
		_write_version_info("PSeInt",argv[2]);
		exit(0);
	}

	bool check=true, run=true, user=true, log_file=false, error=false, draw=false, undef_vars=true, var_definition=false;
	int fil_count=0,delay=0; char *fil_args[5];
	for (int i=1;i<argc;i++) {
		if (argv[i][0]=='-') {
			string str(argv[i]);
			if (str.substr(0,8)=="--delay=")
				delay=atoi(str.substr(8,str.size()-8).c_str());
			else if (str.substr(0,8)=="--input=")
				predef_input.push(str.substr(8,str.size()-8));
			else if (str.substr(0,7)=="--port=")
				Inter.SetPort(atoi(str.substr(7,str.size()-7).c_str()));
			else if (str=="--rawerrors")
				raw_errors=true;
			else if (str=="--norun")
				run=false;
			else if (str=="--forcedefinevars")
				var_definition=true;
			else if (str=="--color")
				colored_output=true;
			else if (str=="--nowordoperators")
				word_operators=false;
			else if (str=="--allowconcatenation")
				allow_concatenation=true;
			else if (str=="--overloadequal")
				overload_equal=true;
			else if (str=="--allowddims")
				allow_dinamyc_dimensions=true;
			else if (str=="--basezeroarrays")
				base_zero_arrays=true;
			else if (str=="--nocheck")
				check=false;
			else if (str=="--nouser")
				user=false;
			else if (str=="--noinput")
				noinput=true;
			else if (str=="--nolazysyntax")
				lazy_syntax=false;
			else if (str=="--forceinitvars")
				undef_vars=false;
			else if (str=="--forcesemicolon")
				force_semicolon=true;
			else if (str=="--hidestringfunctions")
				enable_string_functions=false;
			else if (str=="--disableuserfunctions")
				enable_user_functions=false;
			else if (str=="--coloquialconditions")
				coloquial_conditions=word_operators=true;
			else if (str=="--fixwincharset")
				fix_win_charset=true;
			else if (str=="--usecasemap")
				case_map=new map<string,string>();
			else if (str=="--draw") {
				draw=true;
				run=false;
			} else if (str=="--easteregg") {
				cerr<<"Bazinga!"<<endl;
				return 0;
			} else if (str=="--forrealtimesyntax") {
				real_time_syntax=true;
			} else if (str=="--forpseintterminal") {
				for_pseint_terminal=true;
			} else 
				error=true;
		} else {
			fil_args[fil_count++]=argv[i];
		}
	}
	error = error&&((fil_count<(draw?2:1))||(fil_count>(draw?3:2)));
	log_file = fil_count>(draw?2:1);
#ifdef __APPLE__
	wait_key=false;
#else
	wait_key=user&&!for_pseint_terminal;
#endif

	// comprobar parametros
	if (error || (fil_count==0 && !real_time_syntax)) {
		cout<<"Use: pseint FileName.psc [<options>] [LogFile]\n";
		cout<<"     pseint FileName.psc --draw DrawFile.psd [LogFile]\n";
		cout<<" <options> puede ser una o mas de las siguientes:"<<endl;
		cout<<"      --color                utilizar colores para formatear la salida"<<endl;
		cout<<"      --nocheck              no revisar la sintaxis"<<endl;
		cout<<"      --norun                no ejecutar"<<endl;
		cout<<"      --nowordoperators      no permitir utilzar las palabras y,o y no en lugar de &,| y ~"<<endl;
		cout<<"      --nouser               no mostrar mensajes de estado ni esperar un tecla al final"<<endl;
		cout<<"      --forcedefinevars      no permitir utilizar variables sin definir antes su tipo"<<endl;
		cout<<"      --forceinitvars        no permitir utilizar variables sin inicializar"<<endl;
		cout<<"      --forcesemicolon       controlar el uso del punto y coma en las instrucciones secuenciales"<<endl;
		cout<<"      --allowddims           permitir utilizar variables para dimensionar arreglos"<<endl;
		cout<<"      --nolazysyntax         no permitir sintaxis flexible (omitir algunas palabras, o utilizar sinonimos)"<<endl;
		cout<<"      --basezeroarrays       trabajar con arreglos en base 0"<<endl;
		cout<<"      --overloadequal        permitir asignar con ="<<endl;
		cout<<"      --coloquialconditions  permite condiciones en lenguaje coloquial (ej: \"x es par\")"<<endl;
		cout<<"      --delay=<num>          define el retardo entre instrucciones para la ejecucion paso a paso"<<endl;
		cout<<"      --forcevardefinition   obliga a definir explicitamente los tipos de variable"<<endl;
		cout<<"      --hidestringfunctions  deshabilitar funciones para el manejo de cadenas (ej: longitud(x), subcadena(x,i,j))"<<endl;
		cout<<"      --disableuserfunctions no permitir la definición de subprocesos"<<endl;
		cout<<"      --port=<num>           define el puerto tpc para comunicar controlar la depuracion"<<endl;
		cout<<"      --rawerrors            muestra los errores sin descripcion, para testing automatizado"<<endl;
		cout<<"      --noinput              en lugar realizar las lecturas desde el teclado, lo hace desde los argumentos"<<endl;
		cout<<"      --input=<str>          sirve para predefinir uno o más valores de entrada para acciones LEER"<<endl;
		cout<<"      --fixwincharset        corrige la codificación de algunos caracteres para que se muestren correctamente"<<endl;
		cout<<"                             en la consola de Windows"<<endl;
		exit(1);
	}
	
	if (colored_output) { 
#ifndef __APPLE__
		setBackColor(COLOR_BLACK); 
#endif
		clrscr();
	}
	
	if (log_file) {
		ExeInfo.open(fil_args[draw?2:1]);
		if (ExeInfo.is_open()) ExeInfoOn=true;
	}

	// inicializaciones varias
	int errores;
	LoadFunciones();
	srand(time(NULL));
	
	if (real_time_syntax) {
		while (true) {
//			memoria->HardReset();
			programa.HardReset();
			UnloadSubprocesos();
			string line;
			int lcount=0;
			while (true) {
				getline(cin,line); lcount++;
				if (line=="<!{[EXIT]}!>") { UnloadFunciones(); return 0; }
				if (line=="<!{[END_OF_INPUT]}!>") break;
				programa.PushBack(line);
			}
			SynCheck();
			cout<<"<!{[END_OF_OUTPUT]}!>"<<endl;
			map<string,Funcion*>::iterator it1=subprocesos.begin(), it2=subprocesos.end();
			while (it1!=it2) {
				if (it1->first!=main_process_name) cout<<"SUB";
				cout<<"PROCESO "<<it1->first<<":"<<it1->second->userline_start<<':'<<it1->second->userline_end<<endl;
				(it1++)->second->memoria->ListVars();
			}
			cout<<"<!{[END_OF_VARS]}!>"<<endl;
			if (lcount) {
				int n=programa.GetInstSize();
				int *bk=new int[lcount], *st=new int[n+1], stn=0;
				for(int i=0;i<lcount;i++) bk[i]=-1;
				for(int i=0;i<n;i++) { 
					Instruccion &in=programa[i];
					if (
						LeftCompare(in.instruccion,"SI ") ||
						LeftCompare(in.instruccion,"PARA ") ||
						LeftCompare(in.instruccion,"PARACADA ") ||
						LeftCompare(in.instruccion,"MIENTRAS ") ||
						LeftCompare(in.instruccion,"SEGUN ") ||
						in.instruccion=="REPETIR")
							st[stn++]=in.num_linea;
					else if (stn&& (
						in.instruccion=="FINSI" ||
						in.instruccion=="FINMIENTRAS" ||
						in.instruccion=="FINSEGUN" ||
						in.instruccion=="FINPARA" ||
						LeftCompare(in.instruccion,"HASTA QUE ")||
						LeftCompare(in.instruccion,"MIENTRAS QUE ")))
							bk[st[--stn]]=in.num_linea;
				}
				for(int i=0;i<lcount;i++) { 
					if (bk[i]!=-1) cout<<i<<" "<<bk[i]<<endl;
				}
				delete []bk; delete []st;
			}
			cout<<"<!{[END_OF_BLOCKS]}!>"<<endl;
		}
		return 0; // nunca se llega hasta aqui
	}
	
	// Leer el archivo	
	char *filename=fil_args[0];
	ifstream archivo(filename);
	if (!archivo.is_open()) {
		if (colored_output) setForeColor(COLOR_ERROR);
		cout<<"ERROR: No se pudo abrir el archivo \""<<filename<<"\""<<endl;
		if (ExeInfoOn) {
			ExeInfo<<"ERROR: No se pudo abrir el archivo \""<<filename<<"\""<<endl;
			if (user) {
				hideCursor();
				if (wait_key) getKey();
				showCursor();
			}
		}
		exit(1);
	}
	
	if (user && check) {
		show_user_info("Leyendo archivo y comprobando sintaxis...");
	}
	string buffer;
	while (!archivo.eof()) {
		getline(archivo,buffer);
		programa.PushBack(buffer);
		Inter.AddLine(buffer);
	}
	archivo.close();

	// comprobar sintaxis y ejecutar
	errores=SynCheck();
	Inter.InitDebug(delay);
	
	if (for_pseint_terminal) { cout<<"\033[zt"<<main_process_name<<"\n"; }

	//ejecutar
	if (errores==0) {
	// salida para diagrama
		if (draw) {
			if (case_map) CaseMapPurge();
			ofstream dibujo(fil_args[1]);
			for (int i=0;i<programa.GetSize();i++) {
				if (case_map) CaseMapApply(programa[i].instruccion);
				dibujo<<programa[i].instruccion<<endl;
			}
			dibujo.close();
			if (user) show_user_info("Dibujo guardado.");
			ExeInfo.close();
			ExeInfoOn=false;
		} else if (run) {
			allow_undef_vars=undef_vars;
			force_var_definition=var_definition;
			if (ExeInfoOn) if (user) ExeInfo<<"*** Ejecucion Iniciada. ***"<<endl;
			if (user) show_user_info("*** Ejecución Iniciada. ***");
			map<string,Funcion*>::iterator it1=subprocesos.begin(), it2=subprocesos.end();
			while (it1!=it2) (it1++)->second->memoria->FakeReset();
			Inter.SetStarted();
			checksum(programa);
			const Funcion *main_func=EsFuncion(main_process_name,true);
			memoria=main_func->memoria;
			Ejecutar(main_func->line_start);
			Inter.SetFinished();
			if (ExeInfoOn) ExeInfo<<"*** Ejecucion Finalizada. ***";
			if (user) show_user_info("*** Ejecución Finalizada. ***");
		}
	} else {
		if (user) cout<<endl;
		if (errores==1) {
			if (ExeInfoOn) if (user) ExeInfo<<"*** Se encontro 1 error. ***";
			if (user) show_user_info("*** Se encontró 1 error. ***");
		} else {
			if (ExeInfoOn) if (user) ExeInfo<<"*** Se encontraron "<<errores<<" errores. ***";
			if (user) show_user_info("*** Se encontraron ",errores," errores. ***");
		}
	}
	// esperar un enter si es necesario
	if (ExeInfoOn) {   
		ExeInfo.close();
	}
	if (user && !draw) {
		hideCursor();
		if (wait_key) getKey();
		showCursor();
	}

#ifdef _DEBUG	
	UnloadFunciones();
	UnloadSubprocesos();
#endif
	return 0;
}


//--------------------------------------------------------------------------------------

