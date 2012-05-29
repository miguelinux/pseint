#include "global.h"
#include "common.h"
#include "new_evaluar.h"
#include "intercambio.h"
#include "new_memoria.h"
#include "utils.h"
#include "zcurlib.h"

// ***************** Conversiones Numeros/Cadenas **********************

// -------------------------------------------------------------------
//    Convierte un entero a una cadena
// -------------------------------------------------------------------
string IntToStr(int f) {
	string s,stmp="0";
	int tmp;
	int a=1,b,c;
	if (f<0) {f=f*(-1);s="-";}
	while (a<=f) a=a*10;
	a=a/10;
	while (a>0) {
		b=a;c=1;
		while (b<=f) {b+=a;c++;}
		stmp[0]=(c-1)+48;
		s=s+stmp;
		f=f-(b-a);
		a=a/10; 
	}
	tmp=s.find(".",0);
	while (s[s.size()-1]==' ' && (int)s.size()>tmp && tmp>=0)
		s.erase(s.size()-1,1);
	if (s=="") s="0";
	return s;
}

// -------------------------------------------------------------------
//    Convierte una cadena a un double
// -------------------------------------------------------------------
//double StrToDbl(string s) {
//	int Neg=0; // Bandera de Numero negativo
//	int i=0;
//	if (s[0]=='-') {i++; Neg=1;}
//	else if (s[0]=='+') i++;
//	double f=0,b,punto=0;
//	while ((int)s[i]!=0) {
//		if (punto==0)
//			if ((int)s[i]==46)
//				punto=.1;
//			else
//				f=(f*10)+((int)s[i]-48);
//		else {
//			b=(int)s[i]-48;
//			f+=b*punto;
//			punto=punto/10; }
//		i++;};
//	if (Neg==1) f=-f;
//	return f;
// }

// -------------------------------------------------------------------
//    Convierte un double a una cadena - Alta Precision
// -------------------------------------------------------------------
//string DblToStrM(double f) {
//	stringstream s;
//	s<<setprecision(50)<<f;
//	return s.str();
// }
//// -------------------------------------------------------------------
////    Convierte un double a una cadena
//// -------------------------------------------------------------------
//string DblToStr(double f) {
//	stringstream s;
//	s<<fixed<<setprecision(10)<<f;
//	string str = s.str();
//	size_t p = str.find('.',0);
//	if (p!=string::npos) {
//		int s = str.size();
//		while (str[s-1]=='0')
//			s--;
//		if (str[s-1]=='.')
//			s--;
//		str.erase(s,str.size()-s);
//	}
//	return str;
//	
//	/*
//	string s,stmp="0";
//	int pos;
//	double a=1,c,ff=0, aa,b=0, la;
//	if (f<0) {s="-";f=-f;} else s="+";
//	
//	pos=1;
//	while (f>=a)
//	{a=a*10;s=s+"0";}
//	//   s.erase(s.size()-1,1);
//	aa=a;
//	a=0;
//	while (ff<f && aa>.000000001){
//	if (aa>1)
//	aa=aa/10;
//	else {
//	stmp=IntToStr((int)((f-a)*100000000));
//	s=s+".";
//	s=s+stmp;
//	while (s[s.size()-1]=='0') 
//	s.erase(s.size()-1,1);
//	break;
//	}
//	//      TRUNC(aa,5);
//	b=aa; c=0;
//	la=a-1;;
//	while (a<=f && la!=a) {la=a;a=a+b;c++;}
//	//      if (!(b<1))
//	a=a-b; c--;
//	if (b<1 && b>.02) {s=s+"."; pos++;}
//	if (pos>=(int)s.size())
//	s=s+"0";
//	if (c==0) s[pos]='0';
//	if (c==1) s[pos]='1';
//	if (c==2) s[pos]='2';
//	if (c==3) s[pos]='3';
//	if (c==4) s[pos]='4';
//	if (c==5) s[pos]='5';
//	if (c==6) s[pos]='6';
//	if (c==7) s[pos]='7';
//	if (c==8) s[pos]='8';
//	if (c==9) s[pos]='9';
//	ff=a; pos++;
//	}
//	if (s[0]=='+') s.erase(0,1);
//	
//	int tmp=s.find(".",0);
//	while (s[s.size()-1]=='0' && (int)s.size()>tmp && tmp>=0)
//	s.erase(s.size()-1,1);
//	if (s[s.size()-1]=='.')
//	s.erase(s.size()-1,1);
//	if (s=="") s="0";
//	return s;
//	*/
// }


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
		if (ExeInfoOn) {
			ExeInfo<<"Lin "<<Inter.GetLineNumber()<<" (inst "<<Inter.GetInstNumber()<<"): ERROR "<<num<<": "<<s<<endl;
			ExeInfo<<"*** Ejecucion Interrumpida. ***\n";
		} 
		if (wait_key) {
			if (colored_output) setForeColor(COLOR_INFO);
			cout<<"*** Ejecucion Interrumpida. ***\n";
		}
		Inter.AddError(s,Inter.GetLineNumber());
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
		Inter.AddError(s,line);
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
bool CheckVariable(string str) { 
	if (str.find("(",0)>0 && str.find("(",0)<str.size() && str[str.size()-1]==')')
		str.erase(str.find("(",0),str.size()-str.find("(",0)); // si es arreglo corta los subindices
	bool ret=true;
	if (!EsLetra(str[0]))
		ret=false;
	for (int x=0;x<(int)str.size();x++) {
		if (!EsLetra(str[x]) && (str[x]<'0' || str[x]>'9') && str[x]!='_')
			ret=false;
	}
	// Comprobar que no sea palabra reservada
	if (str=="LEER" || str=="ESCRIBIR" || str=="MIENTRAS" || str=="HACER" || str=="SEGUN" || str=="VERDADERO" || str=="FALSO" || str=="PARA")
		ret=false;
	if (str=="REPETIR" || str=="SI" || str=="SINO" || str=="ENTONCES" || str=="RC" || str=="RAIZ" || str=="EXP" || str=="ATAN" || str=="DIMENSION" || str=="PROCESO")
		ret=false;
	if (str=="ABS" || str=="SEN" || str=="TRUNC" || str=="FINSI" ||  str=="LN" || str=="COS" || str=="REDON" || str=="AZAR" || str=="" || str=="FINPARA")
		ret=false;
	if (str=="FINSEGUN" || str=="FINPROCESO" || str=="FINMIENTRAS" || str=="HASTA" || str=="DEFINIR" || str=="COMO" || str=="ACOS" || str=="ASEN" || str=="TAN")
		ret=false;
	return ret;
}


// ------------------------------------------------------------
//    Comprueba que sea un identificador correcto y unico
// ------------------------------------------------------------
//bool CheckVariableForW(string str) {
//	if (str.find("(",0)>0 && str.find("(",0)<str.size() && str[str.size()-1]==')')
//		str.erase(str.find("(",0),str.size()-str.find("(",0)); // si es arreglo corta los subindices
//	bool ret=true;
//	if (!EsLetra(str[0])
//		ret=false;
//	for (int x=0;x<(int)str.size();x++) {
//		if (!EsLetra(str[x]) && (str[x]<'0' || str[x]>'9') && str[x]!='_')
//			ret=false;
//	}
//	// Comprobar que no sea palabra reservada
//	if (str=="LEER" || str=="ESCRIBIR" || str=="MIENTRAS" || str=="HACER" || str=="SEGUN" || str=="VERDADERO" || str=="FALSO" || str=="PARA")
//		ret=false;
//	if (str=="REPETIR" || str=="SI" || str=="SINO" || str=="ENTONCES" ||   str=="DIMENSION" || str=="PROCESO")
//		ret=false;
//	if (str=="" || str=="FINSI" ||  str=="FINPARA" || str=="FINSEGUN" || str=="FINPROCESO" || str=="FINMIENTRAS" ||  str=="HASTA" || str=="DEFINIR" || str=="COMO")
//		ret=false;
//	return ret;
//}


// *********************** Funciones Auxiliares **************************

// ----------------------------------------------------------------------
//    Escribe un variable en Memoria realizando algunas verificaciones
// ----------------------------------------------------------------------
//void EscribirVar(string nombre,tipo_var tipo,string contenido) { // Escribe el contenido de una variable en la clase Memoria
//	string aux1,aux3,aux4;
//	int tmp3,tmp4;
//	char tipo='n';
//	// Borrar ceros al final de los decimales
//	if (tip=='n' && contenido.find(".",0)>=0 && contenido.find(".",0)<contenido.size())
//		while (contenido[contenido.size()-1]=='0')
//			contenido.erase(contenido.size()-1,1);
//	if (nombre.find("(",0)>=0 && nombre.find("(",0)<nombre.size()) { // si es un arreglo
//		// Comprobar indices
//		aux3=nombre; aux3.erase(0,nombre.find("(",0)+1);
//		aux3.erase(aux3.size()-1,1);
//		nombre.erase(nombre.find("(",0),nombre.size()-nombre.find("(",0));
//		if (contenido[0]=='\'') {
//			tipo='C';
//			contenido.erase(0,1);
//			contenido.erase(contenido.size()-1,1);
//		} else
//			tipo=TipoVar(nombre);
//		if (tip=='n' && (tipo=='l' || tipo=='c')) // cambio 20080421
//			ExeError(136,"No coinciden los tipos.",Inter.GetLineNumber());
//		if (tip=='l' && (tipo=='n' || tipo=='c')) // cambio 20080421
//			ExeError(137,"No coinciden los tipos.",Inter.GetLineNumber());
//		if (tip=='c' && (tipo=='n' || tipo=='l')) // cambio 20080421
//			ExeError(138,"No coinciden los tipos.",Inter.GetLineNumber());
//		int *dim, dims=0, *dimb; // Arreglo para los indices
//		tmp3=0; tmp4=0;
//		while (tmp4<(int)aux3.size()) {   // Contar Dimensiones
//			while (!(tmp3==0 && aux3[tmp4]==',') && tmp4<(int)aux3.size()) {
//				tmp4++;
//				if (aux3[tmp4]=='(') tmp3++;
//				if (aux3[tmp4]==')') tmp3--;
//			}
//			dims++;
//			tmp4++;
//		}
//		dim=new int[dims+1]; // Generar arreglo
//		dim[0]=dims;
//		tmp3=0; tmp4=0;dims=0;
//		while (tmp4<(int)aux3.size()) {   // Separar Indices
//			while (!(tmp3==0 && aux3[tmp4]==',') && tmp4<(int)aux3.size()) {
//				tmp4++;
//				if (aux3[tmp4]=='(') tmp3++;
//				if (aux3[tmp4]==')') tmp3--;
//			}
//			dims++;
//			aux4=aux3; // Cortar Indice
//			aux4.erase(tmp4,aux4.size()-tmp4);
//			aux3.erase(0,aux4.size()+1);
//			tmp4-=aux4.size()+1;
//			aux4=Evaluar(aux4,tipo);
//			if (tipo!='d' && tipo!='n') // Comprobar tipo
//				if (tipo<'c')
//					ExpError(tipo,1,Inter.GetLineNumber());
//				else
//					ExeError(139,"No coinciden los tipos.",Inter.GetLineNumber());
//			else
//				dim[dims]=(int)StrToDbl(aux4);
//			tmp4++;
//		}
//		// Comprobar la dimension y que los indices esten dentro del rango de validez
//		dimb=Mem->a_getdim(nombre);
//		if (dimb[0]!=dim[0]) {
//			if (dimb[0])
//				ExeError(140,string("Numero de subindices incorrecto (")+nombre+").",Inter.GetLineNumber());
//			else
//				ExeError(149,string("No se dimensiono el arreglo (")+nombre+").",Inter.GetLineNumber());
//		} else
//			for (int x=1;x<=dim[0];x++)
//				if (!(dim[x]>0 && dim[x]<=dimb[x]))
//			ExeError(141,string("Subindice fuera de rango (")+nombre+").",Inter.GetLineNumber());
//		Mem->a_escribir(nombre,tip,dim,contenido); // Escribir el Dato en Memoria
//		delete dim;
//	} else { // si no era un arreglo
//		if (contenido[0]=='\'') {
//			tipo='c';
//			contenido.erase(0,1);
//			contenido.erase(contenido.size()-1,1);
//		} else
//			tipo=TipoVar(nombre);
//		if (Mem->a_existe(nombre))
//			ExeError(142,string("Faltan Subindices (")+nombre+").",Inter.GetLineNumber());
//		if (tip=='n' && (tipo=='l' || tipo=='c'))
//			ExeError(143,"No coinciden los tipos.",Inter.GetLineNumber());
//		if (tip=='c' && (tipo=='l' || tipo=='n'))
//			ExeError(144,"No coinciden los tipos.",Inter.GetLineNumber());
//		if (tip=='l' && (tipo=='n' || tipo=='c'))
//			ExeError(145,"No coinciden los tipos.",Inter.GetLineNumber());
//		Mem->escribir(nombre,tip,contenido); // Escribir el Dato en Memoria
//	}
// }

// ----------------------------------------------------------------------
//    Lee una variable de Memoria o evalua una funcion predefinida
// ----------------------------------------------------------------------
//string LeerVar(string str) { // Lee el contenido de una variable de la clase Memoria
//	if (str.find("(",0)<0 || str.find("(",0)>str.size()){
//		if (str!="VERDADERO" && str!="FALSO") {
//			if (Mem->a_existe(str))
//				ExeError(146,string("Faltan subindices (")+str+").",Inter.GetLineNumber());
//			string nombre=str;
//			str=Mem->leer(nombre);
//			if (str=="\r")
//				ExeError(150,string("Variable no inicializada (")+nombre+").",Inter.GetLineNumber());
//		}
//	} else {
//		string aux1,aux2; int tmp1,tmp2; // variables auxiliares
//		tmp1=str.find("(",0);  // Cortar nombre del arreglo
//		aux1=str; aux1.erase(tmp1,aux1.size()-tmp1);
//		aux2=str; // cortar indices
//		aux2.erase(0,aux1.size()+1);  aux2.erase(aux2.size()-1,1);
//		tmp1=1; tmp2=0; // Contar indices
//		while (aux2.find(",",tmp2)>=0 && aux2.find(",",tmp2)<str.size())
//		{ tmp1++; tmp2=aux2.find(",",tmp2)+1; }
//		// Generar arreglo
//		if (aux1!="RC" && aux1!="ABS" && aux1!="LN" && aux1!="EXP" && aux1!="SEN" && aux1!="COS" && aux1!="ASEN" && aux1!="ACOS" && aux1!="AZAR" && aux1!="ATAN" && aux1!="TAN" && aux1!="TRUNC" && aux1!="REDON") {
//			int *dim; // arreglo para las dimensiones
//			dim=new int[tmp1+1]; dim[0]=tmp1;
//			tmp2=0;
//			tmp1=1;
//			aux2=aux2+",";
//			while (aux2.find(",",0)>=0 && aux2.find(",",0)<=aux2.size()) {
//				str=aux2;
//				tmp2=aux2.find(",",0);
//				str.erase(tmp2,str.size()-tmp2);
//				aux2.erase(0,tmp2+1);
//				dim[tmp1]=(int)StrToDbl(str);
//				tmp1++;
//			}
//			str=Mem->a_leer(aux1,dim);
//			if (str=="\r")
//				ExeError(151,string("Posicion no inicializada (")+aux1+").",Inter.GetLineNumber());
//			delete dim;
//		} else {
//			if (aux1=="RC") {
//				if (StrToDbl(aux2)<0) {
//					if (Inter.Running())
//						ExeError(147,"Raiz de negativo.",Inter.GetLineNumber());
//					;;
//				} else
//					str=DblToStr((double)(sqrt(double(StrToDbl(aux2)))));
//			}
//			if (aux1=="ABS") {
//				str=aux2;
//				if (str[0]=='-') str.erase(0,1);
//			}
//			if (aux1=="LN") {
//				if (StrToDbl(aux2)<=0) {
//					if (Inter.Running())
//						ExeError(148,"Logaritmo de 0 o negativo.",Inter.GetLineNumber());
//					;;
//				} else
//					str=DblToStr((double)(log((double)(StrToDbl(aux2)))));
//			}
//			if (aux1=="EXP") {
//				str=DblToStr((double)(exp((double)(StrToDbl(aux2)))));
//			}
//			if (aux1=="SEN") {
//				str=DblToStr((double)(sin((double)(StrToDbl(aux2)))));
//			}
//			if (aux1=="ASEN") {
//				str=DblToStr((double)(asin((double)(StrToDbl(aux2)))));
//			}
//			if (aux1=="ACOS") {
//				str=DblToStr((double)(acos((double)(StrToDbl(aux2)))));
//			}
//			if (aux1=="COS") {
//				str=DblToStr((double)(cos((double)(StrToDbl(aux2)))));
//			}
//			if (aux1=="TAN") {
//				str=DblToStr((double)(tan((double)(StrToDbl(aux2)))));
//			}
//			if (aux1=="ATAN") {
//				str=DblToStr((double)(atan((double)(StrToDbl(aux2)))));
//			}
//			if (aux1=="AZAR") {
//				if (int(StrToDbl(aux2))<=0) {
//					if (Inter.Running())
//						ExeError(152,"Azar de 0 o negativo.",Inter.GetLineNumber());
//				} else
//					str=IntToStr(rand() % (int)StrToDbl(aux2));
//			}
//			if (aux1=="TRUNC") {
//				str=aux2;
//				if (str.find(".",0)>=0 && str.find(".",0)<str.size())
//					str.erase(str.find(".",0),str.size()-str.find(".",0));
//				if (str=="") str="0";
//			}
//			if (aux1=="REDON") {
//				str=aux2;
//				if (str.find(".",0)>=0 && str.find(".",0)<str.size())
//					str.erase(str.find(".",0),str.size()-str.find(".",0));
//				if (str=="") str="0";
//				double a,b;
//				a=(StrToDbl(aux2));
//				b=(StrToDbl(str));
//				if (a<0) {a=-a;b=-b;}
//				if ((a-b)>.5) b=b+1;
//				if (aux2[0]=='-') b=-b;
//				str=DblToStr(b);
//			}
//		}
//	}
//	return str;
// }

// ----------------------------------------------------------------------
//    Devuelve el tipo de una variable o una funcion predefinida
// ----------------------------------------------------------------------
//tipo_var TipoVar(string str) { // Lee el tipo de una variable de la clase Memoria
//	if (str.find("(",0)>0 && str.find("(",0)<str.size())
//		str.erase(str.find("(",0),str.size()-str.find("(",0));
//	char tipo;
//	if (str!="RC" && str!="ABS" && str!="LN" && str!="EXP" && str!="ASEN" && str!="ACOS"  && str!="SEN" && str!="COS" && str!="AZAR" && str!="ATAN" && str!="TAN" && str!="TRUNC" && str!="REDON") {
//		if (str!="VERDADERO" && str!="FALSO")
////			tipo=Mem->tipo(str);
//			tipo=memoria->LeerTipo(str);
//		else
//			tipo=vt_logica;
//	} else
//		tipo=vt_numerica;
////	if (tipo>64 && tipo<97) tipo+=32;
//	return tipo;
// }

// ----------------------------------------------------------------------
//    Escribe una expresion en el ambiente
// ----------------------------------------------------------------------
void Escribir(string aux1) {
	if (colored_output) setForeColor(COLOR_OUTPUT);
	tipo_var x;
	aux1=Evaluar(aux1,x);
//	if (x==vt_error)
//		ExpError(x,1,Inter.GetLineNumber()); 
//	else {
	if (x!=vt_error) {
		if (x==vt_numerica)
			aux1=DblToStr(StrToDbl(aux1),10);
		else {
			fixwincharset(aux1);
//			if (aux1[0]=='\'') { // esto no debería pasar nunca
//				aux1.erase(0,1);
//				aux1.erase(aux1.size()-1,1);
//			}
		}
		cout<<aux1; // Si es variable, muestra el contenido
	}
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

bool EsLetra(const char &c) {
	return ( (c>='A' && c<='Z') || (lazy_syntax && (c=='Á'||c=='É'||c=='Í'||c=='Ó'||c=='Ú'||c=='Ñ'||c=='Ü') ) );
}
