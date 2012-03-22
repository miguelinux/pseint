#include "Load.h"

#include <fstream>
#include <stack>
#include "Global.h"
#include "Entity.h"
#include "Events.h"
using namespace std;

static bool StartsWith(const string &s1, string s2) {
	if (s1.length()<s2.length()) return false;
	else return s1.substr(0,s2.length())==s2;
}

static Entity *Add(stack<int> &ids, Entity *vieja, Entity *nueva, int id=-1) {
	int mid=ids.top(); 
	ids.pop(); 
	ids.push(-1);
	if (mid==-1) 
		vieja->LinkNext(nueva);
	else 
		vieja->LinkChild(mid,nueva);
	if (id!=-1) 
		ids.push(id);
	return nueva;
}

void Load(const char *fname) {
	ifstream file(fname);
	string str;
	start = new Entity(ET_PROCESO,"Inicio");
	Entity *aux=start;
	stack<int> ids; ids.push(-1);
	while (getline(file,str)) {
		if (str.size() && str[str.size()-1]==';') str=str.substr(0,str.size()-1);
		bool comillas=false;
		for (unsigned int i=0;i<str.size();i++) {
			if (str[i]=='\''||str[i]=='\"') comillas=!comillas;
			else if (!comillas) {
				if (str[i]=='&') { str.replace(i,1," & "); i+=2; }
				else if (str[i]=='|') { str.replace(i,1," | "); i+=2; }
			}
		}
		if (!str.size()||StartsWith(str,"PROCESO ")||str=="FINPROCESO"||str=="ENTONCES") {
			continue;
		}
		else if (StartsWith(str,"ESCRIBIR ")) {
			aux=Add(ids,aux,new Entity(ET_ESCRIBIR,str.substr(9)));
		}
		else if (StartsWith(str,"LEER ")) {
			aux=Add(ids,aux,new Entity(ET_LEER,str.substr(5)));
		}
		else if (StartsWith(str,"MIENTRAS ")) {
			aux=Add(ids,aux,new Entity(ET_MIENTRAS,str.substr(9)),0);
		}
		else if (str=="REPETIR") {
			aux=Add(ids,aux,new Entity(ET_REPETIR,""),0);
		}
		else if (StartsWith(str,"PARA ")) {
			str=str.substr(5,str.size()-11);
			int i=str.find("<-");
			string var=str.substr(0,i);
			str=str.substr(i+2);
			i=str.find(" HASTA ");
			string ini=str.substr(0,i);
			str=str.substr(i+7);
			i=str.find(" CON PASO ");
			string paso,fin;
			if (i==string::npos) {
				paso="1"; fin=str;
			} else {
				fin=str.substr(0,i);
				paso=str.substr(i+10);
				if (paso.size()&&paso[0]==' ') paso=paso.substr(1); // a veces viene con doble espacio
			}
			if (ini.size()&&ini[0]=='(') { // suele venir envuelta en parentesis
				int par=1,i=0;
				for(unsigned int i=1;i<ini.size();i++) { 
					if (ini[i]=='(') par++;
					else if (ini[i]==')') { 
						par--;
						if (par==0 && i==ini.size()-1)
							ini=ini.substr(1,ini.size()-2);
						break;
					}
				}
			}
			aux=Add(ids,aux,new Entity(ET_PARA,var),0);
			aux->child[1]->SetLabel(ini);
			aux->child[2]->SetLabel(paso);
			aux->child[3]->SetLabel(fin);
		}
		else if (StartsWith(str,"SI ")) {
			aux=Add(ids,aux,new Entity(ET_SI,str.substr(3)),0);
		}
		else if (StartsWith(str,"SINO")) {
			aux=aux->parent; ids.pop(); ids.push(1);
		}
		else if (StartsWith(str,"SEGUN ")) {
			aux=Add(ids,aux,new Entity(ET_SEGUN,str.substr(6)),0);
		}
		else if (StartsWith(str,"OPCION ")) {
			if (aux->type==ET_SEGUN) { // si esta despues del segun, es el primer hijo
				aux=Add(ids,aux,new Entity(ET_OPCION,str.substr(7)),0);
			} else if (aux->type==ET_OPCION) {  // si esta despues de una opcion vacia, subir al segun
				aux=aux->parent; ids.pop();
				aux=Add(ids,aux,new Entity(ET_OPCION,str.substr(7)),aux->n_child);
			} else if (aux->type==ET_OPCION) { // si esta despues de una instruccion comun, subir a la opcion, subir al segun
				aux=aux->parent; ids.pop();
				aux=aux->parent; ids.pop();
				aux=Add(ids,aux,new Entity(ET_OPCION,str.substr(7)),aux->n_child);
			}
		}
		else if (StartsWith(str,"HASTA QUE ")) {
			aux=aux->parent; ids.pop(); aux->SetLabel(str.substr(10),false);
		}
		else if (str=="FINPARA"||str=="FINSI"||str=="FINMIENTRAS"||str=="FINSEGUN") {
			aux=aux->parent; ids.pop();
		}
		else { // asignacion, dimension, definicion
			aux=Add(ids,aux,new Entity(ET_ASIGNAR,str));
		}
	}
	aux->LinkNext(new Entity(ET_PROCESO,"FinProceso"));
	start->Calculate();
	ProcessMenu(MO_ZOOM_EXTEND);
	modified=false;
}


void Load() {
	d_dx=300; d_dy=680;
	
	start = new Entity(ET_PROCESO,"Inicio");
	
//	Entity *aux1 = new Entity(ET_SEGUN,"VARIABLE DE DESICION"); start->LinkNext(aux1);
//	Entity *aux1a = new Entity(ET_OPCION,"1"); aux1->InsertChild(0,aux1a);
//	Entity *aux1aa = new Entity(ET_ESCRIBIR,"1"); aux1a->LinkChild(0,aux1aa);
//	Entity *aux1b = new Entity(ET_OPCION,"2"); aux1->InsertChild(1,aux1b);
//	Entity *aux1ba = new Entity(ET_ESCRIBIR,"1"); aux1b->LinkChild(0,aux1ba);
//	Entity *aux1c = new Entity(ET_OPCION,"3"); aux1->InsertChild(2,aux1c);
	////	Entity *aux1ca = new Entity(ET_PARA,"1aslkfj asklhdskhfakjfhdskjfhdslkjhldsfa"); aux1c->LinkChild(0,aux1ca);
//	Entity *aux2 = new Entity(ET_PROCESO,"Fin"); aux1->LinkNext(aux2);
	
//	Entity *aux1 = new Entity(ET_LEER,"A,B"); start->LinkNext(aux1);
//	Entity *aux2 = new Entity(ET_ASIGNAR,"C<-RC(A^2+B^2)"); aux1->LinkNext(aux2);
//	Entity *aux3 = new Entity(ET_ESCRIBIR,"'Hipotenusa=',C"); aux2->LinkNext(aux3);
//	Entity *aux4 = new Entity(ET_SI,"SI"); aux3->LinkNext(aux4);
//	Entity *aux4a = new Entity(ET_ESCRIBIR,"LALA LALA"); aux4->LinkChild(0,aux4a);
//	Entity *aux4b = new Entity(ET_ESCRIBIR,"BOOGA"); aux4a->LinkNext(aux4b);
//	Entity *aux4c = new Entity(ET_ESCRIBIR,"FOO FOO FOO"); aux4->LinkChild(1,aux4c);
//	Entity *aux5 = new Entity(ET_PARA,"PARA"); aux4->LinkNext(aux5);
//	Entity *aux5a = new Entity(ET_ESCRIBIR,"HOLA"); aux5->LinkChild(0,aux5a);
//	Entity *aux6 = new Entity(ET_MIENTRAS,"MIENTRAS"); aux5->LinkNext(aux6);
//	Entity *aux6a = new Entity(ET_ESCRIBIR,"HOLA"); aux6->LinkChild(0,aux6a);
//	Entity *aux7 = new Entity(ET_REPETIR,"REPETIR"); aux6->LinkNext(aux7);
//	Entity *aux7a = new Entity(ET_ESCRIBIR,"HOLA"); aux7->LinkChild(0,aux7a);
//	Entity *aux8 = new Entity(ET_PROCESO,"Fin"); aux7->LinkNext(aux8);
	Entity *aux8 = new Entity(ET_PROCESO,"Fin"); start->LinkNext(aux8);
	start->Calculate();
	ProcessMenu(MO_ZOOM_EXTEND);
	modified=false;
}

