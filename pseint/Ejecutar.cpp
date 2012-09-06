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
		if (cadena=="FINPROCESO" || cadena=="FINSUBPROCESO") break;
		Inter.SetLineAndInstructionNumber(line);
		if (cadena[cadena.size()-1]==';') { // Si es una accion secuencial
			// ----------- ESCRIBIR ------------- //
			if (cadena=="BORRARPANTALLA;") {
				clrscr();
				gotoXY(1,1);
			} else if (cadena=="ESPERARTECLA;") {
				getKey();
			} else if (LeftCompare(cadena,"INVOCAR ")) {
				string llamada=cadena.substr(8); llamada.erase(llamada.length()-1,1); // cortar el "invocar" y el ";"
				tipo=vt_desconocido; size_t p=llamada.find('(',0);
				if (p==string::npos)
					EvaluarFuncion(EsFuncion(llamada),"()",tipo,false);
				else
					EvaluarFuncion(EsFuncion(llamada.substr(0,p)),llamada.substr(p),tipo,false);
			} else if (LeftCompare(cadena,"ESCRIBIR ") || LeftCompare(cadena,"ESCRIBNL ")) {
				bool saltar=LeftCompare(cadena,"ESCRIBIR ");
				cadena.erase(0,9);
				cadena.erase(cadena.size()-1,1);
				tmp1=-1;tmp2=0;tmp3=0;
				// Separar parametros
				while (tmp3<(int)cadena.size()) {
					while (!(tmp1<0 && tmp2==0 && cadena[tmp3]==',') && tmp3<(int)cadena.size()) {
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
					Escribir(aux1);
				}
				if (saltar) cout<<endl; else cout<<flush;
			} else 
			// ------------- LEER --------------- //
			if (LeftCompare(cadena,"LEER ")) {
				string aux3,aux4;
				cadena.erase(0,5);
				cadena.erase(cadena.size()-1,1);
				tmp1=0; tmp2=0;
				while (tmp2<(int)cadena.size()) {
					while (!(tmp1==0 && cadena[tmp2]==',') && tmp2<(int)cadena.size()) {
						tmp2++;
						if (cadena[tmp2]=='(') tmp1++;
						if (cadena[tmp2]==')') tmp1--;
					}
					// Cortar el nombre de la variable a leer
					aux2=cadena;
					aux2.erase(tmp2,cadena.size()-tmp2);
					cadena.erase(0,aux2.size()+1);
					tmp2-=aux2.size();

					if (force_var_definition && !memoria->EstaDefinida(aux2)) {
						ExeError(208,"Variable no definida ("+aux2+").");
					}
					tipo=memoria->LeerTipo(aux2);
					size_t pp=aux2.find("(");
					if (tipo.dims && pp==string::npos)
						ExeError(200,"Faltan subindices para el arreglo ("+aux2+").");
					else if (!tipo.dims && pp!=string::npos)
						ExeError(201,"La variable ("+aux2.substr(0,pp)+") no es un arreglo.");
					if (tipo.dims) CheckDims(aux2);
					
					if (colored_output) setForeColor(COLOR_INFO);
					cout<<"> "<<flush;
					if (colored_output) setForeColor(COLOR_INPUT);
					// Leer dato
					if (!predef_input.empty() || noinput) {
						if (predef_input.empty()) ExeError(214,"Sin entradas disponibles.");
						aux1=predef_input.front(); predef_input.pop(); cout<<aux1<<endl;
					} else
						getline(cin,aux1);  
					
					fixwincharset(aux1,true); // "descorrige" para que al corregir no traiga problemas
					
					if (toUpper(aux1)=="VERDADERO" || toUpper(aux1)=="FALSO") aux1=toUpper(aux1);
					
					if (tipo==vt_logica && aux1.size()==1 && (toupper(aux1[0])=='F'||aux1[0]=='0')) aux1=FALSO;
					if (tipo==vt_logica && aux1.size()==1 && (toupper(aux1[0])=='V'||aux1[0]=='1')) aux1=VERDADERO;
					tipo_var tipo2 = GuestTipo(aux1);
					if (!tipo.set(tipo2))
//						if (tipo=='n' && (GuestTipo(aux1)=='c' || GuestTipo(aux1)=='l')) {
						ExeError(120,"No coinciden los tipos.");
//						}
//						if (tipo=='l' && (GuestTipo(aux1)=='c' || GuestTipo(aux1)=='n')) {
//							ExeError(121,"No coinciden los tipos.");
//						}
//						if (tipo=='d') tipo=GuestTipo(aux1);
//						if (tipo=='n') tipo='d'; // cambio 20080421
					memoria->DefinirTipo(aux2,tipo);
					memoria->EscribirValor(aux2,aux1);
//						EscribirVar(aux2,tipo,aux1); // Escribir el Dato en Memoria
				}
			} else 
					// ------------- DIMENSION --------------- //
			if (LeftCompare(cadena,"DIMENSION ")) {
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
					while (!(tmp2==0 && aux2[tmp1]==',') && tmp1<(int)aux2.size()) {
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
				while (tmp1<(int)aux2.size()) {
					while (!(tmp2==0 && aux2[tmp1]==',') && tmp1<(int)aux2.size()) {
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
						ExeError(999,"Las dimensiones deben ser mayores a 0.");
					}
					tmp3++; last=tmp1+1; tmp1++;
				}
				if (memoria->HaSidoUsada(aux1)||memoria->LeerDims(aux1))
					ExeError(123,"Identificador en uso.");
				if (dim!=0) memoria->AgregarArreglo(aux1, dim);
				if (otro!="") { // si hay otro, inicilizarlo tambien
					programa[line]=otro;
					Ejecutar(line,line);
				}
			} else
			// ------------- DEFINICION --------------- //
			if (LeftCompare(cadena,"DEFINIR ")) {
				string aux3,aux4; tmp1=0; tmp2=0; cadena.erase(0,8); bool rounded=false;
				if (RightCompare(cadena," COMO LOGICO;")) { tipo=vt_logica; aux1="FALSO"; cadena.erase(cadena.size()-13,13); }
				else if (RightCompare(cadena," COMO REAL;")) { tipo=vt_numerica; aux1="0"; cadena.erase(cadena.size()-11,11); }
				else if (RightCompare(cadena," COMO ENTERO;")) { tipo=vt_numerica; aux1="0"; cadena.erase(cadena.size()-13,13); rounded=true; }
				else if (RightCompare(cadena," COMO CARACTER;")) { tipo=vt_caracter; aux1=""; cadena.erase(cadena.size()-15,15); }
				while (tmp2<(int)cadena.size()) {
					while (!(tmp1==0 && cadena[tmp2]==',') && tmp2<(int)cadena.size()) {
						tmp2++;
						if (cadena[tmp2]=='(') tmp1++;
						if (cadena[tmp2]==')') tmp1--;
					}
					// Cortar el nombre de la variable a leer
					aux2=cadena;
					aux2.erase(tmp2,cadena.size()-tmp2);
					cadena.erase(0,aux2.size()+1);
					tmp2-=aux2.size();
					if (memoria->EstaDefinida(aux2)) 
						ExeError(124,string("La variable (")+aux2+") ya estaba definida.");
					memoria->DefinirTipo(aux2,tipo,rounded);
//								memoria->EscribirValor(aux2,aux1);
//								EscribirVar(aux2,tipo,aux1); // Escribir el Dato en Memoria
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
				aux2=Evaluar(aux2,tipo);
				if (!tipo.cb_num) ExeError(219,string("La longitud del intervalo debe ser numérica."));
				else Sleep(int(StrToDbl(aux2)*factor));
			} else {
				// ------------- ASIGNACION --------------- //
				// separar variable y expresion en aux1 y aux2
				tmp1=cadena.find("<-",0);
				aux1=cadena.substr(0,tmp1);  
				aux2=cadena.substr(tmp1+3,cadena.size()-tmp1-5); // ignorar flecha, punto y como y parentesis extras
				if (force_var_definition && !memoria->EstaDefinida(aux1)) {
					ExeError(211,string("La variable (")+aux1+") no esta definida.");
				}
				// verificar indices si es arreglo
				if (memoria->LeerTipo(aux1).dims) {
					if (aux1.find("(",0)==string::npos)
						ExeError(200,"Faltan subindices para el arreglo ("+aux1+").");
					else
						CheckDims(aux1);
				} else if (aux1.find("(",0)!=string::npos) {
					ExeError(201,"La variable ("+aux1.substr(0,aux1.find("(",0))+") no es un arreglo.");
				}
				// evaluar expresion
				aux2=Evaluar(aux2,tipo);
				// comprobar tipos
				if (/*tipo.is_known() && */!memoria->LeerTipo(aux1).can_be(tipo))
					ExeError(125,"No coinciden los tipos.");
				// escribir en memoria
				memoria->DefinirTipo(aux1,tipo);
				memoria->EscribirValor(aux1,aux2);
//							EscribirVar(aux1,tipo,aux2);
			}
		} else { // Si no es secuencial
			// ---------------- SI ------------------ //
			if (LeftCompare(cadena,"SI ")) {
				cadena.erase(0,3);
				aux1=Evaluar(cadena,tipo,vt_logica);
//				if (tipo<'c')
//					ExpError(tipo,1);
//				else {
				if (tipo!=vt_error) {
					if (aux1=="VERDADERO") {
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
						Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
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
							Ejecutar(line+1,tmp1-1); // ejecutar salida por verdadero
						}
						line=tmp1;
						Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
					}
				} else {
					ExeError(999,"No coinciden los tipos.");
				}
			} else 
			// ---------------- MIENTRAS ------------------ //
			if (LeftCompare(cadena,"MIENTRAS ")) {
				cadena.erase(0,9);
				cadena.erase(cadena.size()-6,6);
				aux1=Evaluar(cadena,tipo);
		//					if (tipo<'c')
		//						ExpError(tipo,1);
		//					else {
				if (tipo!=vt_error) {
					tmp1=line+1; tmp2=0; // Buscar hasta donde llega el bucle
					while (!(tmp2==0 && programa[tmp1]=="FINMIENTRAS")) {
						// Saltear bucles anidados
						if (LeftCompare(programa[tmp1],"MIENTRAS ")) tmp2++;
						if (programa[tmp1]=="FINMIENTRAS") tmp2--;
						tmp1++;
					}
					while (Evaluar(cadena,tipo)=="VERDADERO") {
		//							if (tipo<'c')
		//								ExpError(tipo,1);
						Ejecutar(line+1,tmp1-1);
						Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
					}
		//						if (tipo<'c') ExpError(tipo,1);
					line=tmp1;
					Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
				}
			} else 
			// ---------------- REPETIR HASTA QUE ------------------ //
			if (cadena=="REPETIR") {
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
				// ejecutar por primera vez
				Ejecutar(line+1,tmp1-1);
				// evaluar condicion y seguir
				aux1=Evaluar(cadena,tipo);
	//						if (tipo<'c')
	//							ExpError(tipo,1);
	//						else {
				if (tipo!=vt_error) {
					// bucle
					while (Evaluar(cadena,tipo)==valor_verdad) {
	//								if (tipo<'c') ExpError(tipo,1);
						Ejecutar(line+1,tmp1-1);
						Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
					}
					line=tmp1;
					Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
				}
			} else 
			// ------------------- PARA --------------------- //
			if (LeftCompare(cadena,"PARA ")) {
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
					positivo=(Evaluar(val_paso+">=0",tipo)=="VERDADERO");
				} else { // si no hay paso adivinar
					val_fin=cadena;
					if (lazy_syntax && Evaluar(val_fin+"<"+val_ini,tipo)=="VERDADERO") {
						positivo=false; val_paso="-1";
					} else {
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
				
				memoria->EscribirValor(contador,val_ini); // inicializa el contador
				if (positivo) // si el paso es positivo
					while (Evaluar(contador+"<="+val_fin,tipo)=="VERDADERO") {
						Ejecutar(line+1,tmp1-1);
						Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
						memoria->EscribirValor(contador,Evaluar(contador+"+("+val_paso+")",tipo));
					}
				else // si el paso es negativo
					while (Evaluar(contador+">="+val_fin,tipo)=="VERDADERO"){
						Ejecutar(line+1,tmp1-1);
						Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
						memoria->EscribirValor(contador,Evaluar(contador+"+("+val_paso+")",tipo));
					}
				line=tmp1;
				Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
			} else 
			// ------------------- PARA CADA --------------------- //
			if (LeftCompare(cadena,"PARACADA ")) {
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

				int *dims=memoria->LeerDims(aux2);
				if (!dims) ExeError(999,"La variable ("+aux2+") no es un arreglo.");
				int nelems=1; // cantidad total de iteraciones
				for (int i=1;i<=dims[0];i++) nelems*=dims[i];
				
				// bucle posta
				for (int i=0;i<nelems;i++) {
					// armar expresion del elemento (ej: A[1])
					string elemento=")";
					int naux=1;
					for (int j=dims[0];j>0;j--) {
						elemento=string(",")+IntToStr((base_zero_arrays?0:1)+((i/naux)%dims[j]))+elemento;
						naux*=dims[j];
					}
					elemento=aux2+"("+elemento.substr(1);
					// asignar el elemento en la variable del bucle
					Inter.SetLineAndInstructionNumber(line);
					if (!memoria->DefinirTipo(aux1,memoria->LeerTipo(elemento)))
						ExeError(999,"No coinciden los tipos.");
					memoria->EscribirValor(aux1,memoria->LeerValor(elemento));
					// ejecutar la iteracion
					Ejecutar(line+1,tmp1-1);
					// asignar la variable del bucle en el elemento
					Inter.SetLineAndInstructionNumber(tmp1);
					memoria->DefinirTipo(aux1,memoria->LeerTipo(elemento));
					memoria->EscribirValor(elemento,memoria->LeerValor(aux1));
				}
				line=tmp1; // liena del finpara
				
			} else 
			// ------------------- SEGUN --------------------- //
			if (LeftCompare(cadena,"SEGUN ")) {
				int fin;
				cadena.erase(0,6); cadena.erase(cadena.size()-6,6); // Cortar la variable
				tipo_var tipo_master=vt_caracter_o_numerica;
				Evaluar(aux2=cadena,tipo,tipo_master); // evaluar para verificar el tipo
				if (!tipo.cb_num&&(!lazy_syntax||!tipo.cb_car)) {
					if (lazy_syntax) 
						ExeError(205,"La expresión del SEGUN debe ser de tipo numerica o caracter.");
					else
						ExeError(206,"La expresión del SEGUN debe ser numerica.");
				}
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
						if (cadena=="DE OTRO MODO:")
						{ flag_coincide=1; break;}
						else {
							cadena[cadena.size()-1]=',';
							tmp2=cadena.find(",",0);
							while (tmp2<=(int)cadena.size() && tmp2>=0) {
								// evaluar el parametro para verificar el tipo
								aux3=cadena.substr(0,tmp2);
								cadena.erase(0,aux3.size()+1);
								aux1=Evaluar(aux3,tipo,tipo_master);
								if (!tipo.cb_num&&(!lazy_syntax||!tipo.cb_car)) ExeError(127,"No coinciden los tipos.");
								// evaluar la condicion (se pone como estaban y no los resultados de la evaluaciones de antes porque sino las variables indefinida pueden no tomar el valor que corresponde
								if (Evaluar(aux3+"="+aux2,tipo)==VERDADERO) {cadena=aux1;flag_coincide=1;x--;break;}
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
					Ejecutar(x+1,tmp1-1); }
				line=fin+1;
				Inter.SetLineAndInstructionNumber(line); // cambio 20080623 para ejecucion paso a paso
			}
		}
	}
}
