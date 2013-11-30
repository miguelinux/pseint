// Este codigo ha sido generado por el modulo psexport 20131129-l64 de PSeInt
// dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental
// puede que el codigo generado no sea completamente correcto. Si encuentra
// errores por favor reportelos en el foro (http://pseint.sourceforge.net).

#include<iostream>
using namespace std;

// Para las variables que no se pudo determinar el tipo se utiliza la constante
// SIN_TIPO. El usuario debe reemplazar sus ocurrencias por el tipo adecuado
// (usualmente int,float,string o bool).
#define SIN_TIPO string

// Para leer variables de texto se utiliza el operador << del objeto cin, que
// lee solo una palabra. Para leer una linea completa (es decir, incluyendo los
// espacios en blanco) se debe utilzar getline (ej, reemplazar cin>>x por
// getline(cin,x), pero obliga a agregar un cin.ignore() si antes del getline
// se leyó otra variable con >>.

int main() {
	SIN_TIPO a;
	SIN_TIPO b;
	SIN_TIPO c;
	SIN_TIPO d;
	int a;
	float b;
	string c;
	bool d;
	cin>>a;
	cin>>b;
	cin>>c;
	cin>>d;
	cout<<a<<endl;
	cout<<b<<endl;
	cout<<c<<endl;
	cout<<d<<endl;
	cin>>a>>b>>c>>d;
	cout<<a<<b<<c<<d<<endl;
	cout<<"Esta linea no lleva enter al final";
	return 0;
}

