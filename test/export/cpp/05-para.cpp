#include<iostream>
using namespace std;
int main() {
	float a[10];
	float b[3][6];
	float c;
	int i;
	int j;
	for (i=1;i<=10;i++) {
		a[i-1]=i*10;
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
			cout<<b[i-1][j-1]<<endl;
		}
	}
	return 0;
}
