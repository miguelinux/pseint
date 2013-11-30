// Este codigo ha sido generado por el modulo psexport 20131129-l64 de PSeInt
// dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental
// puede que el codigo generado no sea completamente correcto. Si encuentra
// errores por favor reportelos en el foro (http://pseint.sourceforge.net).

#include<iostream>
using namespace std;

int main() {
	float c;
	cin>>c;
	switch (int(c)) {
	case 1:
		cout<<"1"<<endl;
		break;
	case 3: case 4: case 9:
		cout<<"3, 5 o 9"<<endl;
		break;
	case 7:
		cout<<"7"<<endl;
		break;
	default:
		cout<<"ni 1, ni 3, ni 5, ni 9, ni 7"<<endl;
		break;
	}
	return 0;
}

