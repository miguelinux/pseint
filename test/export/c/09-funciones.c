#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<ctype.h>
using namespace std;
char *convertiratexto(float f);
char *mayusculas(const char *s);
char *minusculas(const char *s);
#define MAX_STRLEN 256
int main() {
	char s[MAX_STRLEN];
	float x;
	x=1.5;
	printf("%f\n",sqrtf(x));
	printf("%f\n",abs(x));
	printf("%f\n",log(x));
	printf("%f\n",exp(x));
	printf("%f\n",sin(x));
	printf("%f\n",cos(x));
	printf("%f\n",tan(x));
	printf("%f\n",asin(x));
	printf("%f\n",acos(x));
	printf("%f\n",atan(x));
	printf("%f\n",floor((x)+.5));
	printf("%f\n",floor(x));
	printf("%f\n",(rand()%15));
	s="Hola";
	printf("%f\n",strlen(s));
	printf("%s\n",minusculas(s));
	printf("%s\n",mayusculas(s));
	printf("%s\n",s.substr(1,2-1+1));
	printf("%s\n",(s+" Mundo"));
	printf("%s\n",(string("Mundo ")+s));
	printf("%f\n",atof("15.5"));
	printf("%s\n",convertiratexto(15.5));
	return 0;
}
char *convertiratexto(float f) {
	char *buf=get_aux_buffer();
	sprintf(buf,"%f",f);
	return buf;
}
char *mayusculas(const char *s) {
	char *buf=get_aux_buffer();
	for(unsigned int i=0;i<s.size();i++)
		buf[i]=toupper(s[i]);
	return buf;
}
char *minusculas(const char *s) {
	char *buf=get_aux_buffer();
	for(unsigned int i=0;i<s.size();i++)
		buf[i]=tolower(s[i]);
	return buf;
}
