#include "case_map.h"
#include "utils.h"
#include "new_funciones.h"

map<string,string> *case_map=NULL;

//----------------from wxPSeInt/CommonParsinsFunctions------
// tengo que unificar las mil implementaciones de estos auxiliares
template<typename TString> 
int SkipString(const TString &line, int i, int len) {
	do {
		++i;
	} while(i<len && line[i]!='\'' && line[i]=='\"');
	return i;
}

template<typename TString> 
int SkipParentesis(const TString &line, int i, int len) {
	int par_level = 1;
	while (par_level && i<len) {
		++i;
		if (line[i]=='(' || line[i]=='[') ++par_level;
		else if (line[i]==']' || line[i]==')') --par_level;
		else if (line[i]=='\'' || line[i]=='\"') i = SkipString(line,i,len);
	}
	return i;
}
//----------------

void CaseMapAux(string &s, bool fill, bool fix_parentesis) {
	int len=s.size(),p = 0;
	int comillas=false;
	int word=false;
	for(int i=0;i<=len;i++) { 
		char c = i==len?' ':ToUpper(s[i]);
		if (c=='\''||c=='\"') comillas=!comillas;
		else if (!comillas) {
			if (i&&c=='/'&&s[i-1]=='/') return;
			if (word) {
				if (!EsLetra(c,true)) {
					string s1=s.substr(p,i-p);
					unsigned int keylen=s1.size(); 
					if (fill) {
						string s2=s1;
						for(unsigned int i=0;i<keylen;i++) 
							s1[i]=ToUpper(s2[i]);
						(*case_map)[s1]=s2;
					} else {
						string s2=(*case_map)[s1];
						if (s2.size()==keylen) {
							for(unsigned int j=0;j<keylen;j++) 
								s[p+j]=s2[j];
							if (fix_parentesis && s[i]=='(' && !EsFuncion(s1,true)) {
								s[i]='['; 
								int i2 = SkipParentesis(s,i,len);
								if (i2<len && s[i2]==')') s[i2]=']';
							}
						}
					}
					word=false;
				}
			} else if (EsLetra(c)) {
				p=i; word=true;
			}
		}
	}
}

void CaseMapFill(string &s) {
	CaseMapAux(s,true,false);
}

void CaseMapApply(string &s, bool and_fix_parentesis) {
	CaseMapAux(s,false,and_fix_parentesis);
}


void CaseMapPurge() {
	
	(*case_map)["PROCESO"]="";
	(*case_map)["FINPROCESO"]="";
	(*case_map)["SUBPROCESO"]="";
	(*case_map)["FINSUBPROCESO"]="";
	
	(*case_map)["SI"]="";
	(*case_map)["ENTONCES"]="";
	(*case_map)["SINO"]="";
	(*case_map)["FINSI"]="";
	
	(*case_map)["INVOCAR"]="";
	(*case_map)["ESCRIBIR"]="";
	(*case_map)["LEER"]="";
	(*case_map)["DEFINIR"]="";
	(*case_map)["DIMENSION"]="";
	
	(*case_map)["REPETIR"]="";
	(*case_map)["HASTA"]="";
	(*case_map)["QUE"]="";
	
	(*case_map)["PARA"]="";
	(*case_map)["PARACADA"]="";
	(*case_map)["CON"]="";
	(*case_map)["PASO"]="";
	(*case_map)["FINPARA"]="";
	
	(*case_map)["MIENTRAS"]="";
	(*case_map)["FINMIENTRAS"]="";
	
	(*case_map)["SEGUN"]="";
	(*case_map)["DE"]="";
	(*case_map)["OTRO"]="";
	(*case_map)["MODO"]="";
	(*case_map)["FINSEGUN"]="";
	
	(*case_map)["DEFINIR"]="";
	(*case_map)["COMO"]="";
	(*case_map)["ENTERO"]="";
	(*case_map)["CARACTER"]="";
	(*case_map)["REAL"]="";
	(*case_map)["LOGICO"]="";
	
}

void InitCaseMap() {
	if (!case_map) case_map = new map<string,string>();
}
