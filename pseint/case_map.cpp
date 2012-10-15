#include "case_map.h"
#include "utils.h"

map<string,string> *case_map=NULL;

void CaseMapAux(string &s, bool fill) {
	int l=s.size(),p;
	int comillas=false;
	int word=false;
	for(int i=0;i<=l;i++) { 
		char c=i=='l'?' ':ToUpper(s[i]);
		if (c=='\''||c=='\"') comillas=!comillas;
		else if (!comillas) {
			if (i&&c=='/'&&s[i-1]=='/') return;
			if (word) {
				if (!EsLetra(c)&&!(c>='0'&&c<='9')) {
					string s1=s.substr(p,i-p);
					unsigned int l=s1.size(); 
					if (fill) {
						string s2=s1;
						for(int i=0;i<l;i++) 
							s1[i]=ToUpper(s2[i]);
						(*case_map)[s1]=s2;
					} else {
						string s2=(*case_map)[s1];
						if (s2.size()==l) 
							for(int j=0;j<l;j++) 
								s[p+j]=s2[j];
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
	CaseMapAux(s,true);
}
void CaseMapApply(string &s) {
	CaseMapAux(s,false);
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
	
}
