#include "global.h"
#include "common.h"
#include "new_evaluar.h"
#include "intercambio.h"
#include "new_memoria.h"
#include "utils.h"
#include "zcurlib.h"
#include "new_funciones.h"

// ***************** Control de Errores y Depuración **********************

// ------------------------------------------------------------
//    Informa un error en tiempo de ejecucion
// ------------------------------------------------------------
void ExeError(int num,string s) { 
	if (Inter.EvaluatingForDebug()) {
		Inter.SetError(string("<<")+s+">>");
	} else {
		if (raw_errors) {
			cout<<"=== Line "<<Inter.GetLineNumber()<<": ExeError "<<num<<endl;
			exit(0);
		}
		if (colored_output) setForeColor(COLOR_ERROR);
		cout<<"Lin "<<Inter.GetLineNumber()<<" (inst "<<Inter.GetInstNumber()<<"): ERROR "<<num<<": "<<s<<endl;
		for(int i=Inter.GetBacktraceLevel()-1;i>0;i--) {  
			FrameInfo fi=Inter.GetFrame(i);
			cout<<"...dentro del subproceso "<<fi.func_name<<", invocado desde la línea "<<fi.line<<"."<<endl;
		}
		if (ExeInfoOn) {
			ExeInfo<<"Lin "<<Inter.GetLineNumber()<<" (inst "<<Inter.GetInstNumber()<<"): ERROR "<<num<<": "<<s<<endl;
			for(int i=Inter.GetBacktraceLevel()-1;i>0;i--) {  
				FrameInfo fi=Inter.GetFrame(i);
				ExeInfo<<"Lin "<<fi.line<<" (inst "<<fi.instr<<"): ";
				ExeInfo<<"...dentro del subproceso "<<fi.func_name<<", invocado desde aquí."<<endl;
			}
			ExeInfo<<"*** Ejecucion Interrumpida. ***"<<endl;
		} 
		if (wait_key) {
			if (colored_output) setForeColor(COLOR_INFO);
			cout<<"*** Ejecucion Interrumpida. ***"<<endl;
		}
//		Inter.AddError(s,Inter.GetLineNumber());
		if (ExeInfoOn) ExeInfo.close();
		if (wait_key) getKey();
		exit(0);
	}
}

// ------------------------------------------------------------
//    Informa un error de syntaxis antes de la ejecucion
// ------------------------------------------------------------
void SynError(int num,string s, InstruccionLoc il) { 
	SynError(num,s,il.linea,il.inst);
}

void SynError(int num,string s, int line, int inst) { 
	if (line==-1) {
		line=Inter.GetLineNumber();
		inst=Inter.GetInstNumber();
	}
	if (raw_errors) {
		cout<<"=== Line "<<line<<": SynError "<<num<<endl;
		SynErrores++;
		return;
	}
	if (Inter.EvaluatingForDebug()) {
		Inter.SetError(string("<<")+s+">>");
	} else {
		if (colored_output) setForeColor(COLOR_ERROR);
		cout<<"Lin "<<line;
		if (inst>0) cout<<" (inst "<<inst<<")";
		cout<<": ERROR "<<num<<": "<<s<<endl;
		if (ExeInfoOn) {
			ExeInfo<<"Lin "<<line;
			if (inst>0) ExeInfo<<" (inst "<<inst<<")";
			ExeInfo<<": ERROR "<<num<<": "<<s<<endl;
		}
//		Inter.AddError(s,line);
		SynErrores++;
	}
}

// ------------------------------------------------------------
//    Analiza el tipo de error devuelo por Evaluar(expresion)
//  e informa segun corresponda.
// ------------------------------------------------------------
//void ExpError(char tipo, int level, int line) { 
//	string str;                            // a una expresion (level=0(CheckSintax) o 1(Ejecutar)
//	if (tipo&2) {
//		if (level==0)
//			SynError(130,"No coinciden los tipos.",line);
//		else
//			ExeError(131,"No coinciden los tipos.",line);
//	}
//	if (tipo=='0') {
//		if (level==0)
//			SynError(132,"Division por cero.",line);
//		else
//			ExeError(133,"Division por cero.",line);
//	}
//	if (tipo&4) {
//		if (level==1)
//			ExeError(134,"Numero de parametros o subindices incorrecto.",line);
//	}
//	if (tipo&1) {
//		if (level==1)
//			ExeError(135,"Subindices fuera de rango.",line);
//	}
// }

// ------------------------------------------------------------
//    Comprueba que sea un identificador correcto y unico
//    A diferencia del anterior, no tiene en cuenta las
//  funciones predefinidas.
// ------------------------------------------------------------
bool CheckVariable(string str, int errcode) { 
	size_t pi=str.find("(",0);
	if (pi!=string::npos && str[str.size()-1]==')')
		str.erase(pi,str.size()-pi); // si es arreglo corta los subindices
	bool ret=true;
	if (!EsLetra(str[0]))
		ret=false;
	for (int x=0;x<(int)str.size();x++) {
		if (!EsLetra(str[x]) && (str[x]<'0' || str[x]>'9') && str[x]!='_')
			ret=false;
	}
	// Comprobar que no sea palabra reservada
	if (EsFuncion(str)) 
		ret=false;
	else if (str=="LEER" || str=="ESCRIBIR" || str=="MIENTRAS" || str=="HACER" || str=="SEGUN" || str=="VERDADERO" || str=="FALSO" || str=="PARA")
		ret=false;
	else if (str=="REPETIR" || str=="SI" || str=="SINO" || str=="ENTONCES" || str=="DIMENSION" || str=="PROCESO" || str=="FINSI" ||  str=="" || str=="FINPARA")
		ret=false;
	else if (str=="FINSEGUN" || str=="FINPROCESO" || str=="FINMIENTRAS" || str=="HASTA" || str=="DEFINIR" || str=="COMO")
		ret=false;
	else if (enable_user_functions && (str=="FINSUBPROCESO" || str=="SUBPROCESO" ||str=="FINFUNCION" || str=="FUNCION" ||str=="FINFUNCIÓN" || str=="FUNCIÓN") )
		ret=false;
	if (!ret && errcode!=-1) SynError (errcode,string("Identificador no valido (")+str+")."); 
	return ret;
}

// ----------------------------------------------------------------------
//    Compara los comienzos de dos cadenas (case sensitive)
// ----------------------------------------------------------------------
bool LeftCompare(string a, string b) {  
	// compara los caracteres de la izquierda de a con b
	bool ret;
	if (a.size()<b.size()) ret=false; else {
		a.erase(b.size(),a.size()-b.size());
		if (a==b) ret=true; else ret=false; }
	return ret; }

// ----------------------------------------------------------------------
//    Compara los comienzos de dos cadenas (case insensitve)
// ----------------------------------------------------------------------
//bool LeftCompareNC(string a, string b){  
//	// compara los caracteres de la izquierda de a con b
//	for (int x=0;x<(int)a.size();x++)
//		if (a[x]>96 && a[x]<123) a[x]-=32;
//	for (int x=0;x<(int)b.size();x++)
//		if (b[x]>96 && b[x]<123) b[x]-=32;
//	bool ret;
//	if (a.size()<b.size()) ret=false; else {
//		a.erase(b.size(),a.size()-b.size());
//		if (a==b) ret=true; else ret=false; }
//	return ret; 
//}

// ----------------------------------------------------------------------
//    Compara las terminaciones de dos cadenas (case sensitve)
// ----------------------------------------------------------------------
bool RightCompare(string a, string b) { 
	// compara los caracteres de la derecha de a con b
	bool ret;
	if (a.size()<b.size()) ret=false; else {
		a.erase(0,a.size()-b.size());
		if (a==b) ret=true; else ret=false; }
	return ret; 
}

// ----------------------------------------------------------------------
//    Compara las terminaciones de dos cadenas (case insensitve)
// ----------------------------------------------------------------------
//bool RightCompareNC(string a, string b) { 
//	// compara los caracteres de la derecha de a con b
//	for (int x=0;x<(int)a.size();x++)
//		if (a[x]>96 && a[x]<123) a[x]-=32;
//	for (int x=0;x<(int)b.size();x++)
//		if (b[x]>96 && b[x]<123) b[x]-=32;
//	bool ret;
//	if (a.size()<b.size()) ret=false; else {
//		a.erase(0,a.size()-b.size());
//		if (a==b) ret=true; else ret=false; }
//	return ret; 
//}

// ----------------------------------------------------------------------
//    Compara parte de una cadena con otra (case insensitve)
// ----------------------------------------------------------------------
bool MidCompareNC(string a, string b, int from) { 
	unsigned int to=from+a.size();
	// controla los tamaños y corta la parte de interes
	if (b.size()<to) return false;
	b.erase(to,b.size());
	b.erase(0,from);
	for (int x=0;x<(int)a.size();x++)
		if (a[x]>96 && a[x]<123) a[x]-=32;
	for (int x=0;x<(int)b.size();x++)
		if (b[x]>96 && b[x]<123) b[x]-=32;
	bool ret;
	// compara los caracteres de la derecha de a con b
	if (a.size()<b.size()) ret=false; else {
		a.erase(0,a.size()-b.size());
		if (a==b) ret=true; else ret=false; }
	return ret; 
}

// ----------------------------------------------------------------------
//    Pasa una cadena a mayusculas
// ----------------------------------------------------------------------
string toUpper(string a) { 
	int l=a.size();
	for (int x=0;x<l;x++)
		a[x]=toupper(a[x]);
	return a; 
}

// ----------------------------------------------------------------------
//    Pasa una cadena a mayusculas
// ----------------------------------------------------------------------
string toLower(string a) { 
	int l=a.size();
	for (int x=0;x<l;x++)
		a[x]=tolower(a[x]);
	return a; 
}

// ----------------------------------------------------------------------
//    Averigua el tipo de variable para un dato
// ----------------------------------------------------------------------
tipo_var GuestTipo(string str) { 
	tipo_var ret=vt_desconocido;
	string strb;
	strb=str;
	if (str.size()==0) ret=vt_desconocido; else {
		if (strb=="VERDADERO" || strb=="FALSO") {
			ret=vt_caracter_o_logica;
		} else {
			ret=vt_caracter_o_numerica;
			for (int x=0;x<(int)str.size();x++)
				if ((str[x]<46 || str[x]>57 || str[x]==47) && !(str[x]=='-' && x==0)) ret=vt_caracter;
		}
	}
	return ret;
}

// ----------------------------------------------------------------------
//    Reemplaza una cadena por otra si es que se encuentra
// ----------------------------------------------------------------------
bool ReplaceIfFound(string &str, string str1,string str2,bool saltear_literales) { 
	size_t x=str.find(str1,0);
	size_t ox=0;
	bool ret=false;
	while (x!=string::npos) {
		if (saltear_literales) {
			size_t xc=str.find("\'",ox);
			if (xc!=string::npos && xc<x) {
				xc=str.find("\'",xc+1);
				if (xc==string::npos) break;
				x=str.find(str1,ox=(xc+1));
				continue;
			}
		}
		ret=true;
		str.erase(x,str1.size());
		str.insert(x,str2);
		x=str.find(str1,ox=(x+str2.size()));
	}
	return ret; 
}


// **************************************************************************

// funciones auxiliares para psexport
inline int max(int a,int b){ return (a>b)?a:b; }
inline int min(int a,int b){ return (a<b)?a:b; }
string CutString(string s, int a, int b){
	string r=s;
	r.erase(0,a);
	if (b!=0) r.erase(r.size()-b,b);
	return r;
}

// determina si una letra puede ser parte de una palabra clave o identificador
bool parteDePalabra(char c) {
	return (EsLetra(c) || c=='_' || (c>='0' && c<='9'));
}

void fixwincharset(string &s, bool reverse) {
	if (!fix_win_charset) return;
	if (reverse) {
		for(unsigned int i=0;i<s.size();i++) { 
			char &c=s[i];
			if (c==-96) c='á';
			else if (c==-126) c='é';
			else if (c==-95) c='í';
			else if (c==-94) c='ó';
			else if (c==-93) c='ú';
			else if (c==-75) c='Á';
			else if (c==-112) c='É';
			else if (c==-42) c='Í';
			else if (c==-32) c='Ó';
			else if (c==-23) c='Ú';
			else if (c==-92) c='ñ';
			else if (c==-91) c='Ñ';
			else if (c==-83) c='¡';
			else if (c==-88) c='¿';
			else if (c==-127) c='ü';
			else if (c==-102) c='Ü';
		}
	} else {
		for(unsigned int i=0;i<s.size();i++) { 
			char &c=s[i];
			if (c=='á') c=-96;
			else if (c=='é') c=-126;
			else if (c=='í') c=-95;
			else if (c=='ó') c=-94;
			else if (c=='ú') c=-93;
			else if (c=='Á') c=-75;
			else if (c=='É') c=-112;
			else if (c=='Í') c=-42;
			else if (c=='Ó') c=-32;
			else if (c=='Ú') c=-23;
			else if (c=='ñ') c=-92;
			else if (c=='Ñ') c=-91;
			else if (c=='¡') c=-83;
			else if (c=='¿') c=-88;
			else if (c=='ü') c=-127;
			else if (c=='Ü') c=-102;
		}
	}
}

string NextToken(string &cadena, int &p) {
	int l=cadena.size();
	while (p<l && (cadena[p]==' ' || cadena[p]=='\t')) p++;
	if (p==l) return "";
	int p1=p;
	if ((cadena[p]>='A'&&cadena[p]<='Z')||cadena[p]=='_') {
		while (p<l && parteDePalabra(cadena[p]) ) p++;
	} else if ((cadena[p]>='0'&&cadena[p]<='9')||cadena[p]=='.') {
		while (p<l && ((cadena[p]>='0'&&cadena[p]<='9')||cadena[p]=='.')) p++;
	} else if (cadena[p]=='\"'||cadena[p]=='\'') {
		while (p<l && cadena[p]!='\"'&&cadena[p]!='\'' ) p++;
	} else if (cadena[p]==')') {
		p++; return ")";
	} else if (cadena[p]=='(') {
		p++; return "(";
	} else { // operador? todavia no se usa este caso
		if (p<l) p++;
		while (p<l && !parteDePalabra(cadena[p]) && cadena[p]!=' ' && cadena[p]!='\t' && cadena[p]!=')' && cadena[p]!='(' && cadena[p]!=',' && cadena[p]!=';' && cadena[p]!='\"' && cadena[p]!='\'') p++;
	}
	return cadena.substr(p1,p-p1);
}

