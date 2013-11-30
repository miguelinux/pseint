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

// forward declarations
float funciondoble(float b);
void noretornanada(SIN_TIPO a, SIN_TIPO b);
void porreferencia(float b);
void recibevector(SIN_TIPO v);
void recibematriz(SIN_TIPO m);

int main() {
	float c;
	SIN_TIPO a[0][;
	SIN_TIPO b[0][93][;
	cout<<funciondoble(5)<<endl;
	noretornanada(3,9);
	c=0;
	porreferencia(c);
	cout<<c<<endl;
	recibevector(a);
	recibematriz(b);
	return 0;
}

float funciondoble(float b) {
	a=2*b;
	return a;
}

void noretornanada(SIN_TIPO a, SIN_TIPO b) {
	cout<<a+b<<endl;
}

void porreferencia(float b) {
	b=7;
}

void recibevector(SIN_TIPO v) {
	cin>>v[1];
}

void recibematriz(SIN_TIPO m) {
	cin>>m[1][1];
}

