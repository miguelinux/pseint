#include "Ejecutar.h"
#include <string>
#include "global.h"
#include "intercambio.h"
#include "utils.h"
#include <iostream>
#include "new_evaluar.h"
#include "new_memoria.h"
#include "zcurlib.h"
#include "new_programa.h"
#include "new_funciones.h"
using namespace std;

// ********************* Ejecutar un Bloque de Instrucciones **************************
void Ejecutar(int LineStart, int LineEnd) {
	// variables auxiliares
	string aux1,aux2;
//	char tipo;
	tipo_var tipo;
	int tmp1,tmp2,tmp3;
	string cadena;
	// Ejecutar el bloque
//	for (int line=LineStart;line<=LineEnd;line++) {
	int line=LineStart-1;
	while (true) {
		line++;
		if (LineEnd!=-1 && line>LineEnd) break; 
		cadena=programa[line].instruccion;
		if (cadena=="FINPROCESO" || cadena=="FINSUBPROCESO") {
			_pos(line);
			if (cadena=="FINSUBPROCESO") {
				_sub(line,string("Se sale del subproceso ")+cadena);
				Inter.OnFunctionOut();
			} else {
				_sub(line,"Finaliza el algoritmo");
			}
			break;
		}
		if (LeftCompare(cadena,"PROCESO ") || LeftCompare(cadena,"SUBPROCESO ")) {
			bool es_proc=LeftCompare(cadena,"PROCESO ");
			size_t p=cadena.find(' '); cadena=cadena.substr(p);
			p=cadena.find('<'); if (p==string::npos) p=cadena.find('='); else p++;
			if (p==string::npos) p=0; else p++; cadena=cadena.substr(p);
			p=cadena.find('('); if (p!=string::npos) cadena=cadena.substr(0,p);
			Inter.OnFunctionIn(cadena);
			_pos(line);
			_sub(line,string(es_proc?"El algoritmo comienza con el proceso ":"Se ingresa en el subproceso ")+cadena);
			continue;
		}
		if (cadena[cadena.size()-1]==';') { // Si es una accion secuencial
			_pos(line);
			if (cadena=="BORRARPANTALLA;") {
				if (for_test) cout<<"***LimpiarPantalla***"<<endl; else { clrscr(); gotoXY(1,1); }
				_sub(line,"Se borra la pantalla");
			} else if (cadena=="ESPERARTECLA;") {
				_sub_msg(line,"Se espera a que el usuario presione una tecla.");
				_sub_raise();
				if (for_test) cout<<"***EsperarTecla***"<<endl; else getKey();
				_sub_wait();
			} else if (LeftCompare(cadena,"INVOCAR ")) {
				string llamada=cadena.substr(8); llamada.erase(llamada.length()-1,1); // cortar el "invocar" y el ";"
				tipo=vt_desconocido; size_t p=llamada.find('(',0);
				_sub(line,string("Se va a invocar al subproceso")+llamada.substr(0,p));
				if (p==string::npos)
					EvaluarFuncion(EsFuncion(llamada),"()",tipo,false);
				else
					EvaluarFuncion(EsFuncion(llamada.substr(0,p)),llamada.substr(p),tipo,false);
				// ----------- ESCRIBIR ------------ //
			} else if (LeftCompare(cadena,"ESCRIBIR ") || LeftCompare(cadena,"ESCRIBNL ")) {
				bool saltar=LeftCompare(cadena,"ESCRIBIR ");
				cadena.erase(0,9);
				cadena.erase(cadena.size()-1,1);
				tmp1=-1;tmp2=0;tmp3=0;
				// Separar parametros
				while (tmp3<(int)cadena.size()) {
					while (tmp3<(int)cadena.size() && !(tmp1<0 && tmp2==0 && cadena[tmp3]==',')) {
						if (cadena[tmp3]=='\'') tmp1=-tmp1;
						if (tmp1<0) {
							if (cadena[tmp3]=='(') tmp2++;
							if (cadena[tmp3]==')') tmp2--;
						}
						tmp3++;
					}
					aux1=cadena;
					aux1.erase(tmp3,aux1.size()-tmp3);
					tmp3-=aux1.size();
					cadena.erase(0,aux1.size()+1);
					
					if (colored_output) setForeColor(COLOR_OUTPUT);
					if (with_io_references) Inter.SendPositionToTerminal();
					tipo_var x;
					_sub(line,string("Se evalúa la expresion: ")+aux1);
					aux1=Evaluar(aux1,x);
					if (x!=vt_error) {
						if (x==vt_numerica)
							aux1=DblToStr(StrToDbl(aux1),true);
						else
							fixwincharset(aux1);
						cout<<aux1<<flush; // Si es variable, muestra el contenido
						_sub(line,string("Se muestra en pantalla el resultado: ")+aux1);
					}
				}
				if (saltar) cout<<endl; else cout<<flush;
			} else 
			// ------------- LEER --------------- //
			if (LeftCompare(cadena,"LEER ")) {
				cadena.erase(0,5);
				cadena.erase(cadena.size()-1,1);
				tmp1=0; tmp2=0;
				while (tmp2<(int)cadena.size()) {
					while (tmp2<(int)cadena.size() && !(tmp1==0 && cadena[tmp2]==',')) {
						tmp2++;
						if (cadena[tmp2]=='(') tmp1++;
						if (cadena[tmp2]==')') tmp1--;
					}
					// Cortar el nombre de la variable a leer
					aux2=cadena;
					aux2.erase(tmp2,cadena.size()-tmp2);
					cadena.erase(0,aux2.size()+1);
					tmp2-=aux2.size();

					if (lang[LS_FORCE_DEFINE_VARS] && !memoria->EstaDefinida(aux2)) {
						ExeError(208,"Variable no definida ("+aux2+").");
					}
					tipo=memoria->LeerTipo(aux2);
					const int *dims=memoria->LeerDims(aux2);
					size_t pp=aux2.find("(");
					if (dims && pp==string::npos)
						ExeError(200,"Faltan subindices para el arreglo ("+aux2+").");
					else if (!dims && pp!=string::npos)
						ExeError(201,"La variable ("+aux2.substr(0,pp)+") no es un arreglo.");
					if (dims) {
						_sub(line,string("Se alizan las dimensiones de ")+aux2);
						CheckDims(aux2);
						_sub(line,string("El resultado es ")+aux2);
					}
					
					if (with_io_references) Inter.SendPositionToTerminal();
					if (colored_output) setForeColor(COLOR_INFO);
					cout<<"> "<<flush;
					if (colored_output) setForeColor(COLOR_INPUT);
					// Leer dato
					_sub_msg(line,"Se espera a que el usuario ingrese un valor y presiones enter."); // tipo?
					_sub_raise();
					
					if (!predef_input.empty() || noinput) {
						if (predef_input.empty()) ExeError(214,"Sin entradas disponibles.");
						aux1=predef_input.front(); predef_input.pop(); cout<<aux1<<endl;
						_sub_wait();
					} else {
						aux1=getLine();  
						if (for_eval) cout<<aux1<<endl; // la entrada en psEval es un stream separado de la salida, entonces la reproducimos alli para que la salida contenga todo el "dialogo"
					}
					
					fixwincharset(aux1,true); // "descorrige" para que al corregir no traiga problemas
					
					string auxup=ToUpper(aux1);
					if (auxup=="VERDADERO" || auxup=="FALSO") aux1=auxup;
					
					if (tipo==vt_logica && aux1.size()==1 && (toupper(aux1[0])=='F'||aux1[0]=='0')) aux1=FALSO;
					if (tipo==vt_logica && aux1.size()==1 && (toupper(aux1[0])=='V'||aux1[0]=='1')) aux1=VERDADERO;
					tipo_var tipo2 = GuestTipo(aux1);
					if (!tipo.set(tipo2)) 
						ExeError(120,string("No coinciden los tipos (")+aux2+").");
					else if (tipo==vt_numerica_entera && tipo.rounded && aux1.find(".",0)!=string::npos)
						ExeError(313,string("No coinciden los tipos (")+aux2+"), el valor ingresado debe ser un entero.");
					_sub(line,string("El valor ingresado se almacena en ")+aux2); // tipo?
					memoria->DefinirTipo(aux2,tipo);
					memoria->EscribirValor(aux2,aux1);
//						EscribirVar(aux2,tipo,aux1); // Escribir el Dato en Memoria
				}
			} else 
					// ------------- DIMENSION --------------- //
			if (LeftCompare(cadena,"DIMENSION ")) {
				do {
					string otro=""; // por si hay mas de un arreglo
					cadena.erase(0,10);
					tmp1=cadena.find("(",0);  // Cortar nombre del arreglo
					aux1=cadena; aux1.erase(tmp1,aux1.size()-tmp1);
					aux2=cadena; // cortar indices
					aux2.erase(0,aux1.size()+1); aux2.erase(aux2.size()-2,2);
					// Separar indices
					tmp2=0; tmp1=0;
					int *dim; tmp3=0; // arreglo para las dimensiones
					while (tmp1<(int)aux2.size()) {
						while (tmp1<(int)aux2.size() && !(tmp2==0 && aux2[tmp1]==',')) {
							tmp1++;
							if (aux2[tmp1]=='(') tmp2++;
							if (aux2[tmp1]==')') tmp2--;
							if (tmp2<0) break;
						}
						if (tmp2<0) {
							otro=aux2;
							otro.erase(0,tmp1+2);
							otro="DIMENSION "+otro+");";
							aux2.erase(tmp1,aux2.size()-tmp1);
							aux2=aux2+",";
							tmp3++;
							break;
						}
						tmp1++; tmp3++;
					}
					dim=new int[tmp3+1]; dim[0]=tmp3;
					int last=0;tmp3=1; tmp1=0; tmp2=0;
					if (lang[LS_ALLOW_DINAMYC_DIMENSIONS]) { _sub(line,string("Se evalúan las expresiones para cada dimensión del arreglo ")+aux1); }
					while (tmp1<(int)aux2.size()) {
						while (tmp1<(int)aux2.size() && !(tmp2==0 && aux2[tmp1]==',')) {
							tmp1++;
							if (aux2[tmp1]=='(') tmp2++;
							if (aux2[tmp1]==')') tmp2--;
						}
						cadena=aux2; // Cortar la expresion indice
						cadena.erase(tmp1,cadena.size()-tmp1); cadena.erase(0,last);
						cadena=Evaluar(cadena,tipo);
						if (!tipo.cb_num) // Controlar tipo
	//								if (tipo<'c')
	//									ExpError(tipo,1);
	//								else
								ExeError(122,"No coinciden los tipos.");
						dim[tmp3]=(int)StrToDbl(cadena);
						if (dim[tmp3]<=0) {
							ExeError(274,"Las dimensiones deben ser mayores a 0.");
						}
						tmp3++; last=tmp1+1; tmp1++;
					}
					if (Inter.subtitles_on) {
						string tamanio;
						for(int i=1;i<=dim[0];i++) tamanio+="x"+IntToStr(dim[i]);
						tamanio[0]=' ';
						_sub(line,string("Se crea el arreglo ")+aux1+" de"+tamanio+" elementos");
					}
					if (memoria->HaSidoUsada(aux1)||memoria->LeerDims(aux1))
						ExeError(123,"Identificador en uso.");
					if (dim!=0) memoria->AgregarArreglo(aux1, dim);
					if (otro!="") cadena=otro; else cadena="";
				} while (cadena.size());
			} else
			// ------------- DEFINICION --------------- //
			if (LeftCompare(cadena,"DEFINIR ")) {
				tmp1=0; tmp2=0; cadena.erase(0,8); bool rounded=false;
				if (RightCompare(cadena," COMO LOGICO;")) { tipo=vt_logica; aux1="FALSO"; cadena.erase(cadena.size()-13,13); }
				else if (RightCompare(cadena," COMO REAL;")) { tipo=vt_numerica; aux1="0"; cadena.erase(cadena.size()-11,11); }
				else if (RightCompare(cadena," COMO ENTERO;")) { tipo=vt_numerica; aux1="0"; cadena.erase(cadena.size()-13,13); rounded=true; }
				else if (RightCompare(cadena," COMO CARACTER;")) { tipo=vt_caracter; aux1=""; cadena.erase(cadena.size()-15,15); }
				while (tmp2<(int)cadena.size()) {
					while (tmp2<(int)cadena.size() && !(tmp1==0 && cadena[tmp2]==',')) {
						tmp2++;
						if (cadena[tmp2]=='(') tmp1++;
						if (cadena[tmp2]==')') tmp1--;
					}
					// Cortar el nombre de la variable a leer
					aux2=cadena;
					aux2.erase(tmp2,cadena.size()-tmp2);
					cadena.erase(0,aux2.size()+1);
					tmp2-=aux2.size();
					if (memoria->EstaDefinida(aux2) || memoria->EstaInicializada(aux2)) 
						ExeError(124,string("La variable (")+aux2+") ya estaba definida.");
					memoria->DefinirTipo(aux2,tipo,rounded);
					if (tipo==vt_numerica_entera) {
						_sub(line,string("Se define el tipo de la variable \"")+aux2+"\" como Numérico(Entero).");
					} else if (tipo==vt_numerica) {
						_sub(line,string("Se define el tipo de la variable \"")+aux2+"\" como Numérico(Real).");
					} else if (tipo==vt_caracter) {
						_sub(line,string("Se define el tipo de la variable \"")+aux2+"\" como Caracter/Cadena de Caracteres.");
					} else if (tipo==vt_logica) {
						_sub(line,string("Se define el tipo de la variable \"")+aux2+"\" como Lógico.");
					}
				}
			} else
			// ------------- ESPERAR un tiempo --------------- //
			if (LeftCompare(cadena,"ESPERAR ")) {
				aux2=cadena.substr(8); 
				int factor=1;
				if (RightCompare(aux2," SEGUNDO;")) { factor=1000; aux2.erase(aux2.size()-9); }
				else if (RightCompare(aux2," SEGUNDOS;")) { factor=1000; aux2.erase(aux2.size()-10); }
				if (RightCompare(aux2," MILISEGUNDO;")) { factor=1; aux2.erase(aux2.size()-13); }
				else if (RightCompare(aux2," MILISEGUNDOS;")) { factor=1; aux2.erase(aux2.size()-14); }
				_sub(line,string("Se evalúa la cantidad de tiempo: ")+aux2);
				aux2=Evaluar(aux2,tipo);
				if (!tipo.cb_num) ExeError(219,string("La longitud del intervalo debe ser numérica."));
				else {
					_sub(line,string("Se esperan ")+aux2+(factor==1?" milisengudos":" segundos"));
					if (!Inter.subtitles_on) Sleep(int(StrToDbl(aux2)*factor));
				}
			} else {
				// ------------- ASIGNACION --------------- //
				// separar variable y expresion en aux1 y aux2
				tmp1=cadena.find("<-",0);
				aux1=cadena.substr(0,tmp1);  
				aux2=cadena.substr(tmp1+3,cadena.size()-tmp1-5); // ignorar flecha, punto y como y parentesis extras
				if (lang[LS_FORCE_DEFINE_VARS] && !memoria->EstaDefinida(aux1)) {
					ExeError(211,string("La variable (")+aux1+") no esta definida.");
				}
				// verificar indices si es arreglo
				if (memoria->LeerDims(aux1)) {
					if (aux1.find("(",0)==string::npos)
						ExeError(200,"Faltan subindices para el arreglo ("+aux1+").");
					else
						CheckDims(aux1);
				} else if (aux1.find("(",0)!=string::npos) {
					ExeError(201,"La variable ("+aux1.substr(0,aux1.find("(",0))+") no es un arreglo.");
				}
				// evaluar expresion
				_sub(line,string("Se evalúa la expresion a asignar: ")+aux2);
				aux2=Evaluar(aux2,tipo);
				// comprobar tipos
				tipo_var tipo_aux1 = memoria->LeerTipo(aux1);
				if (!tipo_aux1.can_be(tipo))
					ExeError(125,"No coinciden los tipos.");
				else if (tipo_aux1==vt_numerica_entera && tipo_aux1.rounded && aux2.find(".")!=string::npos)
					ExeError(314,"No coinciden los tipos, el valor a asignar debe ser un entero.");
				_sub(line,string("El resultado es: ")+aux2);
				// escribir en memoria
				_sub(line,string("El resultado se guarda en ")+aux1);
				tipo.rounded=false; // no forzar a entera la variable en la que se asigna
				memoria->DefinirTipo(aux1,tipo);
				memoria->EscribirValor(aux1,aux2);
			}
		} else { // Si no es secuencial
			// ---------------- SI ------------------ //
			if (LeftCompare(cadena,"SI ")) {
				cadena.erase(0,3);
				_pos(line);
				_sub(line,string("Se evalúa la condicion para Si-Entonces: ")+cadena);
				aux1=Evaluar(cadena,tipo,vt_logica);
				if (tipo!=vt_error) {
					if (aux1=="VERDADERO") {
						_sub(line+1,"El resultado es Verdadero, se sigue por la rama del Entonces");
						tmp1=line+2; tmp2=0; // Buscar hasta donde llega el verdadero
						while (!(tmp2==0 && (programa[tmp1]=="SINO" || programa[tmp1]=="FINSI"))) {
							// Saltear bucles anidados
							if (LeftCompare(programa[tmp1],"SI ")) tmp2++;
							if (programa[tmp1]=="FINSI") tmp2--;
							tmp1++;
						}
						Ejecutar(line+2,tmp1-1); // ejecutar salida por verdadero
						while (!(tmp2==0 &&  (LeftCompare(programa[tmp1],"FINSI")))) {
							// Buscar salida del bucle
							if (LeftCompare(programa[tmp1],"SI ")) tmp2++;
							if (programa[tmp1]=="FINSI") tmp2--;
							tmp1++;
						}
						line=tmp1;
					} else {
						tmp1=line+1; tmp2=0; // Buscar hasta donde llega el verdadero
						while (!(tmp2==0 && (programa[tmp1]=="SINO" || programa[tmp1]=="FINSI"))) {
							// Saltear bucles anidados
							if (LeftCompare(programa[tmp1],"SI ")) tmp2++;
							if (programa[tmp1]=="FINSI") tmp2--;
							tmp1++;
						}
						if (programa[tmp1]=="SINO") {
							line=tmp1; // Buscar hasta donde llega el falso
							while (!(tmp2==0 && programa[tmp1]=="FINSI")) {
								if (LeftCompare(programa[tmp1],"SI ")) tmp2++;
								if (LeftCompare(programa[tmp1],"FINSI")) tmp2--;
								tmp1++;
							}
							_pos(line);
							_sub(line,"El resultado es Falso, se sigue por la rama del Sino");
							Ejecutar(line+1,tmp1-1); // ejecutar salida por falso
						} else {
							_sub(line,"El resultado es Falso, no se hace nada");
						}
						line=tmp1;
					}
					_pos(line);
					_sub(line,"Se sale de la estructura Si-Entonces");
				} else {
					ExeError(275,"No coinciden los tipos.");
				}
			} else 
			// ---------------- MIENTRAS ------------------ //
			if (LeftCompare(cadena,"MIENTRAS ")) {
				cadena.erase(0,9);
				cadena.erase(cadena.size()-6,6);
				_pos(line);
				_sub(line,string("Se evalúa la condicion para Mientras: ")+cadena);
				aux1=Evaluar(cadena,tipo);
				if (tipo!=vt_error) {
					tmp1=line+1; tmp2=0; // Buscar hasta donde llega el bucle
					while (!(tmp2==0 && programa[tmp1]=="FINMIENTRAS")) {
						// Saltear bucles anidados
						if (LeftCompare(programa[tmp1],"MIENTRAS ")) tmp2++;
						if (programa[tmp1]=="FINMIENTRAS") tmp2--;
						tmp1++;
					}
					while (aux1=="VERDADERO") {
						_sub(line,"La condicion es Verdadera, se iniciará una iteración.");
						Ejecutar(line+1,tmp1-1);
						_pos(line);
						_sub(line,string("Se evalúa nuevamente la condicion: ")+cadena);
						aux1=Evaluar(cadena,tipo);
					}
					line=tmp1;
					_pos(line);
					_sub(line,"La condicion es Falsa, se sale de la estructura Mientras.");
				}
			} else 
			// ---------------- REPETIR HASTA QUE ------------------ //
			if (cadena=="REPETIR") {
				_pos(line);
				tmp1=line+1; tmp2=0; // Buscar hasta donde llega el bucle
				while (!(tmp2==0 && (LeftCompare(programa[tmp1],"HASTA QUE ")||LeftCompare(programa[tmp1],"MIENTRAS QUE ")))) {
					// Saltear bucles anidados
					if (programa[tmp1]=="REPETIR") tmp2++;
					else if (LeftCompare(programa[tmp1],"HASTA QUE ")) tmp2--;
					else if (LeftCompare(programa[tmp1],"MIENTRAS QUE ")) tmp2--;
					tmp1++;
				}
				// cortar condicion de cierre
				cadena=programa[tmp1];
				string valor_verdad;
				if (LeftCompare(cadena,"HASTA QUE ")){
					cadena.erase(0,10);
					valor_verdad="FALSO";
				} else {
					cadena.erase(0,13);
					valor_verdad="VERDADERO";
				}
				_sub(line,"Se ejecutaran las acciones contenidas en la estructura Repetir");
				do {
					Ejecutar(line+1,tmp1-1);
					// evaluar condicion y seguir
					_pos(tmp1);
					_sub(tmp1,string("Se evalúa la condicion: ")+cadena);
					aux1=Evaluar(cadena,tipo);
//					if (tipo==vt_error) ???
					if (aux1==valor_verdad) 
						_sub(tmp1,string("La condicion es ")+Evaluar(cadena,tipo)+", se sale de la estructura Repetir.");
				} while (aux1==valor_verdad);
				line=tmp1;
				_sub(tmp1,string("La condicion es ")+Evaluar(cadena,tipo)+", se sale de la estructura Repetir.");
			} else 
			// ------------------- PARA --------------------- //
			if (LeftCompare(cadena,"PARA ")) {
				_pos(line);
				bool positivo; // para saber si es positivo o negativo
				cadena.erase(0,5); // saca "PARA "
				cadena.erase(cadena.size()-6,6); // saca " HACER"
				// corta condicion
				tmp1=cadena.find(" HASTA ");
				aux1=cadena.substr(0,tmp1);
				cadena.erase(0,tmp1+7); // saca la asignacion inicial
				tmp1=aux1.find("<-",0);
				string contador=aux1.substr(0,tmp1); // variable del para
				memoria->DefinirTipo(aux1,vt_numerica);
				string val_ini=aux1.substr(tmp1+2); // valor inicial
				_sub(line,string("Se evalúa la expresion para el valor inicial: ")+val_ini);
				val_ini=Evaluar(val_ini,tipo);
				if (!memoria->LeerTipo(aux1).cb_num || !tipo.cb_num)
					ExeError(126,"No coinciden los tipos.");
				
				string val_fin,val_paso;
				size_t pos_paso=cadena.find(" CON PASO ",0); // busca el paso
				if (pos_paso!=string::npos) { // si hay paso tomar ese
					val_fin=cadena.substr(0,pos_paso);
					if (RightCompare(val_fin," HACER")) // por si ponen "...HASTA 5 HACER CON PASO 3 HACER" (2 HACER)
						val_fin=val_fin.substr(0,val_fin.size()-6);
					val_paso=cadena.substr(pos_paso+10);
					_sub(line,string("Se evalúa la expresion para el paso: ")+val_ini);
					positivo=(Evaluar(val_paso+">=0",tipo)=="VERDADERO");
				} else { // si no hay paso adivinar
					val_fin=cadena;
					if (lang[LS_DEDUCE_NEGATIVE_FOR_STEP] && Evaluar(val_fin+"<"+val_ini,tipo)=="VERDADERO") {
						_sub(line,"Se determina que el paso será -1.");
						positivo=false; val_paso="-1";
					} else {
						_sub(line,"Se determina que el paso será +1.");
						positivo=true; val_paso="1";
					}
				}
				
				// Buscar hasta donde llega el bucle
				tmp1=line+1; tmp2=0;
				while (!(tmp2==0 && LeftCompare(programa[tmp1],"FINPARA"))) {
					// Saltear bucles anidados
					if (LeftCompare(programa[tmp1],"PARA ")||LeftCompare(programa[tmp1],"PARACADA ")) tmp2++;
					if (programa[tmp1]=="FINPARA") tmp2--;
					tmp1++;
				}
				
				_sub(line,string("Se inicializar el contador ")+contador+" en "+val_ini);
				memoria->EscribirValor(contador,val_ini); // inicializa el contador
				string comp=positivo?"<=":">=";
				_sub(line,string("Se compara el contador con el valor final: ")+contador+"<="+val_fin);
				while (Evaluar(contador+comp+val_fin,tipo)=="VERDADERO") {
					_sub(line,"La expresión fue Verdadera, se iniciará una iteración.");
					Ejecutar(line+1,tmp1-1);
					_pos(line);
					memoria->EscribirValor(contador,aux1=Evaluar(contador+"+("+val_paso+")",tipo));
					_sub(line,string("Se actualiza el contador, ahora ")+contador+" vale "+aux1+".");
					_sub(line,string("Se compara el contador con el valor final: ")+contador+comp+val_fin);
				}
				line=tmp1;
				_pos(line);
				_sub(line,"Se sale de la estructura repetitiva Para.");
			} else 
			// ------------------- PARA CADA --------------------- //
			if (LeftCompare(cadena,"PARACADA ")) {
				bool primer_iteracion=true; _pos(line);
				cadena.erase(0,9); // borrar "PARACADA "
				cadena.erase(cadena.size()-6,6); // borrar " HACER"
				tmp1=cadena.find(" ");
				tmp2=cadena.find(" ",tmp1+4);
				aux1=cadena.substr(0,tmp1); // indetificador para el elemento
				aux2=cadena.substr(tmp1+4,tmp2-tmp1-3); // identificador del arreglo
				
				tmp1=line+1; tmp2=0; // Buscar hasta donde llega el bucle
				while (!(tmp2==0 && LeftCompare(programa[tmp1],"FINPARA"))) {
					// Saltear bucles anidados
					if (LeftCompare(programa[tmp1],"PARA ")||LeftCompare(programa[tmp1],"PARACADA ")) tmp2++;
					if (programa[tmp1]=="FINPARA") tmp2--;
					tmp1++;
				}

				const int *dims=memoria->LeerDims(aux2);
				if (!dims) ExeError(276,"La variable ("+aux2+") no es un arreglo.");
				int nelems=1; // cantidad total de iteraciones
				for (int i=1;i<=dims[0];i++) nelems*=dims[i];
				
				// bucle posta
				_sub(line,string("El arreglo \"")+aux2+"\" contiene "+IntToStr(nelems)+" elementos. Se comienza a iterar por ellos.");
				for (int i=0;i<nelems;i++) {
					// armar expresion del elemento (ej: A[1])
					string elemento=")";
					int naux=1;
					for (int j=dims[0];j>0;j--) {
						elemento=string(",")+IntToStr((lang[LS_BASE_ZERO_ARRAYS]?0:1)+((i/naux)%dims[j]))+elemento;
						naux*=dims[j];
					}
					elemento=aux2+"("+elemento.substr(1);
					// asignar el elemento en la variable del bucle
					if (primer_iteracion) primer_iteracion=false; else { _pos(line); }
					_sub(line,aux1+" será equivalente a "+elemento+" en esta iteración.");
					if (!memoria->DefinirTipo(aux1,memoria->LeerTipo(elemento)))
						ExeError(277,"No coinciden los tipos.");
					memoria->EscribirValor(aux1,memoria->LeerValor(elemento));
					// ejecutar la iteracion
					Ejecutar(line+1,tmp1-1);
					// asignar la variable del bucle en el elemento
					memoria->DefinirTipo(aux1,memoria->LeerTipo(elemento));
					memoria->EscribirValor(elemento,memoria->LeerValor(aux1));
				}
				line=tmp1; // linea del finpara
				_pos(line);
				_sub(line,"Se sale de la estructura repetitiva Para Cada.");
				
			} else 
			// ------------------- SEGUN --------------------- //
			if (LeftCompare(cadena,"SEGUN ")) {
				int fin;
				cadena.erase(0,6); cadena.erase(cadena.size()-6,6); // Cortar la variable
				tipo_var tipo_master=vt_caracter_o_numerica;
				_pos(line);
				_sub(line,string("Se evalúa la expresion: ")+cadena);
				Evaluar(aux2=cadena,tipo,tipo_master); // evaluar para verificar el tipo
				if (!tipo.cb_num&&(lang[LS_INTEGER_ONLY_SWITCH]||!tipo.cb_car)) {
					if (!lang[LS_INTEGER_ONLY_SWITCH]) 
						ExeError(205,"La expresión del SEGUN debe ser de tipo numerica o caracter.");
					else
						ExeError(206,"La expresión del SEGUN debe ser numerica.");
				}
				_sub(line,string("El resultado es: ")+Evaluar(cadena,tipo,tipo_master));
				tmp1=line+1; tmp2=0; // Buscar hasta donde llega el bucle
				while (!(tmp2==0 && LeftCompare(programa[tmp1],"FINSEGUN"))) {
					// Saltear bucles anidados
					if (LeftCompare(programa[tmp1],"SEGUN ")) tmp2++;
					if (programa[tmp1]=="FINSEGUN") tmp2--;
					tmp1++;
				}
				fin=tmp1-1;
				int x, flag_coincide=0, anid=0;
				string aux3;
				for (x=line+1;x<fin && flag_coincide==0;x++) {
					cadena=programa[x];
					if (LeftCompare(cadena,"SEGUN ")) anid++;
					if (cadena=="FINSEGUN") anid--;
					if (cadena[cadena.size()-1]==':' && anid==0) {
						if (cadena=="DE OTRO MODO:") { 
							_pos(x);
							_sub(x,"Se ingresará en la opción De Otro Modo");
							flag_coincide=1; break;
						}
						else {
							cadena[cadena.size()-1]=',';
							tmp2=cadena.find(",",0);
							while (tmp2<=(int)cadena.size() && tmp2>=0) {
								// evaluar el parametro para verificar el tipo
								aux3=cadena.substr(0,tmp2);
								cadena.erase(0,aux3.size()+1);
								_pos(x);
								_sub(x,string("Se evalúa la opcion: ")+aux3);
								aux1=Evaluar(aux3,tipo,tipo_master);
								if (!tipo.cb_num&&(lang[LS_INTEGER_ONLY_SWITCH]||!tipo.cb_car)) ExeError(127,"No coinciden los tipos.");
								// evaluar la condicion (se pone como estaban y no los resultados de la evaluaciones de antes porque sino las variables indefinida pueden no tomar el valor que corresponde
								if (Evaluar(aux3+"="+aux2,tipo)==VERDADERO) {
									_sub(x,"El resultado coincide, se ingresará en esta opción.");
									cadena=aux1;flag_coincide=1;x--;break;
								} else {
									_sub(x,string("El resultado no coincide: ")+aux1);
								}
//								if (tipo==vt_error) ExeError(127,"No coinciden los tipos."); // no parece hacer falta
								tmp2=cadena.find(",",0);
							}
						}
					}
				}
				if (x<tmp1) { // si coincide, buscar el final del bucle
					tmp1=x+1; tmp2=0; // Buscar hasta donde llega el bucle
					while (!(tmp2==0 && ((programa[tmp1]=="FINSEGUN") || (RightCompare(programa[tmp1],":"))) )) {
						// Saltear bucles anidados
						if (LeftCompare(programa[tmp1],"SEGUN ")) tmp2++;
						if (programa[tmp1]=="FINSEGUN") tmp2--;
						tmp1++;
					}
					Ejecutar(x+1,tmp1-1); 
				}
				line=fin+1;
				_pos(line);
				_sub(line,"Se sale de la estructura Segun.	");
			}
		}
	}
}
