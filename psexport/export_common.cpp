#include "export_common.h"
#include "../pseint/utils.h"
#include "exportexp.h"
#include "../pseint/new_evaluar.h"
#include <cstdlib>
#include "version.h"
#include "../pseint/new_funciones.h"
using namespace std;

map<string,Memoria*> ExporterBase::mapa_memorias;

ExporterBase *exporter=NULL;

string ExporterBase::make_dims(const int *tdims, string c1, string c2, string c3, bool numbers) {
	string dims=c1;
	for (int j=1;j<=tdims[0];j++) {
		if (j==tdims[0]) dims+=(numbers?IntToStr(tdims[j]):"")+c3;
		else dims+=(numbers?IntToStr(tdims[j]):"")+c2;
	}
	return dims;
}


void ExporterBase::bloque(t_output &prog, t_proceso_it r, t_proceso_it q,string tabs){
	if (r==q) return;
	int deep;
	string s;
	
	while (r!=q) {
		s=(*r).nombre;
		if (s=="#comment" || s=="#comment-inline") {
			comentar(prog,(*r).par1,tabs);
		}
		else if (s=="ESCRIBIR") {
			t_arglist args; string param=(*r).par1+",";
			bool comillas=false, saltar=true;
			int parentesis=0, lastcoma=0;
			for (unsigned int i=0;i<param.size();i++) {
				if (param[i]=='\'') {
					comillas=!comillas;
					param[i]='"';
				} else if (!comillas) {
					if (param[i]=='(') parentesis++;
					else if (param[i]==')') parentesis--;
					else if (parentesis==0 && param[i]==',') {
						string expr=param.substr(lastcoma,i-lastcoma);
						if (expr=="**SINSALTAR**") saltar=false;
						else args.push_back(expr); 
						lastcoma=i+1;
					}
				}
			}
			escribir(prog,args,saltar,tabs);
		}
		else if (s=="INVOCAR") invocar(prog,(*r).par1,tabs);
		else if (s=="BORRARPANTALLA") borrar_pantalla(prog,(*r).par1,tabs);
		else if (s=="ESPERARTECLA") esperar_tecla(prog,(*r).par1,tabs);
		else if (s=="ESPERAR") 
			esperar_tiempo(prog,(*r).par1,(*r).par2.substr(0,4)=="MILI",tabs);
		else if (s=="DIMENSION") {
			t_arglist arglist;
			
			string params=(*r).par1+",";
			while (params.size()>1) {
				unsigned int i=1;
				while (params[i]!=')') i++;
				while (params[i]!=',') i++;
				arglist.push_back(params.substr(0,i));
				params.erase(0,i+1);
			}
			dimension(prog,arglist,tabs);
		}
		else if (s=="DEFINIR") {
			string param=(*r).par1;
			string tipo=param.substr(param.rfind(" ")+1);
			param.erase(param.size()-tipo.size()-6);
			int lastcoma=0;	param+=",";
			t_arglist arglist;
			for (unsigned int i=0;i<param.size();i++) {
				if (param[i]==',') {
					string varname=ToUpper(param.substr(lastcoma,i-lastcoma));
					if (tipo=="ENTERO") memoria->DefinirTipo(varname,vt_numerica,true);
					else if (tipo=="REAL") memoria->DefinirTipo(varname,vt_numerica,false);
					else if (tipo=="CARACTER") memoria->DefinirTipo(varname,vt_caracter,false);
					else if (tipo=="LOGICO") memoria->DefinirTipo(varname,vt_logica,false);
					arglist.push_back(/*expresion(*/varname/*)*/);
					lastcoma=i+1;
				}
			}
			definir(prog,arglist,tipo,tabs);
		}
		else if (s=="LEER") {
			t_arglist args;
			bool comillas=false; string param=(*r).par1+",";
			int parentesis=0, lastcoma=0;
			for (unsigned int i=0;i<param.size();i++) {
				if (param[i]=='\'') {
					comillas=!comillas;
					param[i]='"';
				} else if (!comillas) {
					if (param[i]=='(') parentesis++;
					else if (param[i]==')') parentesis--;
					else if (parentesis==0 && param[i]==',') {
						string varname=param.substr(lastcoma,i-lastcoma);
						args.push_back(varname);
						lastcoma=i+1;
						// para que registre las variables en la memoria?
						if (varname.find('(')==string::npos) {
							tipo_var t;
							memoria->DefinirTipo(varname,t);
						}
					}
				}
			}
			leer(prog,args,tabs);
		}
		else if (s=="ASIGNACION") {
			tipo_var t;
			string param1=/*expresion(*/(*r).par1/*)*/;
			string param2=/*expresion(*/(*r).par2/*,t)*/;
			asignacion(prog,param1,param2,tabs);
//			memoria->DefinirTipo(param1,t);
		}
		else if (s=="MIENTRAS") {
			t_proceso_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINMIENTRAS"  && deep==0) ) {
				if ((*r).nombre=="MIENTRAS") deep++;
				else if ((*r).nombre=="FINMIENTRAS") deep--;
				++r;
			}
			mientras(prog,r1,r,tabs);
		} else if (s=="REPETIR") {
			t_proceso_it r1=r++;
			deep=0;
			while ( ! (((*r).nombre=="HASTAQUE"||(*r).nombre=="MIENTRASQUE")  && deep==0) ) {
				if ((*r).nombre=="REPETIR") deep++;
				else if ((*r).nombre=="HASTAQUE") deep--;
				else if ((*r).nombre=="MIENTRASQUE") deep--;
				++r;
			}
			repetir(prog,r1,r,tabs);
		} else if (s=="SEGUN") {
			list<t_proceso_it> its;
			its.insert(its.end(),r);
			deep=0;
			++r;
			while ( ! ((*r).nombre=="FINSEGUN" && deep==0) ) {
				if (deep==0 && (*r).nombre=="OPCION") 
					its.insert(its.end(),r);
				else if ((*r).nombre=="SEGUN") 
					deep++;
				else if ((*r).nombre=="FINSEGUN")
					deep--;
				++r;
			}
			its.insert(its.end(),r);
			segun(prog,its,tabs);
		} else if (s=="PARA") {
			t_proceso_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINPARA" && deep==0) ) {
				if ((*r).nombre=="PARA"||(*r).nombre=="PARACADA") deep++;
				else if ((*r).nombre=="FINPARA") deep--;
				++r;
			}
			memoria->DefinirTipo(ToLower((*r1).par1),vt_numerica);
			para(prog,r1,r,tabs);
		} else if (s=="PARACADA") {
			t_proceso_it r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINPARA" && deep==0) ) {
				if ((*r).nombre=="PARA"||(*r).nombre=="PARACADA") deep++;
				else if ((*r).nombre=="FINPARA") deep--;
				++r;
			}
			paracada(prog,r1,r,tabs);
			memoria->DefinirTipo(ToLower((*r1).par2),memoria->LeerTipo(ToLower((*r1).par1)));
			memoria->RemoveVar(ToLower((*r1).par1));
		} else if (s=="SI") {
			t_proceso_it r1=r++,r2;
			++r;
			deep=0;
			while ( ! ( ( (*r).nombre=="FINSI" || (*r).nombre=="SINO") && deep==0) ) {
				if ((*r).nombre=="SI") deep++;
				else if ((*r).nombre=="FINSI") deep--;
				++r;
			}
			r2=r;
			while ( ! ( (*r).nombre=="FINSI" && deep==0) ) {
				if ((*r).nombre=="SI") deep++;
				else if ((*r).nombre=="FINSI") deep--;
				++r;
			}
			si(prog,r1,r2,r,tabs);
		}
		++r;
	}
}

void ExporterBase::definir(t_output &prog, t_arglist &arglist, string tipo, string tabs) {
	
}

void ExporterBase::dimension(t_output &prog, t_arglist &args, string tabs) {
	t_arglist_it it=args.begin();
	while (it!=args.end()) {
		string arr = *it;
		unsigned int i=0;
		while(i<arr.size())
			if (arr[i]==' ' || arr[i]=='\t')
				arr.erase(i,1);
			else 
				i++;
		arr[arr.size()-1]=',';
		unsigned int p=arr.size(), c=0, pars=0;
		for (i=0;i<arr.size();i++)
			if (arr[i]=='(' && pars==0) { 
				pars++;
				if (i<p) p=i;
			} else if (arr[i]==')') {
				pars--;
			} else if (arr[i]==',' && pars==1) {
				c++;
			} else if (arr[i]>='A' && arr[i]<='Z') {
				arr[i]=tolower(arr[i]);
			}
		int *dims=new int[c+1];
		string nom=arr.substr(0,p);
		dims[0]=c;
		int f=++p; c=1; pars=0;
		while (p<arr.size()) {
			if (arr[p]=='(')
				pars++;
			else if (arr[p]==')')
				pars++;
			else if (arr[p]==',' && pars==0) {
				tipo_var ch=vt_numerica_entera;
				dims[c++] = Evaluar(arr.substr(f,p-f),ch).GetAsInt();
				f=p+1;
			}
			p++;
		}
		memoria->AgregarArreglo(nom,dims);
		++it;
	}
}

// recibe los argumentos de una funcion (incluyendo los parentesis que los envuelven, y extra alguno (cual, base 1)
string ExporterBase::get_arg(string args, int cual) {
	int i=1,i0=1,parentesis=0,n=0; bool comillas=false;
	while (true) {
		if (args[i]=='\''||args[i]=='\"') comillas=!comillas;
		else if (!comillas) {
			if (parentesis==0 && (args[i]==','||args[i]==')')) {
				if (++n==cual) { return args.substr(i0,i-i0); }
				i0=i+1;
			} 
			else if (args[i]=='('||args[i]=='[') parentesis++;
			else if (args[i]==')'||args[i]==']') parentesis--;
		}
		i++;
	}
	return "";
}

bool ExporterBase::es_cadena_constante(const string &s) {
	if (s[0]!='\'' && s[0]!='\"') return false;
	int l=s.size()-1;
	if (s[l]!='\'' && s[l]!='\"') return false;
	for(int i=1;i<l;i++) { 
		if (s[i]=='\'' || s[i]=='\"') return false;
	}
	return true;
}

bool ExporterBase::es_numerica_constante(const string &s) {
	int l=s.size();
	for(int i=0;i<l;i++) { 
		if ( (s[i]>='a' && s[i]<='z') || 
			 (s[i]>='A' && s[i]<='Z') || 
			 s[i]=='\'' || s[i]=='\"' || s[i]=='_'
		   ) return false;
	}
	return true;
}

bool ExporterBase::es_numerica_entera_constante(const string &s) {
	return es_numerica_constante(s) && s.find('.')==string::npos;
}

void ExporterBase::init_header(t_output &out, string comment_pre, string comment_post) {
	stringstream version; 
	version<<VERSION<<"-"<<ARCHITECTURE;
	if (!for_test) {
		out.push_back(comment_pre+"Este codigo ha sido generado por el modulo psexport "+version.str()+" de PSeInt.");
		if (comment_post.size()) comment_pre="";
		out.push_back(comment_pre+"Es posible que el codigo generado no sea completamente correcto. Si encuentra");
		out.push_back(comment_pre+"errores por favor reportelos en el foro (http://pseint.sourceforge.net)."+comment_post);
		if (!for_test) out.push_back("");
	}
}

void ExporterBase::replace_var(t_output &out, string src, string dst) {
	t_output_it it=out.begin();
	while (it!=out.end()) {
		string s=*it;
		bool comillas=false;
		for(unsigned int i=0, l=0;i<=s.size();i++) { 
			if (i<s.size() &&(s[i]=='\''||s[i]=='\"')) comillas=!comillas;
			if (!comillas) {
				if (i==s.size()||(s[i]!='_'&&s[i]!='.'&&(s[i]<'0'||s[i]>'9')&&(s[i]<'a'||s[i]>'z')&&(s[i]<'A'||s[i]>'Z'))) {
					if (i!=l && s.substr(l,i-l)==src) {
						s.replace(l,i-l,dst);
						i+=dst.size()-src.size();
					} 
					l=i+1;
				}
			}
		}
		*it=s;
		++it;
	}
}

string ExporterBase::make_varname(string varname) {
	return ToLower(varname);
}

/**
* Esta funcion está para que varias instancias derivadas de ExporterBase puedan
* utilizar la el mismo mapa de memorias (que contiene una instancia de Memoria
* por proceso/subproceso). Esto es por ejemplo, para que el que exporta a c++
* primero pueda hacer una pasada del algoritmo com TiposExport para que las 
* variables ya tengan sus tipos definidos en cada ambito. SiNo, por ejemplo
* podría declararse una variable como string o sin_tipo en un paso por no 
* conocer su tipo y luego darse cuenta que un paso siguiente se utiliza como
* indice o dimensión de un arreglo y entonces debió ser int. Lo mismo se aplica
* a las lecturas en lenguajes donde no son iguales para todos los tipos (como
* c o vb).
**/
void ExporterBase::set_memoria(string key) {
	map<string,Memoria*>::iterator it=mapa_memorias.find(key);
	if (it==mapa_memorias.end()) {
		mapa_memorias[key]=memoria=new Memoria(NULL);
	} else {
		memoria=it->second;
	}
}

void ExporterBase::sep_args(const string &args, t_arglist &out) {
	int parentesis=0; bool comillas=false; int i0=0;
	for(int i=0,l=args.size();i<l;i++) {
		if (args[i]=='\''||args[i]=='\"') comillas=!comillas;
		else if (!comillas) {
			if (args[i]=='('||args[i]=='[') parentesis++;
			else if (args[i]==')'||args[i]==']') parentesis--;
			else if (parentesis==0 && args[i]==',') {
				insertar(out,args.substr(i0,i-i0));
				i0=i+1;
			}
			
		}
	}
	insertar(out,args.substr(i0));
}

string ExporterBase::get_aux_varname(string pref) {
	stringstream ss; ss<<pref<<(aux_varnames.size()+(output_base_zero_arrays?0:1));
	aux_varnames.push_back(ss.str());
	return ss.str();
}

void ExporterBase::release_aux_varname(string vname) {
	if (vname!=aux_varnames.back()) cerr<<"ERROR RELEASING AUX VARNAME\n";
	aux_varnames.pop_back();
}

void ExporterBase::crop_name_and_dims(string decl, string &name, string &dims, string c1, string c2, string c3) {
	name=decl; dims=decl;
	name.erase(name.find("("));
	dims.erase(0,dims.find("(")+1);
	dims.erase(dims.size()-1,1);
	t_arglist dimlist;
	sep_args(dims,dimlist);
	dims=c1; int n=0;
	t_arglist_it it2=dimlist.begin();
	while (it2!=dimlist.end()) {
		if ((n++)) dims+=c2;
		dims+=expresion(*it2);
		++it2;
	}
	dims+=c3;
}

bool ExporterBase::replace_all(string &str, string from, string to) {
	bool retval=false;
	size_t pos = str.find(from,0);
	while (pos!=string::npos) {
		retval=true;
		str.replace(pos,from.size(),to);
		pos=str.find(from,pos+to.size());
	}
	return retval;
}

void ExporterBase::comentar (t_output & prog, string text, string tabs) {
	
}

void ExporterBase::translate_all_procs (t_output & out, t_programa & prog, string tabs) {
	for (t_programa_it it1=prog.begin();it1!=prog.end();++it1) {
		t_proceso_it it2 = it1->begin();
		while (it2->nombre=="#comment"||it2->nombre=="#comment-inline") {
			comentar(out,it2->par1,tabs);
			if (++it2==it1->end()) return;
		}
		Funcion *f;
		if (it2->nombre=="PROCESO") { f=NULL; set_memoria(main_process_name); }
		else { int x; f=ParsearCabeceraDeSubProceso(it2->par1,false,x); set_memoria(f->id); }
		t_proceso proc(it2,it1->end());
		translate_single_proc(out,f,proc);	
		delete memoria; delete f;
	}
}

void ExporterBase::translate_all_procs (t_output & out_main, t_output &out_procs, t_programa & prog, string tabs) {
	for (t_programa_it it1=prog.begin();it1!=prog.end();++it1) {
		t_output out_com;
		t_proceso_it it2 = it1->begin();
		while (it2->nombre=="#comment"||it2->nombre=="#comment-inline") {
			comentar(out_com,it2->par1,tabs);
			if (++it2==it1->end()) {
				insertar_out(out_main,out_com);
				return;
			}
		}
		Funcion *f;
		if (it2->nombre=="PROCESO") { f=NULL; set_memoria(main_process_name); }
		else { int x; f=ParsearCabeceraDeSubProceso(it2->par1,false,x); set_memoria(f->id); }
		insertar_out(f?out_procs:out_main,out_com);
		t_proceso proc(it2,it1->end());
		translate_single_proc(f?out_procs:out_main,f,proc);
		delete memoria; delete f;
	}
}

void ExporterBase::translate_single_proc (t_output & out, Funcion *f, t_proceso & proc) {
	
}

