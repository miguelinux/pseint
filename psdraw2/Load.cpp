#include "Load.h"

#include <fstream>
#include <stack>
#include "Global.h"
#include "Entity.h"
#include "Events.h"
#include <iostream>
using namespace std;


static bool StartsWith(const string &s1, string s2) {
	if (s1.length()<s2.length()) return false;
	else return s1.substr(0,s2.length())==s2;
}

static Entity *Add(stack<int> &ids, Entity *vieja, Entity *nueva, int id=-1) {
	int mid=ids.top(); 
	ids.pop(); 
	if (nueva->type==ET_OPCION) ids.push(mid+(nueva->label=="De Otro Modo"?0:1)); 
	ids.push(-1);
	if (mid==-1) {
//		cerr<<"LinkNext( "<<vieja->label<<" , "<<nueva->label<<" )\n";
		vieja->LinkNext(nueva);
	} else { 
		if (nueva->type==ET_OPCION) {
			if (nueva->label=="De Otro Modo") {
//				cerr<<"ChildExists( "<<vieja->n_child-1<<" , "<<vieja->label<<" , "<<nueva->label<<" )\n";
				delete nueva; nueva=vieja->child[vieja->n_child-1]; // porque el child para dom se crea ya en el ctor del segun
			} else {
//				cerr<<"InsertChild( "<<mid<<" , "<<vieja->label<<" , "<<nueva->label<<" )\n";
				vieja->InsertChild(mid,nueva);
			}
		} else {
			vieja->LinkChild(mid,nueva);
//			cerr<<"LinkChild( "<<vieja->label<<" , "<<nueva->label<<" )\n";
		}
	}
	if (id!=-1) 
		ids.push(id);
	return nueva;
}

static Entity *Up(stack<int> &ids, Entity *vieja) {
//	cerr<<"Up: "<<vieja->label<<endl;
	int oid=ids.top();
	ids.pop(); 
	if (oid==-1) {
		return vieja->parent;
	} else { // si esperaba un hijo pero no vino nada (bucle vacio)
		if (vieja->type!=ET_OPCION && vieja->type!=ET_SEGUN) ids.push(-1); 
		return vieja;
	}
}

bool Load(const char *filename) {
	if (filename) fname=filename;
	else { New(); return false; }
	ifstream file(filename);
	if (!file.is_open()) { New(); return false; }
	string str;
	start = new Entity(ET_PROCESO,"Inicio");
	Entity *aux=start;
	stack<int> ids; ids.push(-1);
	while (getline(file,str)) {
//		stack<int> saux;
//		cerr<<"ids:";
//		while (!ids.empty()) { saux.push(ids.top()); ids.pop(); }
//		while (!saux.empty()) { ids.push(saux.top()); cerr<<" "<<saux.top(); saux.pop(); }
//		cerr<<endl;
//		cerr<<str<<endl;
		if (str.size() && str[str.size()-1]==';') str=str.substr(0,str.size()-1);
		bool comillas=false;
		for (unsigned int i=0;i<str.size();i++) {
			if (str[i]=='\''||str[i]=='\"') comillas=!comillas;
			else if (!comillas) {
				if (str[i]=='&') { str.replace(i,1," & "); i+=2; }
				else if (str[i]=='|') { str.replace(i,1," | "); i+=2; }
			}
		}
		if (StartsWith(str,"PROCESO ")) {
			pname = str.substr(8);
			continue;
		}
		if (!str.size()||str=="FINPROCESO"||str=="ENTONCES") {
			continue;
		}
		else if (StartsWith(str,"ESCRIBIR ")) {
			aux=Add(ids,aux,new Entity(ET_ESCRIBIR,str.substr(9)));
		}
		else if (StartsWith(str,"LEER ")) {
			aux=Add(ids,aux,new Entity(ET_LEER,str.substr(5)));
		}
		else if (StartsWith(str,"HASTA QUE ")) {
			aux=Up(ids,aux); aux->SetLabel(str.substr(10),false);
		}
		else if (StartsWith(str,"MIENTRAS QUE ")) {
			aux=Up(ids,aux); aux->SetLabel(str.substr(13),false);
			aux->variante=true;
		}
		else if (StartsWith(str,"MIENTRAS ")) {
			aux=Add(ids,aux,new Entity(ET_MIENTRAS,str.substr(9,str.size()-14)),0);
		}
		else if (str=="REPETIR") {
			aux=Add(ids,aux,new Entity(ET_REPETIR,""),0);
		}
		else if (StartsWith(str,"PARA ")) {
			str=str.substr(5,str.size()-11);
			size_t i=str.find("<-");
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
				int par=1;
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
		else if (StartsWith(str,"PARACADA ")) {
			str=str.substr(9,str.size()-15);
			size_t i=str.find(" ");
			string var=str.substr(0,i);
			str=str.substr(i+1);
			i=str.find(" ");
			string ini=str.substr(0,i);
			str=str.substr(i+1);
			aux=Add(ids,aux,new Entity(ET_PARA,var),0);
			aux->variante=true;
//			aux->child[1]->SetLabel("");
			aux->child[2]->SetLabel(str);
//			aux->child[3]->SetLabel("");
		}
		else if (StartsWith(str,"SI ")) {
			aux=Add(ids,aux,new Entity(ET_SI,str.substr(3)),1);
		}
		else if (StartsWith(str,"SINO")) {
			aux=aux->parent; ids.pop(); ids.push(0);
		}
		else if (StartsWith(str,"SEGUN ")) {
			aux=Add(ids,aux,new Entity(ET_SEGUN,str.substr(6,str.size()-6-5)),0);
		}
		else if (str.size() && str[str.size()-1]==':') {
			str.erase(str.size()-1,1);
			if (aux->type==ET_SEGUN) { // si esta despues del segun, es el primer hijo
				aux=Add(ids,aux,new Entity(ET_OPCION,str),0);
			} else {
				aux=Up(ids,aux); // sube a la opcion
				aux=Up(ids,aux); // sube al segun
				if (str=="DE OTRO MODO") str="De Otro Modo";
				aux=Add(ids,aux,new Entity(ET_OPCION,str),0);
			}
		}
		else if (str=="FINPARA"||str=="FINSI"||str=="FINMIENTRAS"||str=="FINSEGUN") {
			if (str=="FINSEGUN" && aux->type!=ET_SEGUN) { aux=Up(ids,aux); aux=Up(ids,aux); }
			aux=Up(ids,aux);
		}
		else { // asignacion, dimension, definicion
			aux=Add(ids,aux,new Entity(ET_ASIGNAR,str));
		}
	}
	aux->LinkNext(new Entity(ET_PROCESO,"FinProceso"));
	start->Calculate();
	ProcessMenu(MO_ZOOM_EXTEND);
	modified=false;
	return true;
}


bool Save(const char *filename) {
	if (filename) fname=filename;
	ofstream fout(fname.c_str());
	if (!fout.is_open()) return false;
	start->Print(fout);
	fout.close();
	modified=false;
	return true;
}

void New() {
	fname="temp.psd"; pname="SinTitulo";
	start = new Entity(ET_PROCESO,"Inicio");
	Entity *aux8 = new Entity(ET_PROCESO,"Fin"); start->LinkNext(aux8);
	start->Calculate();
	ProcessMenu(MO_ZOOM_EXTEND);
	modified=false;
}

