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
// Ejecuta desde linestart+1 hasta lineend inclusive, o hasta finproceso/finsubproceso si lineend es -1.
// Las variables aux?, tmp? y tipo quedaron del código viejo, se reutilizan para diferentes
// cosas, por lo que habría que analizarlas y cambiarlas por varias otras variables con scope y 
// nombres mas claros... por ahora cambie las obvias y reduje el scope de las que quedaron, pero falta...
void Ejecutar(int LineStart, int LineEnd) {
	// variables auxiliares
	string cadena;
	// Ejecutar el bloque
	int line=LineStart-1;
	while (true) {
		line++;
		if (LineEnd!=-1 && line>LineEnd) break; 
		cadena=programa[line].instruccion;
		InstructionType instruction_type=programa[line].type;
//cout << LineStart+1 << ":"<<LineEnd+1<< "   " << cadena << endl; // debug
		if (instruction_type==IT_FINPROCESO || instruction_type==IT_FINSUBPROCESO) {
			Inter.OnAboutToEndFunction();
			_pos(line);
			if (instruction_type==IT_FINSUBPROCESO) {
				_sub(line,string("Se sale del subproceso ")+cadena);
				Inter.OnFunctionOut();
			} else {
				_sub(line,"Finaliza el algoritmo");
			}
			break;
		}
		if (instruction_type==IT_PROCESO || instruction_type==IT_SUBPROCESO) {
			bool es_proc=instruction_type==IT_PROCESO;
			size_t p=cadena.find(' '); cadena=cadena.substr(p+1);
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
			if (instruction_type==IT_BORRARPANTALLA) {
				if (for_test) cout<<"***LimpiarPantalla***"<<endl; else { clrscr(); gotoXY(1,1); }
				_sub(line,"Se borra la pantalla");
			} else if (instruction_type==IT_ESPERARTECLA) {
				_sub_msg(line,"Se espera a que el usuario presione una tecla.");
				_sub_raise();
				if (for_test) cout<<"***EsperarTecla***"<<endl; else getKey();
				_sub_wait();
			} else if (instruction_type==IT_INVOCAR) {
				string llamada=cadena.substr(8); llamada.erase(llamada.length()-1,1); // cortar el "invocar" y el ";"
				tipo_var tipo=vt_desconocido; size_t p=llamada.find('(',0);
				_sub(line,string("Se va a invocar al subproceso")+llamada.substr(0,p));
				if (p==string::npos)
					EvaluarFuncion(EsFuncion(llamada),"()",tipo,false);
				else
					EvaluarFuncion(EsFuncion(llamada.substr(0,p)),llamada.substr(p),tipo,false);
				// ----------- ESCRIBIR ------------ //
			} else if (instruction_type==IT_ESCRIBIR || instruction_type==IT_ESCRIBIRNL) {
				bool saltar=instruction_type==IT_ESCRIBIR;
				cadena.erase(0,9);
				cadena.erase(cadena.size()-1,1);
				// Separar parametros
				for(size_t i=0, arg_num=1;i<cadena.size();++arg_num) {
					for(int par_level = 0; i<cadena.size() && !(par_level==0 && cadena[i]==',');++i) {
						if (cadena[i]=='\'') while (cadena[++i]!='\'');
						else if (cadena[i]=='(') par_level++;
						else if (cadena[i]==')') par_level--;
					}
					string aux1 = cadena;
					aux1.erase(i,aux1.size()-i);
					i -= aux1.size();
					cadena.erase(0,aux1.size()+1);
					
					if (colored_output) setForeColor(COLOR_OUTPUT);
					if (with_io_references) Inter.SendIOPositionToTerminal(arg_num);
					_sub(line,string("Se evalúa la expresion: ")+aux1);
					DataValue res = Evaluar(aux1);
					if (res.IsOk()) {
						string ans = res.GetForUser(); fixwincharset(ans);
						cout<< ans <<flush; // Si es variable, muestra el contenido
						_sub(line,string("Se muestra en pantalla el resultado: ")+res.GetForUser());
					}
				}
				if (saltar) cout<<endl; else cout<<flush;
			} else 
			// ------------- LEER --------------- //
			if (instruction_type==IT_LEER) {
				cadena.erase(0,5);
				cadena.erase(cadena.size()-1,1);
				for(size_t i=0, arg_num=1; i<cadena.size();++arg_num) {
					for(int par_level=0;i<cadena.size() && !(par_level==0 && cadena[i]==',');++i) {
						if (cadena[i]=='\'') while(cadena[++i]!='\'');
						else if (cadena[i]=='(') par_level++;
						else if (cadena[i]==')') par_level--;
					}
					// Cortar el nombre de la variable a leer
					string aux2=cadena;
					aux2.erase(i,cadena.size()-i);
					cadena.erase(0,aux2.size()+1);
					i-=aux2.size();
					
					if (lang[LS_FORCE_DEFINE_VARS] && !memoria->EstaDefinida(aux2)) {
						ExeError(208,"Variable no definida ("+aux2+").");
					}
					tipo_var tipo=memoria->LeerTipo(aux2);
					const int *dims=memoria->LeerDims(aux2);
					size_t pp=aux2.find("(");
					if (dims && pp==string::npos)
						ExeError(200,"Faltan subindices para el arreglo ("+aux2+").");
					else if (!dims && pp!=string::npos)
						ExeError(201,"La variable ("+aux2.substr(0,pp)+") no es un arreglo.");
					if (dims) {
						_sub(line,string("Se analizan las dimensiones de ")+aux2);
						CheckDims(aux2);
						_sub(line,string("El resultado es ")+aux2);
					}
					
					if (with_io_references) Inter.SendIOPositionToTerminal(arg_num);
					if (colored_output) setForeColor(COLOR_INFO);
					cout<<"> "<<flush;
					if (colored_output) setForeColor(COLOR_INPUT);
					// Leer dato
					_sub_msg(line,"Se espera a que el usuario ingrese un valor y presiones enter."); // tipo?
					_sub_raise();
					
					string aux1;
					if (!predef_input.empty() || noinput) {
						if (predef_input.empty()) ExeError(214,"Sin entradas disponibles.");
						aux1=predef_input.front(); predef_input.pop(); cout<<aux1<<endl;
						_sub_wait();
					} else {
						aux1=getLine();  
						if (for_eval) {
							if (aux1=="<{[END_OF_INPUT]}>") {
								cout << "<<No hay más datos de entrada>>" << endl; exit(0);
							}
							cout<<aux1<<endl; // la entrada en psEval es un stream separado de la salida, entonces la reproducimos alli para que la salida contenga todo el "dialogo"
						}
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
					memoria->EscribirValor(aux2,DataValue(tipo,aux1));
				}
			} else 
					// ------------- DIMENSION --------------- //
			if (instruction_type==IT_DIMENSION) {
				do {
					string otro=""; // por si hay mas de un arreglo
					cadena.erase(0,10);
					int pos_par = cadena.find("(",0);  // Cortar nombre del arreglo
					string name=cadena; name.erase(pos_par,name.size()-pos_par);
					string aux2=cadena; // cortar indices
					aux2.erase(0,name.size()+1); aux2.erase(aux2.size()-2,2);
					// Separar indices
					int anid_parent=0, cant_dims=0;
					for(size_t i=0;i<aux2.size();i++) {
						while (i<aux2.size() && !(anid_parent==0 && (aux2[i]==','||aux2[i]==')'))) {
							if (aux2[i]=='(') anid_parent++;
							else if (aux2[i]==')') anid_parent--;
							i++;
						}
						if (aux2[i]==')') {
							otro=aux2;
							otro.erase(0,i+2);
							otro="DIMENSION "+otro+");";
							aux2.erase(i,aux2.size()-i);
							aux2=aux2+",";
							cant_dims++;
							break;
						}
						cant_dims++;
					}
					int *dim = new int[cant_dims+1]; dim[0]=cant_dims; // arreglo para las dimensiones
					int last=0, num_idx=0; anid_parent = 0;
					if (lang[LS_ALLOW_DINAMYC_DIMENSIONS]) { _sub(line,string("Se evalúan las expresiones para cada dimensión del arreglo ")+name); }
					for(size_t i=0;i<aux2.size();i++) {
						while (i<aux2.size() && !(anid_parent==0 && aux2[i]==',')) {
							if (aux2[i]=='(') anid_parent++;
							else if (aux2[i]==')') anid_parent--;
							i++;
						}
						cadena=aux2; // Cortar la expresion indice
						cadena.erase(i,cadena.size()-i); cadena.erase(0,last);
						DataValue index = Evaluar(cadena);
						if (!index.CanBeReal()) ExeError(122,"No coinciden los tipos.");
						dim[++num_idx] = index.GetAsInt();
						if (dim[num_idx]<=0) {
							ExeError(274,"Las dimensiones deben ser mayores a 0.");
						}
						last=i+1;
					}
					if (Inter.subtitles_on) {
						string tamanio;
						for(int i=1;i<=dim[0];i++) tamanio+="x"+IntToStr(dim[i]);
						tamanio[0]=' ';
						_sub(line,string("Se crea el arreglo ")+name+" de"+tamanio+" elementos");
					}
					if (memoria->HaSidoUsada(name)||memoria->LeerDims(name))
						ExeError(123,"Identificador en uso.");
					if (dim!=0) memoria->AgregarArreglo(name, dim);
					if (otro!="") cadena=otro; else cadena="";
				} while (cadena.size());
			} else
			// ------------- DEFINICION --------------- //
			if (instruction_type==IT_DEFINIR) {
				string aux1, aux2; tipo_var tipo;
				int tmp1=0, tmp2=0; cadena.erase(0,8); bool rounded=false;
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
					if (tipo==vt_numerica) {
						if (rounded) {
							_sub(line,string("Se define el tipo de la variable \"")+aux2+"\" como Numérico(Entero).");
						} else {
							_sub(line,string("Se define el tipo de la variable \"")+aux2+"\" como Numérico(Real).");
						}
					} else if (tipo==vt_caracter) {
						_sub(line,string("Se define el tipo de la variable \"")+aux2+"\" como Caracter/Cadena de Caracteres.");
					} else if (tipo==vt_logica) {
						_sub(line,string("Se define el tipo de la variable \"")+aux2+"\" como Lógico.");
					} 
				}
			} else
			// ------------- ESPERAR un tiempo --------------- //
			if (instruction_type==IT_ESPERAR) {
				string aux2=cadena.substr(8); 
				int factor=1;
				if (RightCompare(aux2," SEGUNDO;")) { factor=1000; aux2.erase(aux2.size()-9); }
				else if (RightCompare(aux2," SEGUNDOS;")) { factor=1000; aux2.erase(aux2.size()-10); }
				if (RightCompare(aux2," MILISEGUNDO;")) { factor=1; aux2.erase(aux2.size()-13); }
				else if (RightCompare(aux2," MILISEGUNDOS;")) { factor=1; aux2.erase(aux2.size()-14); }
				_sub(line,string("Se evalúa la cantidad de tiempo: ")+aux2);
				DataValue time = Evaluar(aux2);
				if (!time.CanBeReal()) ExeError(219,string("La longitud del intervalo debe ser numérica."));
				else {
					_sub(line,string("Se esperan ")+time.GetForUser()+(factor==1?" milisengudos":" segundos"));
					if (for_test) cout<<"***Esperar"<<time.GetAsInt()*factor<<"***"<<endl;
					else if (!Inter.subtitles_on) Sleep(time.GetAsInt()*factor);
				}
			} else 
			// ------------- ASIGNACION --------------- //
			if (instruction_type==IT_ASIGNAR) {
				// separar variable y expresion en aux1 y aux2
				int tmp1=cadena.find("<-",0);
				string aux1=cadena.substr(0,tmp1);
				string aux2=cadena.substr(tmp1+3,cadena.size()-tmp1-5); // ignorar flecha, punto y como y parentesis extras
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
				DataValue result = Evaluar(aux2);
				// comprobar tipos
				tipo_var tipo_aux1 = memoria->LeerTipo(aux1);
				if (!tipo_aux1.can_be(result.type))
					ExeError(125,"No coinciden los tipos.");
				else if (tipo_aux1==vt_numerica_entera && tipo_aux1.rounded && result.GetAsInt()!=result.GetAsReal())
					ExeError(314,"No coinciden los tipos, el valor a asignar debe ser un entero.");
				_sub(line,string("El resultado es: ")+result.GetForUser());
				// escribir en memoria
				_sub(line,string("El resultado se guarda en ")+aux1);
				result.type.rounded=false; // no forzar a entera la variable en la que se asigna
				memoria->DefinirTipo(aux1,result.type);
				memoria->EscribirValor(aux1,result);
			} // ya deberíamos haber cubierto todas las opciones
			else {
				ExeError(0,"Ha ocurrido un error interno en PSeInt.");
			}
		} else { // Si no es secuencial
			// ---------------- SI ------------------ //
			if (instruction_type==IT_SI) {
				cadena.erase(0,3);
				_pos(line);
				_sub(line,string("Se evalúa la condición para Si-Entonces: ")+cadena);
				tipo_var tipo;
				bool condition_is_true = Evaluar(cadena,vt_logica).GetAsBool();
				if (tipo!=vt_error) {
					// Buscar hasta donde llega el bucle
					int anidamiento=0, line_sino=-1,line_finsi=line+1; 
					while (!(anidamiento==0 && programa[line_finsi]==IT_FINSI)) {
						// Saltear bucles anidados
						if (anidamiento==0 && programa[line_finsi]==IT_SINO) line_sino=line_finsi; 
						else if (programa[line_finsi]==IT_SI) anidamiento++;
						else if (programa[line_finsi]==IT_FINSI) anidamiento--;
						line_finsi++;
					}
					// ejecutar lo que corresponda
					if (condition_is_true) {
						_sub(line+1,"El resultado es Verdadero, se sigue por la rama del Entonces");
						if (line_sino==-1) line_sino=line_finsi;
						Ejecutar(line+2,line_sino-1); // ejecutar salida por verdadero
					} else {
						if (line_sino!=-1) {
							line = line_sino;
							_pos(line);
							_sub(line,"El resultado es Falso, se sigue por la rama del SiNo");
							Ejecutar(line+1,line_finsi-1); // ejecutar salida por falso
						} else {
							_sub(line,"El resultado es Falso, no se hace nada");
						}
					}
					// marcar la salida
					line=line_finsi;
					_pos(line);
					_sub(line,"Se sale de la estructura Si-Entonces");
				} else {
					ExeError(275,"No coinciden los tipos.");
				}
			} else 
			// ---------------- MIENTRAS ------------------ //
			if (instruction_type==IT_MIENTRAS) {
				cadena.erase(0,9);
				cadena.erase(cadena.size()-6,6);
				_pos(line);
				_sub(line,string("Se evalúa la condición para Mientras: ")+cadena);
				tipo_var tipo;
				bool condition_is_true = Evaluar(cadena,vt_logica).GetAsBool();
				if (tipo!=vt_error) {
					int line_finmientras = line+1, anidamiento=0; // Buscar hasta donde llega el bucle
					while (!(anidamiento==0 && programa[line_finmientras]==IT_FINMIENTRAS)) {
						// Saltear bucles anidados
						if (programa[line_finmientras]==IT_MIENTRAS) anidamiento++;
						else if (programa[line_finmientras]==IT_FINMIENTRAS) anidamiento--;
						line_finmientras++;
					}
					while (condition_is_true) {
						_sub(line,"La condición es Verdadera, se iniciará una iteración.");
						Ejecutar(line+1,line_finmientras-1);
						_pos(line);
						_sub(line,string("Se evalúa nuevamente la condición: ")+cadena);
						condition_is_true = Evaluar(cadena,vt_logica).GetAsBool();
					}
					line=line_finmientras;
					_pos(line);
					_sub(line,"La condición es Falsa, se sale de la estructura Mientras.");
				}
			} else 
			// ---------------- REPETIR HASTA QUE ------------------ //
			if (instruction_type==IT_REPETIR) {
				_pos(line);
				int line_hastaque=line+1, anidamiento=0; // Buscar hasta donde llega el bucle
				while (!(anidamiento==0 && (programa[line_hastaque]==IT_HASTAQUE||programa[line_hastaque]==IT_MIENTRASQUE))) {
					// Saltear bucles anidados
					if (programa[line_hastaque]==IT_REPETIR) anidamiento++;
					else if (programa[line_hastaque]==IT_HASTAQUE || programa[line_hastaque]==IT_MIENTRASQUE) anidamiento--;
					line_hastaque++;
				}
				// cortar condicion de cierre
				cadena=programa[line_hastaque];
				instruction_type=programa[line_hastaque].type;
				bool valor_verdad;
				if (instruction_type==IT_HASTAQUE){
					cadena.erase(0,10);
					valor_verdad=false;
				} else {
					cadena.erase(0,13);
					valor_verdad=true;
				}
				_sub(line,"Se ejecutarán las acciones contenidas en la estructura Repetir");
				tipo_var tipo;
				bool should_continue_iterating=true;
				while (should_continue_iterating) {
					Ejecutar(line+1,line_hastaque-1);
					// evaluar condicion y seguir
					_pos(line_hastaque);
					_sub(line_hastaque,string("Se evalúa la condición: ")+cadena);
					should_continue_iterating = Evaluar(cadena,vt_logica).GetAsBool()==valor_verdad;
					if (should_continue_iterating)
						_sub(line_hastaque,string("La condición es ")+(valor_verdad?VERDADERO:FALSO)+", se contiúa iterando.");
				} while (should_continue_iterating);
				line=line_hastaque;
				_sub(line_hastaque,string("La condición es ")+(valor_verdad?FALSO:VERDADERO)+", se sale de la estructura Repetir.");
			} else 
			// ------------------- PARA --------------------- //
			if (instruction_type==IT_PARA) {
				_pos(line);
				bool positivo; // para saber si es positivo o negativo
				cadena.erase(0,5); // saca "PARA "
				cadena.erase(cadena.size()-6,6); // saca " HACER"
				// corta condicion
				int tmp1=cadena.find(" HASTA ");
				string aux1=cadena.substr(0,tmp1);
				cadena.erase(0,tmp1+7); // saca la asignacion inicial
				tmp1=aux1.find("<-",0);
				string contador = aux1.substr(0,tmp1); // variable del para
				memoria->DefinirTipo(aux1,vt_numerica);
				string expr_ini = aux1.substr(tmp1+2); // valor inicial
				_sub(line,string("Se evalúa la expresion para el valor inicial: ")+expr_ini);
				
				DataValue res_ini = Evaluar(expr_ini,vt_numerica), res_paso(vt_numerica,"1"), res_fin;
				if (!res_ini.CanBeReal()) ExeError(126,"No coinciden los tipos."); /// @todo: parece que esto no es posible, salta antes adentro del evaluar
				
				size_t pos_paso=cadena.find(" CON PASO ",0); // busca el paso
				if (pos_paso!=string::npos) { // si hay paso tomar ese
					string expr_fin = cadena.substr(0,pos_paso);
					if (RightCompare(expr_fin," HACER")) // por si ponen "...HASTA 5 HACER CON PASO 3 HACER" (2 HACER)
						expr_fin = expr_fin.substr(0,expr_fin.size()-6);
					res_fin = Evaluar(expr_fin,vt_numerica);
					string expr_paso = cadena.substr(pos_paso+10);
					_sub(line,string("Se evalúa la expresion para el paso: ")+expr_paso);
					res_paso = Evaluar(expr_paso,vt_numerica);
					positivo = res_paso.GetAsReal()>=0;
				} else { // si no hay paso adivinar
					res_fin = Evaluar(cadena,vt_numerica);
					if (lang[LS_DEDUCE_NEGATIVE_FOR_STEP] && res_ini.GetAsReal()>res_fin.GetAsReal()) {
						_sub(line,"Se determina que el paso será -1.");
						positivo=false; res_paso.SetFromInt(-1);
					} else {
						_sub(line,"Se determina que el paso será +1.");
						positivo=true; res_paso.SetFromInt(1);
					}
				}
				
				// Buscar hasta donde llega el bucle
				int line_finpara=line+1, anidamiento=0;
				while (!(anidamiento==0 && programa[line_finpara]==IT_FINPARA)) {
					// Saltear bucles anidados
					if (programa[line_finpara]==IT_PARA||programa[line_finpara]==IT_PARACADA) anidamiento++;
					else if (programa[line_finpara]==IT_FINPARA) anidamiento--;
					line_finpara++;
				}
				
				_sub(line,string("Se inicializar el contador ")+contador+" en "+res_ini.GetForUser());
				memoria->EscribirValor(contador,res_ini); // inicializa el contador
				string comp=positivo?"<=":">=";
				do {
					/// @todo: cuando memoria maneje DataValues usar el valor del contador directamente desde ahi en lugar de evaluar
					_sub(line,string("Se compara el contador con el valor final: ")+contador+"<="+res_fin.GetForUser());
					DataValue res_cont = Evaluar(contador,vt_numerica);
					if ( positivo ? (res_cont.GetAsReal()>res_fin.GetAsReal()) : (res_cont.GetAsReal()<res_fin.GetAsReal()) ) break;
					_sub(line,"La expresión fue Verdadera, se iniciará una iteración.");
					Ejecutar(line+1,line_finpara-1);
					_pos(line);
					res_cont = Evaluar(contador,vt_numerica); // pueden haber cambiado a para el contador!!!
					memoria->EscribirValor(contador,DataValue::MakeReal(res_cont.GetAsReal()+res_paso.GetAsReal()));
					_sub(line,string("Se actualiza el contador, ahora ")+contador+" vale "+aux1+".");
				} while(true);
				memoria->Desinicializar(contador);
				line=line_finpara;
				_pos(line);
				_sub(line,"Se sale de la estructura repetitiva Para.");
			} else 
			// ------------------- PARA CADA --------------------- //
			if (instruction_type==IT_PARACADA) {
				bool primer_iteracion=true; _pos(line);
				cadena.erase(0,9); // borrar "PARACADA "
				cadena.erase(cadena.size()-6,6); // borrar " HACER"
				int tmp1=cadena.find(" ");
				int tmp2=cadena.find(" ",tmp1+4);
				string aux1=cadena.substr(0,tmp1); // indetificador para el elemento
				string aux2=cadena.substr(tmp1+4,tmp2-tmp1-3); // identificador del arreglo
				
				int line_finpara=line+1, anidamiento=0; // Buscar hasta donde llega el bucle
				while (!(anidamiento==0 && programa[line_finpara]==IT_FINPARA)) {
					// Saltear bucles anidados
					if (programa[line_finpara]==IT_PARA||programa[line_finpara]==IT_PARACADA) anidamiento++;
					else if (programa[line_finpara]==IT_FINPARA) anidamiento--;
					line_finpara++;
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
					Ejecutar(line+1,line_finpara-1);
					// asignar la variable del bucle en el elemento
					memoria->DefinirTipo(elemento,memoria->LeerTipo(aux1));
					memoria->EscribirValor(elemento,memoria->LeerValor(aux1));
				}
				memoria->Desinicializar(aux1);
				line=line_finpara; // linea del finpara
				_pos(line);
				_sub(line,"Se sale de la estructura repetitiva Para Cada.");
				
			} else 
			// ------------------- SEGUN --------------------- //
			if (instruction_type==IT_SEGUN) {
				string expr_control = cadena.substr(6,cadena.size()-12); // Cortar la variable (sacar SEGUN y HACER)
				tipo_var tipo_master=vt_caracter_o_numerica;
				_pos(line);
				_sub(line,string("Se evalúa la expresion: ")+expr_control);
				DataValue val_control = Evaluar(expr_control,tipo_master); // evaluar para verificar el tipo
				if (!val_control.CanBeReal()&&(lang[LS_INTEGER_ONLY_SWITCH]||!val_control.CanBeString())) {
					if (!lang[LS_INTEGER_ONLY_SWITCH]) 
						ExeError(205,"La expresión del SEGUN debe ser de tipo numerica o caracter.");
					else
						ExeError(206,"La expresión del SEGUN debe ser numerica.");
				}
				_sub(line,string("El resultado es: ")+val_control.GetForUser());
				int line_finsegun=line+1, anidamiento=0; // Buscar hasta donde llega el bucle
				while (!(anidamiento==0 && LeftCompare(programa[line_finsegun],"FINSEGUN"))) {
					// Saltear bucles anidados
					if (programa[line_finsegun]==IT_SEGUN) anidamiento++;
					else if (programa[line_finsegun]==IT_FINSEGUN) anidamiento--;
					line_finsegun++;
				}
				int line_opcion=line; bool encontrado=false; anidamiento=0;
				while (!encontrado && ++line_opcion<line_finsegun) {
					instruction_type=programa[line_opcion].type;
					if (instruction_type==IT_SEGUN) anidamiento++;
					else if (instruction_type==IT_FINSEGUN) anidamiento--;
					else if ((instruction_type==IT_OPCION||instruction_type==IT_DEOTROMODO) && anidamiento==0) {
						if (instruction_type==IT_DEOTROMODO) { 
							_pos(line_opcion);
							_sub(line_opcion,"Se ingresará en la opción De Otro Modo");
							encontrado=true; break;
						}
						else {
							string posibles_valores = programa[line_opcion];
							posibles_valores[posibles_valores.size()-1]=',';
							size_t pos_fin_valor = posibles_valores.find(",",0);
							while (pos_fin_valor!=string::npos) {
								// evaluar el parametro para verificar el tipo
								string expr_opcion=posibles_valores.substr(0,pos_fin_valor);
								posibles_valores.erase(0,expr_opcion.size()+1);
								_pos(line_opcion);
								_sub(line_opcion,string("Se evalúa la opcion: ")+expr_opcion);
								DataValue val_opcion = Evaluar(expr_opcion,tipo_master);
								if (!val_opcion.CanBeReal()&&(lang[LS_INTEGER_ONLY_SWITCH]||!val_opcion.CanBeString())) ExeError(127,"No coinciden los tipos.");
								// evaluar la condicion (se pone como estaban y no los resultados de la evaluaciones de antes porque sino las variables indefinida pueden no tomar el valor que corresponde
								if (Evaluar(string("(")+expr_control+")=("+expr_opcion+")").GetAsBool()) {
									_sub(line_opcion,"El resultado coincide, se ingresará en esta opción.");
									encontrado=true; break;
								} else {
									_sub(line_opcion,string("El resultado no coincide: ")+val_opcion.GetForUser());
								}
//								if (tipo==vt_error) ExeError(127,"No coinciden los tipos."); // no parece hacer falta
								pos_fin_valor=posibles_valores.find(",",0);
							}
						}
					}
				}
				if (encontrado) { // si coincide, buscar el final del bucle
					int line_finopcion=line_opcion+1; anidamiento=0; // Buscar hasta donde llega ese caso
					while (!(anidamiento==0 && ((programa[line_finopcion]==IT_FINSEGUN) || (programa[line_finopcion]==IT_OPCION||programa[line_finopcion]==IT_DEOTROMODO)) )) {
						// Saltear bucles anidados
						if (programa[line_finopcion]==IT_SEGUN) anidamiento++;
						else if (programa[line_finopcion]==IT_FINSEGUN) anidamiento--;
						line_finopcion++;
					}
					Ejecutar(line_opcion+1,line_finopcion-1); 
				}
				line=line_finsegun;
				_pos(line);
				_sub(line,"Se sale de la estructura Segun.	");
			} // ya deberíamos haber cubierto todas las opciones
			else {
				ExeError(0,"Ha ocurrido un error interno en PSeInt.");
			}
		}
	}
}
