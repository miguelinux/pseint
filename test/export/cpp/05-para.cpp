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

int main() {
	float ;
	float a;
	SIN_TIPO b;
	float c;
	SIN_TIPO elemento;
	float i;
	SIN_TIPO j;
	float x;
	SIN_TIPO a[0][;
	SIN_TIPO b[0][93][;
	for (i=1;i<=10;i++) {
		a[i]=i*10;
	}
	for (typeof(&(a[0])) ptr_aux=&(a[0]);ptr_aux<=&(a[9]);ptr_aux++) {
		typeof(a[0]) &elemento=*ptr_aux;
		cout<<elemento<<endl;
	}
	c=0;
	for (typeof(&(b[0][0])) ptr_aux=&(b[0][0]);ptr_aux<=&(b[2][5]);ptr_aux++) {
		typeof(b[0][0]) &x=*ptr_aux;
		c=c+1;
		x=c;
	}
	for (i=3;i>=1;i--) {
		for (j=1;j<=5;j+=2) {
			cout<<b[i][j]<<endl;
		}
	}
	return 0;
}

