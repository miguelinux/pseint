#include <fstream>
#include <stack>
#include <iostream>
#include <sstream>
#include "Load.h"
#include "Global.h"
#include "Entity.h"
#include "Events.h"
#include "ProcessSelector.h"
#include "../wxPSeInt/CommonParsingFunctions.h"
using namespace std;


static bool StartsWith(const string &s1, string s2) {
	if (s1.length()<s2.length()) return false;
	else return s1.substr(0,s2.length())==s2;
}

// Agrega una nueva entidad donde corresponda. El "donde corresponda" sería
// o bien como hijo de la previa, o bien como siguiente de la previa.
// Para decidir cual corresponde está children_stack, que tiene en su top
// -1 si va como siguiente, o el indice de hijo si va como hija.
// El id que recibe es para indicar a dónde debería ir la próxima... es decir
// si recibo -1 (escribir, leer, dimension,... la próxima) irá como siguiente
// mientras que si recibo otra cosa la próxima irá como hija en ese lugar (0
// para Para, Repetir, Mientras; 1 para Si; 0 a X para Segun; etc).

static Entity *Add(stack<int> &children_stack, Entity *previa, Entity *nueva, int child_id_for_next=-1) {
	int where = children_stack.top(); 
	children_stack.pop(); 
	if (nueva->type==ET_OPCION) { assert(previa->type==ET_SEGUN); children_stack.push(where+(nueva->label=="De Otro Modo"?0:1)); }
	children_stack.push(-1);
	if (where==-1) {
		previa->LinkNext(nueva);
	} else { 
		if (nueva->type==ET_OPCION) {
			if (nueva->label=="De Otro Modo") {
				delete nueva; nueva=previa->GetChild(previa->GetChildCount()-1); // porque el child para dom se crea ya en el ctor del segun
			} else {
				previa->InsertChild(where,nueva);
			}
		} else {
			previa->LinkChild(where,nueva);
		}
	}
	if (child_id_for_next!=-1) 
		children_stack.push(child_id_for_next);
	return nueva;
}


// Sube un nivel en el children_stack. Se usa por ej cuando se cierra 
// alguna estructura de control
static Entity *Up(stack<int> &children_stack, Entity *previa) {
	int where = children_stack.top();
	children_stack.pop(); 
	if (where==-1) {
		return previa->GetParent();
	} else { // si esperaba un hijo pero no vino nada (bucle vacio)
//		if (previa->type!=ET_OPCION && previa->type!=ET_SEGUN) children_stack.push(-1); 
		return previa;
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
			if (str[i]=='&') str.replace(i,str[i+1]=='&'?2:1," Y ");
			else if (str[i]=='|') str.replace(i,str[i+1]=='|'?2:1," O ");
			else if (str[i]=='~') str.replace(i,1," NO ");
			else if (str[i]=='%') str.replace(i,1," MOD ");
		}
	}
}

#define _new_this(e) if (cur_pos.size()) code2draw[cur_pos]=LineInfo(cur_proc,e)
#define _new_prev() if (cur_pos.size()) code2draw[cur_pos]=LineInfo(NULL,cur_proc)
#define _new_none() if (cur_pos.size()) code2draw[cur_pos]=LineInfo(NULL,NULL)

void LoadProc(vector<string> &vproc) {
	code2draw.clear();
	string cur_pos; Entity *cur_proc; // para llenar code2draw
	bool start_done=false;
	cur_proc = start = new Entity(ET_PROCESO,"SinTitulo");
	Entity *aux=start, *aux_end=start;
	// child_id guarda en qué lugar de los hijos de la entidad anterior hay que
	// enlazar la próxima, o -1 si la próxima va como next y no como hijo
	stack<int> children_stack; children_stack.push(-1);
	for(size_t iv=0;iv<vproc.size();iv++) { 
		string str = vproc[iv];
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
			if (s1=="PROCESO") { 
				start->lpre=lang[LS_PREFER_ALGORITMO]?"Algoritmo ":"Proceso "; 
				start->SetLabel(s2); 
			} else if (s1=="SUBPROCESO") { 
				start->lpre=lang[LS_PREFER_FUNCION]?"Funcion ":(lang[LS_PREFER_ALGORITMO]?"SubAlgoritmo ":"SubProceso "); 
				start->SetLabel(s2); 
			}
			_new_this(start); /*cur_proc=start;*/ start_done=true;
			continue;
		}
		else if (str=="FINPROCESO") {
			aux_end=aux;
			_new_none();
			continue;
		}
		else if (!str.size()||str=="FINSUBPROCESO"||str=="ENTONCES") {
			aux_end=aux;
			_new_prev();
			continue;
		}
		else if (StartsWith(str,"INVOCAR ")) {
			aux=Add(children_stack,aux,new Entity(ET_ASIGNAR,str.substr(8)));
			aux->variante=true;
			_new_this(aux);
		}
		else if (StartsWith(str,"#comment") && aux && aux->type==ET_SEGUN) {
			// no puedo poner comentarios a las opciones del segun... por ahora el 
			// parche es bajarlos hasta despues de la opcion, o sea ponerlos dentro
			// de su rama, para al menos no perderlos
			if (StartsWith(str,"#comment-inline ")) vproc[iv].erase(8,7);
			int j = iv+1;
			while (StartsWith(vproc[j],"#comment")) ++j;
			string opcion = vproc[j];
			vproc.erase(vproc.begin()+j);
			vproc.insert(vproc.begin()+iv,opcion);
			--iv; continue;
		}
		else if (StartsWith(str,"#comment ")) {
			aux=Add(children_stack,aux,new Entity(ET_COMENTARIO,str.substr(9)));
			if (!start_done) { start->UnLink(); aux->LinkNext(start); aux=start; }
		}
		else if (StartsWith(str,"#comment-inline ")) {
			if (aux && aux->type==ET_SEGUN) continue;
			aux=Add(children_stack,aux,new Entity(ET_COMENTARIO,str.substr(16)));
			aux->variante=true;
			if (!start_done) { start->UnLink(); aux->LinkNext(start); aux=start; }
		}
		else if (StartsWith(str,"ESCRIBIR ")) {
			aux=Add(children_stack,aux,new Entity(ET_ESCRIBIR,str.substr(9)));
			_new_this(aux);
		}
		else if (StartsWith(str,"ESCRIBNL ")) {
			aux=Add(children_stack,aux,new Entity(ET_ESCRIBIR,str.substr(9)));
			aux->variante=true;
			_new_this(aux);
		}
		else if (StartsWith(str,"LEER ")) {
			aux=Add(children_stack,aux,new Entity(ET_LEER,str.substr(5)));
			_new_this(aux);
		}
		else if (StartsWith(str,"HASTA QUE ")) {
			aux=Up(children_stack,aux); aux->SetLabel(str.substr(10),false);
			_new_this(aux);
		}
		else if (StartsWith(str,"MIENTRAS QUE ")) {
			aux=Up(children_stack,aux); aux->SetLabel(str.substr(13),false);
			aux->variante=true;
			_new_this(aux);
		}
		else if (StartsWith(str,"MIENTRAS ")) {
			aux=Add(children_stack,aux,new Entity(ET_MIENTRAS,str.substr(9,str.size()-15)),0);
			_new_this(aux);
		}
		else if (str=="REPETIR") {
			aux=Add(children_stack,aux,new Entity(ET_REPETIR,""),0);
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
			aux=Add(children_stack,aux,new Entity(ET_PARA,var),0);
			aux->GetChild(1)->SetLabel(ini);
			aux->GetChild(2)->SetLabel(paso);
			aux->GetChild(3)->SetLabel(fin);
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
			aux=Add(children_stack,aux,new Entity(ET_PARA,var),0);
			aux->variante=true;
//			aux->child[1]->SetLabel("");
			aux->GetChild(2)->SetLabel(str);
//			aux->child[3]->SetLabel("");
			_new_this(aux);
		}
		else if (StartsWith(str,"SI ")) {
			aux=Add(children_stack,aux,new Entity(ET_SI,str.substr(3)),1);
			_new_this(aux);
		}
		else if (StartsWith(str,"SINO")) {
			aux=aux->GetParent(); children_stack.pop(); children_stack.push(0);
			_new_prev();
		}
		else if (StartsWith(str,"SEGUN ")) {
			aux=Add(children_stack,aux,new Entity(ET_SEGUN,str.substr(6,str.size()-6-5)),0);
			_new_this(aux);
		}
		else if (str.size() && str[str.size()-1]==':') {
			str.erase(str.size()-1,1);
			if (aux->type==ET_SEGUN && children_stack.top()!=-1) { // si esta despues del segun, es el primer hijo
				aux=Add(children_stack,aux,new Entity(ET_OPCION,str),0);
			} else {
				aux=Up(children_stack,aux); // sube a la opcion
				aux=Up(children_stack,aux); // sube al segun
				if (str=="DE OTRO MODO") str="De Otro Modo";
				aux=Add(children_stack,aux,new Entity(ET_OPCION,str),0);
			}
			_new_this(aux);
		}
		else if (str=="FINPARA"||str=="FINSI"||str=="FINMIENTRAS"||str=="FINSEGUN") {
			if (str=="FINSEGUN" && aux->type!=ET_SEGUN) { aux=Up(children_stack,aux); aux=Up(children_stack,aux); }
			aux=Up(children_stack,aux);
			_new_prev();
		}
		else { // asignacion, dimension, definicion
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
			aux=Add(children_stack,aux,new Entity(ET_ASIGNAR,str));
			_new_this(aux);
		}
	}
	Entity *efin=new Entity(ET_PROCESO,""); 
	efin->variante=true;
	efin->lpre=string("Fin")+start->lpre.substr(0,start->lpre.size()-1);
	efin->SetLabel("");
	aux_end->LinkNext(efin);
}

bool Load(const char *filename) {
	// cargar todo el algoritmo en un vector
	loading=true;
	if (filename) fname=filename;
	else { New(); return false; }
	ifstream file(filename);
	if (!file.is_open()) { New(); return false; }
	string str; 
	vector<string> vfile; while (getline(file,str)) vfile.push_back(str);
	file.close();
	
	// separar por procesos/subprocesos
	int imain=0;
	int i0=0; // posición en el vector donde empieza el proceso
	for(unsigned int i=0;i<vfile.size();i++) {
		string &str=vfile[i];
		if (StartsWith(str,"PROCESO ")||StartsWith(str,"SUBPROCESO ")||StartsWith(str,"FUNCION ")||StartsWith(str,"FUNCIÓN ")) {
			if (StartsWith(str,"PROCESO ")) imain=procesos.size();
			Entity::AllSet(start=NULL);
			vector<string> vproc;
			for(int j=i0;;j++) {
				string &str=vfile[j];
				vproc.push_back(str);
				if (str=="FINPROCESO"||str=="FINSUBPROCESO"||str=="FINFUNCION"||str=="FINFUNCIÓN") { i0=j+1; break; }
			}
			LoadProc(vproc);
			Entity::AllIterator it = Entity::AllBegin();
			while (it!=Entity::AllEnd()) {
				it->EditLabel(0);
				++it;
			}
			procesos.push_back(start);
		}
	}
	SetProc(procesos[imain]);
	loading=false; modified=false;
#ifndef _FOR_EXPORT
	if (procesos.size()>1?2:0) process_selector->Show();
#endif
	return true;
}

bool Save(const char *filename) {
	if (filename) fname=filename;
	ofstream fout(fname.c_str());
	if (!fout.is_open()) return false;
	code2draw.clear(); debug_current=NULL;
	int line=1;
	for(unsigned int i=0;i<procesos.size();i++) {
		procesos[i]->GetTopEntity()->Print(fout,"",procesos[i],line);
		fout<<endl; line++;
	}
	fout.close();
	modified=false;
	return true;
}

// inicializa las estructuras de datos con un algoritmo en blanco
void CreateEmptyProc(string type) {
	Entity::AllSet(NULL);
	start = new Entity(ET_PROCESO,"");
	Entity *aux = new Entity(ET_PROCESO,"");
	aux->variante=true;
	start->LinkNext(aux);
	start->lpre=type+" "; start->SetLabel("SinTitulo");
	aux->lpre=string("Fin")+type; aux->SetLabel("");
	Entity::CalculateAll();
	procesos.push_back(start);
}

void New() {
	fname="temp.psd"; 
	CreateEmptyProc(lang[LS_PREFER_ALGORITMO]?"Algoritmo":"Proceso");
	ProcessMenu(MO_ZOOM_EXTEND);
	modified=false;
}

void SetProc(Entity *proc) {
	Entity::AllSet(start=proc);
	Entity::CalculateAll(true);
	ProcessMenu(MO_ZOOM_EXTEND);
	Entity::AllIterator it = Entity::AllBegin();
	while (it!=Entity::AllEnd()) {
		it->d_h=it->d_w=it->d_bh=it->d_bwl=it->d_bwr=it->d_fx=it->d_fy=it->d_x=it->d_y=0;
		++it;
	}
}
