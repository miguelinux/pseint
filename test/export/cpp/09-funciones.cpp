// Este codigo ha sido generado por el modulo psexport 20131129-l64 de PSeInt
// dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental
// puede que el codigo generado no sea completamente correcto. Si encuentra
// errores por favor reportelos en el foro (http://pseint.sourceforge.net).

#include<iostream>
#include<cmath>
#include<cstdlib>
#include<cctype>
#include<stringstream>
using namespace std;

// No hay en el C++ estandar una funcion equivalente a "convertiratexto".
#include<stringstream>
string convertiratexto(float f) {
	stringstream ss;
	ss<<f;
	return ss.str();
}

// No hay en el C++ estandar una funcion equivalente a "mayusculas".
string mayusculas(string s) {
	for(unsigned int i=0;i<s.size();i++)
		s[i]=toupper(s[i]);
	return s;
}

// Para leer variables de texto se utiliza el operador << del objeto cin, que
// lee solo una palabra. Para leer una linea completa (es decir, incluyendo los
// espacios en blanco) se debe utilzar getline (ej, reemplazar cin>>x por
// getline(cin,x), pero obliga a agregar un cin.ignore() si antes del getline
// se leyó otra variable con >>.

int main() {
	float a;
	float i;
	string s;
	float x;
	x=1.5;
	cout<<sqrtf(a)<<endl;
	cout<<abs(i)<<endl;
	cout<<log(x)<<endl;
	cout<<exp(x)<<endl;
	cout<<sin(x)<<endl;
	cout<<cos(x)<<endl;
	cout<<tan(x)<<endl;
	cout<<asin(x)<<endl;
	cout<<acos(x)<<endl;
	cout<<atan(x)<<endl;
	cout<<floor((x).5)<<endl;
	cout<<floor(x)<<endl;
	cout<<(rand()%(x))<<endl;
	s="Hola";
	cout<<s.size()<<endl;
	cout<<mayusculas(s)<<endl;
	cout<<mayusculas(s)<<endl;
	cout<<s.substr(1,2-1+1)<<endl;
	cout<<(s+" mundo")<<endl;
	cout<<atof("15.5")<<endl;
	cout<<convertiratexto(15.5)<<endl;
	return 0;
}

