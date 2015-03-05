#include <fstream>
#include <stack>
#include <iostream>
#include <sstream>
#include "Load.h"
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
	if (nueva->type==ET_OPCION) ids.push(mid+(nueva->label=="De Otro Modo"?0:1)); 
	ids.push(-1);
	if (mid==-1) {
//cerr<<"LinkNext( "<<vieja->label<<" , "<<nueva->label<<" )\n";
		vieja->LinkNext(nueva);
	} else { 
		if (nueva->type==ET_OPCION) {
			if (nueva->label=="De Otro Modo") {
//cerr<<"ChildExists( "<<vieja->n_child-1<<" , "<<vieja->label<<" , "<<nueva->label<<" )\n";
				delete nueva; nueva=vieja->child[vieja->n_child-1]; // porque el child para dom se crea ya en el ctor del segun
			} else {
//cerr<<"InsertChild( "<<mid<<" , "<<vieja->label<<" , "<<nueva->label<<" )\n";
				vieja->InsertChild(mid,nueva);
			}
		} else {
			vieja->LinkChild(mid,nueva);
//cerr<<"LinkChild( "<<vieja->label<<" , "<<nueva->label<<" )\n";
		}
	}
	if (id!=-1) 
		ids.push(id);
	return nueva;
}

static Entity *Up(stack<int> &ids, Entity *vieja) {
//cerr<<"Up: "<<vieja->label<<endl;
	int oid=ids.top();
	ids.pop(); 
	if (oid==-1) {
		return vieja->parent;
	} else { // si esperaba un hijo pero no vino nada (bucle vacio)
		if (vieja->type!=ET_OPCION && vieja->type!=ET_SEGUN) ids.push(-1); 
		return vieja;
	}
}

static void RemoveParentesis(string &ini) {
	if (ini.size()&&ini[0]=='(') { // suele venir envuelta en parentesis
		int par=1;
		for(unsigned int i=1;i<ini.size();i++) { 
			if (ini[i]=='(') par++;
			else if (ini[i]==')') {
				par--;
				if (par==0 && i==ini.size()-1)
					ini=ini.substr(1,ini.size()-2);
				if (par==0) break;
			}
		}
	}
}

static void ReemplazarOperadores(string &str) {
	bool comillas=false;
	for(unsigned int i=0;i<str.size();i++) { 
		if (str[i]=='\'' || str[i]=='\"') comillas=!comillas;
		else if (!comillas) {
			if (str[i]=='&') str.replace(i,1," Y ");
			else if (str[i]=='|') str.replace(i,1," O ");
			else if (str[i]=='~') str.replace(i,1," NO ");
			else if (str[i]=='%') str.replace(i,1," MOD ");
		}
	}
}

#define _new_this(e) if (cur_pos.size()) code2draw[cur_pos]=LineInfo(cur_proc,e)
#define _new_prev() if (cur_pos.size()) code2draw[cur_pos]=LineInfo(NULL,cur_proc)
#define _new_none() if (cur_pos.size()) code2draw[cur_pos]=LineInfo(NULL,NULL)

void LoadProc(istream &fin) {
	code2draw.clear();
	string cur_pos; Entity *cur_proc; // para llenar code2draw
	
	string str;
	cur_proc = start = new Entity(ET_PROCESO,"SinTitulo");
	Entity *aux=start;
	stack<int> ids; ids.push(-1);
	while (getline(fin,str)) {
		//stack<int> saux;
		//cerr<<"ids:";
		//while (!ids.empty()) { saux.push(ids.top()); ids.pop(); }
		//while (!saux.empty()) { ids.push(saux.top()); cerr<<" "<<saux.top(); saux.pop(); }
		//cerr<<endl;
		//cerr<<str<<endl;
		if (str.size() && str[str.size()-1]==';') str=str.substr(0,str.size()-1);
		if (lang[LS_WORD_OPERATORS]) ReemplazarOperadores(str);
		bool comillas=false;
		for (unsigned int i=0;i<str.size();i++) {
			if (str[i]=='\''||str[i]=='\"') comillas=!comillas;
			else if (!comillas) {
				if (str[i]=='&') { str.replace(i,1," & "); i+=2; }
				else if (str[i]=='|') { str.replace(i,1," | "); i+=2; }
			}
		}
		if (StartsWith(str,"#pos ")) {
			cur_pos=str.substr(5);
		}
		else if (StartsWith(str,"PROCESO ")||StartsWith(str,"SUBPROCESO ")) {
			string s1=str.substr(0,str.find(' '));
			string s2=str.substr(str.find(' ')+1);
			if (s1=="PROCESO") { start->lpre="Proceso "; start->SetLabel(s2); }
			else if (s1=="SUBPROCESO") { start->lpre="SubProceso "; start->SetLabel(s2); }
			_new_this(start); cur_proc=start;
			continue;
		}
		else if (str=="FINPROCESO") {
			_new_none();
			continue;
		}
		else if (!str.size()||str=="FINSUBPROCESO"||str=="ENTONCES") {
			_new_prev();
			continue;
		}
		else if (StartsWith(str,"INVOCAR ")) {
			aux=Add(ids,aux,new Entity(ET_ASIGNAR,str.substr(8)));
			_new_this(aux);
		}
		else if (StartsWith(str,"ESCRIBIR ")) {
			aux=Add(ids,aux,new Entity(ET_ESCRIBIR,str.substr(9)));
			_new_this(aux);
		}
		else if (StartsWith(str,"ESCRIBNL ")) {
			aux=Add(ids,aux,new Entity(ET_ESCRIBIR,str.substr(9)));
			aux->variante=true;
			_new_this(aux);
		}
		else if (StartsWith(str,"LEER ")) {
			aux=Add(ids,aux,new Entity(ET_LEER,str.substr(5)));
			_new_this(aux);
		}
		else if (StartsWith(str,"HASTA QUE ")) {
			aux=Up(ids,aux); aux->SetLabel(str.substr(10),false);
			_new_this(aux);
		}
		else if (StartsWith(str,"MIENTRAS QUE ")) {
			aux=Up(ids,aux); aux->SetLabel(str.substr(13),false);
			aux->variante=true;
			_new_this(aux);
		}
		else if (StartsWith(str,"MIENTRAS ")) {
			aux=Add(ids,aux,new Entity(ET_MIENTRAS,str.substr(9,str.size()-15)),0);
			_new_this(aux);
		}
		else if (str=="REPETIR") {
			aux=Add(ids,aux,new Entity(ET_REPETIR,""),0);
			_new_prev();
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
				paso=""; fin=str;
			} else {
				fin=str.substr(0,i);
				paso=str.substr(i+10);
				if (paso.size()&&paso[0]==' ') paso=paso.substr(1); // a veces viene con doble espacio
			}
			RemoveParentesis(ini);
			aux=Add(ids,aux,new Entity(ET_PARA,var),0);
			aux->child[1]->SetLabel(ini);
			aux->child[2]->SetLabel(paso);
			aux->child[3]->SetLabel(fin);
			_new_this(aux);
		}
		else if (StartsWith(str,"PARACADA ")) {
			str=str.substr(9,str.size()-15);
			size_t i=str.find(" ");
			string var=str.substr(0,i);
			str=str.substr(i+1);
			i=str.find(" ");
//			string ini=str.substr(0,i);
			str=str.substr(i+1);
			aux=Add(ids,aux,new Entity(ET_PARA,var),0);
			aux->variante=true;
//			aux->child[1]->SetLabel("");
			aux->child[2]->SetLabel(str);
//			aux->child[3]->SetLabel("");
			_new_this(aux);
		}
		else if (StartsWith(str,"SI ")) {
			aux=Add(ids,aux,new Entity(ET_SI,str.substr(3)),1);
			_new_this(aux);
		}
		else if (StartsWith(str,"SINO")) {
			aux=aux->parent; ids.pop(); ids.push(0);
			_new_prev();
		}
		else if (StartsWith(str,"SEGUN ")) {
			aux=Add(ids,aux,new Entity(ET_SEGUN,str.substr(6,str.size()-6-5)),0);
			_new_this(aux);
		}
		else if (str.size() && str[str.size()-1]==':') {
			str.erase(str.size()-1,1);
			if (aux->type==ET_SEGUN && ids.top()!=-1) { // si esta despues del segun, es el primer hijo
				aux=Add(ids,aux,new Entity(ET_OPCION,str),0);
			} else {
				aux=Up(ids,aux); // sube a la opcion
				aux=Up(ids,aux); // sube al segun
				if (str=="DE OTRO MODO") str="De Otro Modo";
				aux=Add(ids,aux,new Entity(ET_OPCION,str),0);
			}
			_new_this(aux);
		}
		else if (str=="FINPARA"||str=="FINSI"||str=="FINMIENTRAS"||str=="FINSEGUN") {
			if (str=="FINSEGUN" && aux->type!=ET_SEGUN) { aux=Up(ids,aux); aux=Up(ids,aux); }
			aux=Up(ids,aux);
			_new_prev();
		}
		else { // asignacion, dimension, definicion
			int i=0,l=str.size();
			while (i<l && ((str[i]>='a'&&str[i]<='z')||(str[i]>='A'&&str[i]<='Z')||(str[i]>='0'&&str[i]<='9')||str[i]=='_')) i++;
			if (i+2<l && str.substr(i,2)=="<-") {
				string s1=str.substr(0,i+2);
				string s2=str.substr(i+2);
				RemoveParentesis(s2);
				str=s1+s2;
			}
			if (str=="ESPERARTECLA") str="Esperar Tecla";
			else if (str=="BORRARPANTALLA") str="Borrar Pantalla";
			else if (str.find(' ')!=string::npos) {
				size_t p=str.find(' '); string aux=str.substr(0,p);
				if (aux=="DEFINIR") {
					size_t p2=str.rfind("COMO "); string aux2=str.substr(p2);
					if (aux2=="COMO CARACTER") aux2="Como Caracter";
					else if (aux2=="COMO LOGICO") aux2="Como Logico";
					else if (aux2=="COMO ENTERO") aux2="Como Entero";
					else if (aux2=="COMO REAL") aux2="Como Real";
					str.erase(p2);
					str=string("Definir ")+str.substr(p+1)+aux2;
				}
				else if (aux=="DIMENSION") str=string("Dimension ")+str.substr(p+1);
			}
			aux=Add(ids,aux,new Entity(ET_ASIGNAR,str));
			_new_this(aux);
		}
	}
	Entity *efin=new Entity(ET_PROCESO,""); 
	efin->lpre=string("Fin")+start->lpre.substr(0,start->lpre.size()-1);
	efin->SetLabel("");
	aux->LinkNext(efin);
}

bool Load(const char *filename) {
	loading=true;
	if (filename) fname=filename;
	else { New(); return false; }
	ifstream file(filename);
	if (!file.is_open()) { New(); return false; }
	string str, lstr; int imain=0;
	while (getline(file,str)) {
		if (StartsWith(str,"PROCESO ")||StartsWith(str,"SUBPROCESO ")||StartsWith(str,"FUNCION ")||StartsWith(str,"FUNCIÓN ")) {
			if (StartsWith(str,"PROCESO ")) imain=procesos.size();
			Entity::all_any=start=NULL;
			stringstream ss;
			ss<<lstr<<"\n"<<str<<"\n";
			while (getline(file,str)) {
				ss<<str<<"\n";
				if (str=="FINPROCESO"||str=="FINSUBPROCESO"||str=="FINFUNCION"||str=="FINFUNCIÓN") break;
			}
			LoadProc(ss);
			procesos.push_back(start);
		}
		lstr=str;
	}
	SetProc(procesos[choose_process_sel=imain]);
	choose_process_state=procesos.size()>1?2:0;
	loading=false; modified=false;
	return true;
}

bool Save(const char *filename) {
	if (filename) fname=filename;
	ofstream fout(fname.c_str());
	if (!fout.is_open()) return false;
	code2draw.clear(); debug_current=NULL;
	int line=1;
	for(unsigned int i=0;i<procesos.size();i++) {
		procesos[i]->Print(fout,"",procesos[i],line);
		fout<<endl; line++;
	}
	fout.close();
	modified=false;
	return true;
}

// inicializa las estructuras de datos con un algoritmo en blanco
void CreateEmptyProc(string type) {
	Entity::all_any=NULL;
	start = new Entity(ET_PROCESO,"");
	Entity *aux = new Entity(ET_PROCESO,"");
	start->LinkNext(aux);
	start->lpre=type+" "; start->SetLabel("SinTitulo");
	aux->lpre=string("Fin")+type; aux->SetLabel("");
	start->Calculate();
	procesos.push_back(start);
}

void New() {
	fname="temp.psd"; 
	CreateEmptyProc("Proceso");
	ProcessMenu(MO_ZOOM_EXTEND);
	modified=false;
}

void SetProc(Entity *proc) {
	Entity::all_any=start=proc;
	start->Calculate();
	ProcessMenu(MO_ZOOM_EXTEND);
//	pname=proc->prototipo;
	Entity *ent=start, *ent2=start;
	do {
		ent->d_h=ent->d_w=ent->d_bh=ent->d_bwl=ent->d_bwr=ent->d_fx=ent->d_fy=ent->d_x=ent->d_y=0;
		ent=ent->all_next;
	} while(ent!=ent2);
}
