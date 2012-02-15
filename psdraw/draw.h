#include <iostream>
#include <fstream>
#include <list>
#include <math.h>

using namespace std;

int get_text_ancho(string s);

// constantes del dibujo

int c_font_size=13;
int c_size=500;

int c_borde=5;
int c_margen=30;
int c_margenint=5;
int c_margencond=25;

int c_espacio=13;
int c_flecha=4;
int c_flechaio=4;

int c_textline=13;
int c_textancho=8;
int c_textalto=13;

int c_colblanco=0;
int c_colnegro=1;
int c_colazul=2;
int c_colrojo=3;
int c_colverde=4;

// tipos auxiliares
struct t_intret {
	int alto,ancho;
	int derecha, izquierda;
};

struct t_pos {
	int x,y;
}; 

struct t_text {
	int ancho,alto;
	string texto;
};

struct t_linea{
	int x1,y1;
	int x2,y2;
	t_color color;
};

struct t_circulo{
	int x,y;
	int ancho,alto;
	t_color color;
};

struct t_texto {
	int x,y;
	t_color color;
	string texto;
};

struct t_instruccion{
	string nombre;
	string par1,par2,par3,par4;
};

// estructuras auxiliares
t_color e_colores[10];
#define t_programa list <t_instruccion>
struct t_dibujo{
	int x, y;
	int ancho, alto;
	int derecha,izquierda;
	list <t_linea> lineas;
	list <t_circulo> circulos;
	list <t_texto> textos;
	list <t_dibujo> dibujos;
};

void fix_replace(string &s, int i, string s2) {
	if (i>0 && s[i-1]!=' ' && s[i-1]!='(') s2=string(" ")+s2;
	if (i<s.size()-1 && s[i+1]!=' ') s2=s2+" ";
	s.replace(i,1,s2);
}

void fix_texto(string &s) {
	bool comillas=false;
	int i=0, l=s.size();
	while (i<l) {
		if (s[i]=='\'' || s[i]=='\"')
			comillas=!comillas;
		else if (!comillas) {
			if (s[i]=='|')
				fix_replace(s,i,"o");
			else if (s[i]=='&')
				fix_replace(s,i,"y");
			else if (s[i]=='%')
				fix_replace(s,i,"mod");
			else if (s[i]=='~')
				fix_replace(s,i,"no");
//			else if (s[i]=='^')
//				s[i]=255;
		}
		i++;
	}
}


// funciones auxiliares
inline int max(int a,int b){ return (a>b)?a:b; }
inline int min(int a,int b){ return (a<b)?a:b; }
bool LeftCompare(string a, string b) {  
	// compara los caracteres de la izquierda de a con b
	bool ret;
	if (a.size()<b.size()) ret=false; else {
		a.erase(b.size(),a.size()-b.size());
		if (a==b) ret=true; else ret=false; }
	return ret; 
}

string CutString(string s, int a, int b=0){
	string r=s;
	r.erase(0,a);
	if (b!=0) r.erase(r.size()-b,b);
	return r;
}
t_text a_texto(string s);

// funciones de dibujo
void d_cuadro(t_dibujo &d, int x1, int y1, int x2, int y2, t_color c);
void d_rombo(t_dibujo &d, int x1, int y1, int x2, int y2, t_color c);
void d_ovalo(t_dibujo &d, int x1, int y1, int x2, int y2, t_color c);
void d_inout(t_dibujo &d, int x1, int y1, int x2, int y2, int corr, t_color c);
void d_flechaio(t_dibujo &d, int x, int y, bool adentro, t_color c);
void d_flechaa(t_dibujo &d, int x, int y, t_color c);
void d_flechab(t_dibujo &d, int x, int y, t_color c);
void d_texto(t_dibujo &d, int x, int y, string s, t_color c);
void d_circulo(t_dibujo &d, int x, int y, int ancho, int alto, t_color c);
void d_flecha_abajo(t_dibujo &d, int x, int y, t_color c);
void d_flecha_arriba(t_dibujo &d, int x, int y, t_color c);
void d_linea(t_dibujo &d, int x1, int y1, int x2, int y2, t_color c);

// instrucciones
t_intret i_escribir(t_dibujo &d, string param, t_pos p);
t_intret i_leer(t_dibujo &d, string param, t_pos p);
t_intret i_secuencial(t_dibujo &d, string param, t_pos p);
t_intret i_asignacion(t_dibujo &d, string param1, string param2, t_pos p);
t_intret i_proceso(t_dibujo &d, string param, t_pos p);
t_intret i_si(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, list <t_instruccion>::iterator s, t_pos p);
t_intret i_mientras(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p);
t_intret i_segun(t_dibujo &d, list<list <t_instruccion>::iterator> its, t_pos p);
t_intret i_repetir(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p);
t_intret i_para(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p);
t_intret i_paracada(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p);
t_intret i_bloque(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p);

// otras funciones
t_programa cargar(string archivo);
t_dibujo diagrama(t_programa programa);

//-------------------------------------------------------------------------------------------------

// funciones de dibujo
void d_cuadro(t_dibujo &d, int x1, int y1, int x2, int y2, t_color c){
	t_linea l; l.color=c;
	l.x1=x1; l.x2=x2; l.y1=l.y2=y1; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x1; l.x2=x2; l.y1=l.y2=y2; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y1; l.y2=y2; l.x1=l.x2=x2; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y1; l.y2=y2; l.x1=l.x2=x1; 
	d.lineas.insert(d.lineas.end(),l);
}

void d_rombo(t_dibujo &d, int x1, int y1, int x2, int y2, t_color c){
	t_linea l; l.color=c;
	l.x1=x1; l.y1=(y1+y2)/2; l.x2=(x1+x2)/2; l.y2=y1; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x1; l.y1=(y1+y2)/2; l.x2=(x1+x2)/2; l.y2=y2; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x2; l.y1=(y1+y2)/2; l.x2=(x1+x2)/2; l.y2=y1; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x2; l.y1=(y1+y2)/2; l.x2=(x1+x2)/2; l.y2=y2; 
	d.lineas.insert(d.lineas.end(),l);
}

void d_ovalo(t_dibujo &d, int x1, int y1, int x2, int y2, t_color c){
	t_linea l; l.color=c;
	l.x1=x1; l.x2=x2; l.y1=l.y2=y1; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x1; l.x2=x2; l.y1=l.y2=y2; 
	d.lineas.insert(d.lineas.end(),l);
	
	int a3=(y2-y1)/3; if (a3<0) a3=-a3;
	
	l.y1=y1; l.x1=x1; l.y2=y1+a3; l.x2=x1-a3; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y2; l.x1=x1; l.y2=y2-a3; l.x2=x1-a3; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y1+a3; l.x1=x1-a3; l.y2=y2-a3+1; l.x2=x1-a3; 
	d.lineas.insert(d.lineas.end(),l);
	
	l.y1=y1; l.x1=x2; l.y2=y1+a3; l.x2=x2+a3; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y2; l.x1=x2; l.y2=y2-a3; l.x2=x2+a3; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y1+a3; l.x1=x2+a3; l.y2=y2-a3+1; l.x2=x2+a3; 
	d.lineas.insert(d.lineas.end(),l);
	
}

void d_inout(t_dibujo &d, int x1, int y1, int x2, int y2, int corr, t_color c){
	t_linea l; l.color=c;
	l.x1=x1; l.x2=x2+corr; l.y1=l.y2=y1; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x1-corr; l.x2=x2; l.y1=l.y2=y2; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y1; l.y2=y2; l.x1=x2+corr; l.x2=x2; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y1; l.y2=y2; l.x1=x1; l.x2=x1-corr; 
	d.lineas.insert(d.lineas.end(),l);
}

void d_flechaio(t_dibujo &d, int x, int y, bool adentro, t_color c){
	t_linea l; l.color=c;
	l.x1=x; l.x2=x+c_flechaio*2; 
	l.y1=y; l.y2=y-c_flechaio*2; 
	d.lineas.insert(d.lineas.end(),l);
	if (adentro) {
		l.x1=x; l.x2=x; 
		l.y1=y; l.y2=y-c_flechaio; 
		d.lineas.insert(d.lineas.end(),l);
		l.x1=x; l.x2=x+c_flechaio; 
		l.y1=y; l.y2=y; 
		d.lineas.insert(d.lineas.end(),l);
	} else {
		l.x1=x+c_flechaio*2; l.x2=x+c_flechaio; 
		l.y1=y-c_flechaio*2; l.y2=y-c_flechaio*2; 
		d.lineas.insert(d.lineas.end(),l);
		l.x1=x+2*c_flechaio; l.x2=x+2*c_flechaio; 
		l.y1=y-c_flechaio; l.y2=y-c_flechaio*2; 
		d.lineas.insert(d.lineas.end(),l);
	}
}

void d_flechaa(t_dibujo &d, int x, int y, t_color c){
	t_linea l; l.color=c;
	l.x1=x; l.x2=x+c_flechaio*2; 
	l.y1=y; l.y2=y; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x; l.x2=x+c_flechaio; 
	l.y1=y; l.y2=y+c_flechaio; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x; l.x2=x+c_flechaio; 
	l.y1=y; l.y2=y-c_flechaio; 
	d.lineas.insert(d.lineas.end(),l);
}

void d_flechab(t_dibujo &d, int x, int y, t_color c){
	t_linea l; l.color=c;
	l.x1=x; l.x2=x+c_flechaio*2; 
	l.y1=y; l.y2=y; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x+2*+c_flechaio; l.x2=x+c_flechaio; 
	l.y1=y; l.y2=y+c_flechaio; 
	d.lineas.insert(d.lineas.end(),l);
	l.x1=x+2*+c_flechaio; l.x2=x+c_flechaio; 
	l.y1=y; l.y2=y-c_flechaio; 
	d.lineas.insert(d.lineas.end(),l);
}

void d_texto(t_dibujo &d, int x, int y, string s, t_color c){
	t_texto t; t.texto=s;
	t.x=x; t.y=y+c_textline; t.color=c;
	d.textos.insert(d.textos.end(),t);
}

void d_circulo(t_dibujo &d, int x, int y, int ancho, int alto, t_color c){
	t_circulo t; t.color=c;
	t.ancho=ancho; t.alto=alto;
	t.x=x; t.y=y; 
	d.circulos.insert(d.circulos.end(),t);
}

void d_flecha_abajo(t_dibujo &d, int x, int y, t_color c){
	t_linea l; l.color=c;
	l.x1=l.x2=x; l.y1=y; l.y2=y+c_espacio;
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y+c_espacio; l.y2=y+c_espacio-c_flecha; 
	l.x1=x; l.x2=x-c_flecha; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y+c_espacio; l.y2=y+c_espacio-c_flecha; 
	l.x1=x; l.x2=x+c_flecha; 
	d.lineas.insert(d.lineas.end(),l);
}

void d_flecha_arriba(t_dibujo &d, int x, int y, t_color c){
	t_linea l; l.color=c;
	l.x1=l.x2=x; l.y1=y; l.y2=y-c_espacio;
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y-c_espacio; l.y2=y-c_espacio+c_flecha; 
	l.x1=x; l.x2=x-c_flecha; 
	d.lineas.insert(d.lineas.end(),l);
	l.y1=y-c_espacio; l.y2=y-c_espacio+c_flecha; 
	l.x1=x; l.x2=x+c_flecha; 
	d.lineas.insert(d.lineas.end(),l);
}

void d_linea(t_dibujo &d, int x1, int y1, int x2, int y2, t_color c){
	t_linea l; l.color=c;
	l.x1=x1; l.x2=x2; l.y1=y1; l.y2=y2;
	d.lineas.insert(d.lineas.end(),l);
}

// funciones auxiliares
t_text a_texto(string s){
	fix_texto(s);
	t_text ret;
//	ret.ancho=s.size()*c_textancho;
	ret.ancho=get_text_ancho(s);
	ret.alto=c_textalto;
	ret.texto=s;
	return ret;
}

// instrucciones
t_intret i_escribir(t_dibujo &d, string param, t_pos p){

	//flecha
	d_flecha_abajo(d,p.x,p.y,c_colrojo);
	p.y=p.y+c_espacio;
	
	t_intret ret; 
	t_text t=a_texto(param);
	int corr=8;
	d_inout(d,p.x-(c_margenint+t.ancho/2),p.y,p.x+(c_margenint+t.ancho/2),p.y+t.alto+2*c_margenint,corr,c_colnegro);
	d_flechaio(d,p.x+t.ancho/2,p.y+c_margenint,false,c_colazul);
	d_texto(d,p.x-t.ancho/2,p.y+c_margenint,t.texto,c_colazul);
	ret.alto=c_margenint*2+t.alto+c_espacio;
	ret.ancho=t.ancho+c_margenint*2;
	ret.derecha=ret.izquierda=ret.ancho/2;
	return ret;
}

t_intret i_leer(t_dibujo &d, string param, t_pos p){

	//flecha
	d_flecha_abajo(d,p.x,p.y,c_colrojo);
	p.y=p.y+c_espacio;
	
	t_intret ret; 
	t_text t=a_texto(param);
	int corr=8;
	d_inout(d,p.x-(c_margenint+t.ancho/2),p.y,p.x+(c_margenint+t.ancho/2),p.y+t.alto+2*c_margenint,corr,c_colnegro);
	d_flechaio(d,p.x+t.ancho/2,p.y+c_margenint,true,c_colazul);
	d_texto(d,p.x-t.ancho/2,p.y+c_margenint,t.texto,c_colazul);
	ret.alto=c_margenint*2+t.alto+c_espacio;
	ret.ancho=t.ancho+c_margenint*2;
	ret.derecha=ret.izquierda=ret.ancho/2;
	return ret;
}

t_intret i_secuencial(t_dibujo &d, string param, t_pos p){
	//flecha
	d_flecha_abajo(d,p.x,p.y,c_colrojo);
	p.y+=c_espacio;
	
	t_intret ret; 
	t_text t=a_texto(param);
	d_cuadro(d,p.x-(c_margenint+t.ancho/2),p.y,p.x+(c_margenint+t.ancho/2),p.y+t.alto+2*c_margenint,c_colnegro);
	d_texto(d,p.x-t.ancho/2,p.y+c_margenint,t.texto,c_colazul);
	ret.alto=c_margenint*2+t.alto+c_espacio;
	ret.ancho=t.ancho+c_margenint*2;
	ret.derecha=ret.izquierda=ret.ancho/2;
	return ret;
}

t_intret i_asignacion(t_dibujo &d, string param1, string param2, t_pos p){
	
	//flecha
	d_flecha_abajo(d,p.x,p.y,c_colrojo);
	p.y=p.y+c_espacio;
	
	t_intret ret; 
	t_text t1=a_texto(param1);
	t_text t2=a_texto(param2);
	if (t1.alto>t2.alto) 
		ret.alto=c_margenint*2+t1.alto+c_espacio;
	else
		ret.alto=c_margenint*2+t2.alto+c_espacio;
	
	d_cuadro(d,p.x-c_margenint-(4*c_flechaio+t1.ancho+t2.ancho)/2,p.y,p.x+c_margenint+(4*c_flechaio+t1.ancho+t2.ancho)/2,p.y+ret.alto-c_espacio,c_colnegro);
	d_flechaa(d,p.x-(3*c_flechaio+t1.ancho+t2.ancho)/2+t1.ancho,p.y+(ret.alto-c_espacio)/2,c_colazul);
	
	d_texto(d,p.x-(4*c_flechaio+t1.ancho+t2.ancho)/2,p.y+c_margenint,t1.texto,c_colazul);
	d_texto(d,p.x-(-4*c_flechaio+t1.ancho+t2.ancho)/2+t1.ancho,p.y+c_margenint,t2.texto,c_colazul);
	
	ret.ancho=t1.ancho+t2.ancho+4*c_flechaio+c_margenint*2;
	ret.derecha=ret.izquierda=ret.ancho/2;
	return ret;
}

t_intret i_proceso(t_dibujo &d, string param, t_pos p){
	t_intret ret; 
	t_text t=a_texto(param);
	d_ovalo(d,p.x-(c_margenint+t.ancho/2),p.y,p.x+(c_margenint+t.ancho/2),p.y+t.alto+2*c_margenint,c_colnegro);
	d_texto(d,p.x-t.ancho/2,p.y+c_margenint,t.texto,c_colrojo);
	ret.alto=c_margenint*2+t.alto;
	ret.ancho=t.ancho+c_margenint*2;
	ret.derecha=ret.izquierda=ret.ancho/2;
	return ret;
}

t_intret i_si(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, list <t_instruccion>::iterator s, t_pos p){

	
	t_intret ret,i1,i2; 
	t_text t=a_texto((*r).par1);
	
	//flecha
	d_flecha_abajo(d,p.x,p.y,c_colrojo);
	p.y=p.y+c_espacio;
	
	// condicion
	int ralto,rancho;
	rancho=(t.ancho+c_margenint)+2*(t.alto+c_margenint);
	ralto=(t.ancho+c_margenint)/2+(t.alto+c_margenint);
	if (rancho>2*ralto) ralto=rancho/2; else rancho=2*ralto;
	if (ralto>5*c_textalto) ralto=5*c_textalto;
	d_rombo(d,p.x-rancho/2,p.y,p.x+rancho/2,p.y+ralto,c_colnegro);
	d_texto(d,p.x-t.ancho/2,p.y+ralto/2-t.alto/2,t.texto,c_colazul);
	p.y+=ralto;
	t_pos ap; 
	t_dibujo d1,d2;
	++r;
	
	// procesar bloque por verdadero
	ap.x=ap.y=0;
	i1=i_bloque(d1,++r,q,ap);
	d1.y=p.y;
	d1.x=p.x+c_margencond+max(i1.izquierda,rancho/2);
	
	// procesar bloque por falso
	if  (q!=s) {
		ap.x=ap.y=0;
		i2=i_bloque(d2,++q,s,ap);
		i2.ancho=max(i2.ancho,rancho/2);
		i2.izquierda=max(i2.izquierda,rancho/2);
	} else {
		i2.alto=0;
		i2.derecha=0;
		i2.ancho=0;
		i2.izquierda=0;
	}
	d2.y=p.y;
	d2.x=p.x-c_margencond-max(i2.derecha,rancho/2);
	
	
	// guardar tamaños
	ret.derecha=(d1.x-p.x)+i1.derecha;
	ret.izquierda=(p.x-d2.x)+i2.izquierda;

	if (i1.ancho>i2.ancho)
		ret.ancho=(i1.ancho+c_margencond)*2;
	else
		ret.ancho=(i2.ancho+c_margencond)*2;
	if (i1.alto>i2.alto)
		ret.alto=i1.alto+c_espacio*2+ralto;
	else
		ret.alto=i2.alto+c_espacio*2+ralto;
	if (i1.alto>i2.alto) 
		p.y+=i1.alto; 
	else 
		p.y+=i2.alto;
	
	p.y+=c_espacio;

	// completar las lineas que faltan
	d_linea(d,d1.x,d1.y,d1.x,d1.y-ralto/2,c_colrojo);
	d_linea(d,d2.x,d2.y,d2.x,d2.y-ralto/2,c_colrojo);
	
	d_linea(d,d1.x,d1.y-ralto/2,p.x+rancho/2,d1.y-ralto/2,c_colrojo);
	d_linea(d,d2.x,d2.y-ralto/2,p.x-rancho/2,d2.y-ralto/2,c_colrojo);
	
	d_texto(d,p.x+rancho/2+c_flechaio,d1.y-ralto/2-c_textalto-2-2,"V",c_colrojo);
	if  (q!=s) d_texto(d,p.x-rancho/2-c_flechaio-c_textancho,d2.y-ralto/2-c_textalto-2,"F",c_colrojo);

	d_linea(d,d1.x,d1.y+i1.alto,d1.x,p.y,c_colrojo);
	d_linea(d,d2.x,d2.y+i2.alto,d2.x,p.y,c_colrojo);
	
	d_linea(d,d1.x,p.y,d2.x,p.y,c_colrojo);
	
	d.dibujos.insert(d.dibujos.end(),d1);
	d.dibujos.insert(d.dibujos.end(),d2);
		
	return ret;
}

t_intret i_mientras(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p){
	t_intret ret,i; 
	t_text t=a_texto((*r).par1);
	
	// linea
	d_linea(d,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	// flecha
	d_flecha_abajo(d,p.x,p.y,c_colrojo);
	p.y=p.y+c_espacio;
	
	// condicion
	int ralto,rancho;
	rancho=(t.ancho+c_margenint)+2*(t.alto+c_margenint);
	ralto=(t.ancho+c_margenint)/2+(t.alto+c_margenint);
	if (rancho>2*ralto) ralto=rancho/2; else rancho=2*ralto;
	if (ralto>5*c_textalto) ralto=5*c_textalto;
	d_rombo(d,p.x-rancho/2,p.y,p.x+rancho/2,p.y+ralto,c_colnegro);
	d_texto(d,p.x-t.ancho/2,p.y+ralto/2-t.alto/2,t.texto,c_colazul);
	p.y+=ralto;

	// linea
	d_linea(d,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
		
		
	// bloque
	i=i_bloque(d,++r,q,p);
	p.y=p.y+i.alto;

	// linea
	d_linea(d,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	// calcular tamaño
	if (i.ancho>rancho) ret.ancho=i.ancho; else ret.ancho=rancho;
	ret.derecha=max(rancho/2,i.derecha)+c_margencond;
	ret.izquierda=max(rancho/2,i.izquierda)+c_margencond;
	ret.ancho+=c_margencond*2;
	ret.alto=ralto+i.alto+2*c_espacio;

	// cerrar el bucle
	d_linea(d,p.x,p.y,p.x-ret.izquierda,p.y,c_colrojo);
	d_linea(d,p.x-ret.izquierda,p.y,p.x-ret.izquierda,p.y-ret.alto-c_espacio,c_colrojo);
	d_linea(d,p.x,p.y-ret.alto-c_espacio,p.x-ret.izquierda,p.y-ret.alto-c_espacio,c_colrojo);
	d_flechab(d,p.x-c_flechaio*2, p.y-ret.alto-c_espacio,c_colrojo);

	d_texto(d,p.x+c_flechaio+rancho/2,p.y-ret.alto+ralto/2-c_textalto-2,"F",c_colrojo);
	d_texto(d,p.x+c_flechaio,p.y-ret.alto+ralto,"V",c_colrojo);
	
	d_linea(d,p.x+rancho/2,p.y-ret.alto+ralto/2,p.x+ret.derecha,p.y-ret.alto+ralto/2,c_colrojo);
	d_linea(d,p.x+ret.derecha,p.y+c_espacio,p.x+ret.derecha,p.y-ret.alto+ralto/2,c_colrojo);
	d_linea(d,p.x,p.y+c_espacio,p.x+ret.derecha,p.y+c_espacio,c_colrojo);

		
	ret.alto+=3*c_espacio;

	return ret;
}

t_intret i_segun(t_dibujo &d, list<list <t_instruccion>::iterator> its, t_pos p){
	t_intret ret,i; 
	t_text t;
	list<list <t_instruccion>::iterator>::iterator r2=its.begin(),r1;
	list <t_instruccion>::iterator it1,it2;
	r2++; r1=r2++;
	if (r2==its.end()) return ret;
	list <t_dibujo> ds;
	int dalto,dancho;
	
	// para que de_otro_modo aparezca si o si
	bool de_otro_modo=false;
	list<t_instruccion> dom_list;
	t_instruccion dom_inst;
	dom_inst.par1="DE OTRO MODO";
	dom_list.push_back(dom_inst);
	dom_list.push_back(dom_inst);
	
	//flecha
	d_flecha_abajo(d,p.x,p.y,c_colrojo);
	p.y=p.y+c_espacio;
	
	int talto=0; // guardar el texto mas alto;
	ret.alto=0; ret.ancho=(its.size()*2-4)*c_margencond;
	// bloques internos
	while (r2!=its.end() || !de_otro_modo) {
		t_dibujo d1;
		// calcular texto
		t_text t;
		if (r2!=its.end()) {
			t=a_texto((*(*r1)).par1);
			if ((*(*r1)).par1=="DE OTRO MODO") de_otro_modo=true;
		} else {
			ret.ancho+=c_margencond*2;
			t=a_texto("DE OTRO MODO");
			de_otro_modo=true;
		}
		if (t.alto>talto) talto=t.alto;
		// dibujar bloque
		if (r2!=its.end()) {
			it1=*(r1++); it2=*(r2++);
			i=i_bloque(d1,++it1,it2++,p);
		} else
			i.izquierda=i.derecha=i.alto=i.ancho=0;
		d1.ancho=max(t.ancho+c_margenint*2,i.ancho);
		d1.derecha=i.derecha+max((t.ancho-(i.derecha+i.izquierda))/2,0);
		d1.izquierda=i.izquierda+max((t.ancho-(i.derecha+i.izquierda))/2,0);
		d1.alto=i.alto+c_espacio;
		// ajustar anchos y altos
		if (i.alto>ret.alto) 
			ret.alto=i.alto;
		ret.ancho+=d1.izquierda+d1.derecha;
		d1.x=p.x; d1.y=0;
		ds.insert(ds.end(),d1);
	}
	ret.alto+=c_espacio;
	
	//dibujar el trianulo con la expresion
	dalto=max(min(c_textalto*5,ret.ancho/5),c_textalto*2);
	dancho=ret.ancho;
	int x=-ret.ancho/2;
	d_linea(d,p.x+x,p.y+dalto,0,p.y,c_colnegro);
	d_linea(d,p.x-x,p.y+dalto,0,p.y,c_colnegro);
	t=a_texto((*(*(its.begin()))).par1);
	p.y+=dalto; 
	d_texto(d,p.x-t.ancho/2,p.y-c_margenint-talto,t.texto,c_colazul);
	
	
	// acomodar en x y agregar dibujos
	list <t_dibujo>::iterator q=ds.begin();
	d_cuadro(d,p.x+x,p.y,p.x-x,p.y+talto+2*c_margenint,c_colnegro);
	x+=c_margencond;
	int lf1=min(p.x,x+(*q).izquierda),lf2=0;
	r1=its.begin(); r1++; r2=r1; r2++;
	while (q!=ds.end()) {
		t_dibujo d1=*(q);
		// escribir opcion y dibujar la linea separadora
		t_text t;
		if (r2!=its.end()) {
			t=a_texto((*(*(r1++))).par1);
			r2++;
		} else
			t=a_texto("DE OTRO MODO");
		if (++q!=ds.end()) d_linea(d,x+c_margencond+d1.izquierda+d1.derecha,p.y,x+c_margencond+d1.izquierda+d1.derecha,p.y+talto+2*c_margenint,c_colnegro);
		d_texto(d,x+d1.izquierda-t.ancho/2,p.y+c_margenint+talto/2-t.alto/2,t.texto,c_colazul);
		// acomodar dibujo
		d1.y+=talto+2*c_margenint+dalto;
		d1.x=x+d1.izquierda;
		x+=2*c_margencond+d1.izquierda+d1.derecha;
		// agregar dibujo y otras
		d.dibujos.insert(d.dibujos.end(),d1);
		lf2=d1.x;
		// agregar linea
		d_linea(d,lf2,p.y+talto+2*c_margenint+d1.alto-c_espacio,lf2,p.y+ret.alto+talto+2*c_margenint,c_colrojo);
	}
	// cerrar lineas de abajo
	p.y+=talto+2*c_margenint+ret.alto;
	d_linea(d,lf1,p.y,lf2,p.y,c_colrojo);
	
	ret.alto+=c_espacio+dalto+talto+2*c_margenint;
	ret.derecha=ret.izquierda=ret.ancho/2;
	return ret;
}

t_intret i_repetir(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p){
	t_intret ret,i; 
	t_text t=a_texto((*q).par1);
	bool hasta_que=(*q).nombre=="HASTAQUE";
	
	//linea
	d_linea(d,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	//bloque
	i=i_bloque(d,++r,q,p);
	p.y=p.y+i.alto;
	
	//flecha
	d_flecha_abajo(d,p.x,p.y,c_colrojo);
	p.y=p.y+c_espacio;
	
	// condicion
	int ralto,rancho;
	rancho=(t.ancho+c_margenint)+2*(t.alto+c_margenint);
	ralto=(t.ancho+c_margenint)/2+(t.alto+c_margenint);
	if (rancho>2*ralto) ralto=rancho/2; else rancho=2*ralto;
	if (ralto>5*c_textalto) ralto=5*c_textalto;
	d_rombo(d,p.x-rancho/2,p.y,p.x+rancho/2,p.y+ralto,c_colnegro);
	d_texto(d,p.x-t.ancho/2,p.y+ralto/2-t.alto/2,t.texto,c_colazul);
	p.y+=ralto;
	
	//calcular tamaño
//	if (i.ancho>rancho) ret.ancho=i.ancho; else ret.ancho=rancho;
	
	ret.izquierda=max(rancho/2,i.izquierda)+c_margencond;
	ret.derecha=max(rancho/2,i.derecha);
	
	ret.ancho=ret.izquierda+ret.derecha;
	
	ret.ancho+=c_margencond*2;
	ret.alto=ralto+i.alto+c_espacio;
	ret.alto+=c_espacio;

	//cerrar el bucle
	d_texto(d,p.x-rancho/2-c_textancho-c_flechaio,p.y-ralto/2-c_textalto-2,hasta_que?"F":"V",c_colrojo);
	
	d_linea(d,p.x-rancho/2,p.y-ralto/2,p.x-ret.izquierda,p.y-ralto/2,c_colrojo);
	d_linea(d,p.x-ret.izquierda,p.y-ralto/2,p.x-ret.izquierda,p.y-ret.alto+c_espacio,c_colrojo);
	d_linea(d,p.x-ret.izquierda,p.y-ret.alto+c_espacio,p.x,p.y-ret.alto+c_espacio,c_colrojo);
	d_flechab(d,p.x-c_flechaio*2, p.y-ret.alto+c_espacio,c_colrojo);
	
	return ret;
}


t_intret i_para(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p) {
	t_intret ret,i; 
	t_text t1=a_texto((*r).par1), t2=a_texto((*r).par2), t3=a_texto((*r).par3), t4=a_texto((*r).par4);
	
	//linea
	d_linea(d,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	t_dibujo d2;
	d2.x=0;
	d2.y=0;
	//bloque
	i=i_bloque(d2,++r,q,p);
	p.y=p.y+i.alto;
	
	
	int ralto,rancho=0;
	if (t1.ancho+2*c_margenint>t2.ancho+t3.ancho+t4.ancho+6*c_margenint)
		rancho=t1.ancho+2*c_margenint;
	else
		rancho=t2.ancho+t3.ancho+t4.ancho+6*c_margenint;
	rancho+=c_textancho*2	;
	ralto=2*rancho/3;
	if (ralto>c_textalto*5) ralto=c_textalto*5;
	
	if (ralto+c_espacio>i.alto) {
		d2.y=(ralto+c_espacio-i.alto)/2;
		p.y+=(ralto+c_espacio-i.alto);
		d_linea(d,p.x,p.y-i.alto-(ralto+c_espacio-i.alto)/2,p.x,p.y-i.alto-(ralto+c_espacio-i.alto),c_colrojo);
		d_linea(d,p.x,p.y,p.x,p.y-(ralto+c_espacio-i.alto)/2,c_colrojo);
		i.alto+=ralto+c_espacio-i.alto;
	}
	
	
	//linea
	d_linea(d,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	d.dibujos.insert(d.dibujos.end(),d2);
	
	d_circulo(d,p.x-i.izquierda-c_margencond-rancho,p.y-c_espacio/2-i.alto/2-ralto/2,rancho,ralto,c_colnegro);
	d_linea(d,p.x-c_margencond-rancho-i.izquierda,p.y-i.alto/2-c_espacio/2,p.x-c_margencond-i.izquierda,p.y-i.alto/2-c_espacio/2,c_colnegro);
	d_texto(d,p.x-c_margencond-rancho/2-t1.ancho/2-i.izquierda,p.y-c_textalto-i.alto/2-c_espacio/2-2,t1.texto,c_colazul);
	int z,x=p.x-c_margencond-rancho/2-i.izquierda-(4*c_margenint+t2.ancho+t3.ancho+t4.ancho)/2,y=p.y-i.alto/2-c_espacio/2;
	d_texto(d,x,y+3,t2.texto,c_colazul); x+=c_margenint*2+t2.ancho;
	z=(p.x-c_margencond-rancho/2-i.izquierda)-x;
	d_linea(d,x-c_margenint,y,x-c_margenint,y+1+(int)sqrt((1.0*ralto*ralto/4-z*z/(1.0*rancho*rancho/ralto/ralto))),c_colnegro);
	d_texto(d,x,y+3,t3.texto,c_colazul); x+=c_margenint*2+t3.ancho;
	z=(p.x-c_margencond-rancho/2-i.izquierda)-x;
	d_linea(d,x-c_margenint,y,x-c_margenint,y+1+(int)sqrt((1.0*ralto*ralto/4-z*z/(1.0*rancho*rancho/ralto/ralto))),c_colnegro);
	d_texto(d,x,y+3,t4.texto,c_colazul);
	
	d_linea(d,p.x,p.y,p.x-i.izquierda-rancho/2-c_margencond,p.y,c_colrojo);
	d_linea(d,p.x,p.y-i.alto-c_espacio,p.x-i.izquierda-rancho/2-c_margencond,p.y-i.alto-c_espacio,c_colrojo);
	d_linea(d,p.x-i.izquierda-rancho/2-c_margencond,p.y,p.x-i.izquierda-rancho/2-c_margencond,p.y-i.alto/2+ralto/2-c_espacio/2,c_colrojo);
	d_linea(d,p.x-i.izquierda-rancho/2-c_margencond,p.y-c_espacio-i.alto,p.x-i.izquierda-rancho/2-c_margencond,p.y-i.alto/2-ralto/2-c_espacio/2,c_colrojo);
	d_flecha_arriba(d,p.x-i.izquierda-c_margencond-rancho/2,p.y-i.alto/2+ralto/2+c_espacio/2+1,c_colrojo);
	d_flechab(d,p.x-c_flechaio*2, p.y-i.alto-c_espacio,c_colrojo);
	
	
	ret.alto=i.alto+2*c_espacio;
	ret.izquierda=i.izquierda+c_margencond+rancho;
	ret.derecha=i.derecha;
	ret.ancho=ret.izquierda+ret.derecha;
	
	return ret;
}

t_intret i_paracada(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p) {
	t_intret ret,i; 
	t_text t1=a_texto((*r).par1), t2=a_texto((*r).par2);
	
	//linea
	d_linea(d,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	t_dibujo d2;
	d2.x=0;
	d2.y=0;
	//bloque
	i=i_bloque(d2,++r,q,p);
	p.y=p.y+i.alto;
	
	
	int ralto,rancho=0;
	if (t1.ancho+2*c_margenint>t2.ancho+4*c_margenint)
		rancho=t1.ancho+2*c_margenint;
	else
		rancho=t2.ancho+4*c_margenint;
	rancho+=c_textancho*2	;
	ralto=2*rancho/3;
	if (ralto>c_textalto*5) ralto=c_textalto*5;
	
	if (ralto+c_espacio>i.alto) {
		d2.y=(ralto+c_espacio-i.alto)/2;
		p.y+=(ralto+c_espacio-i.alto);
		d_linea(d,p.x,p.y-i.alto-(ralto+c_espacio-i.alto)/2,p.x,p.y-i.alto-(ralto+c_espacio-i.alto),c_colrojo);
		d_linea(d,p.x,p.y,p.x,p.y-(ralto+c_espacio-i.alto)/2,c_colrojo);
		i.alto+=ralto+c_espacio-i.alto;
	}
	
	
	//linea
	d_linea(d,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	d.dibujos.insert(d.dibujos.end(),d2);
	
	d_circulo(d,p.x-i.izquierda-c_margencond-rancho,p.y-c_espacio/2-i.alto/2-ralto/2,rancho,ralto,c_colnegro);
	d_linea(d,p.x-c_margencond-rancho-i.izquierda,p.y-i.alto/2-c_espacio/2,p.x-c_margencond-i.izquierda,p.y-i.alto/2-c_espacio/2,c_colnegro);
	d_texto(d,p.x-c_margencond-rancho/2-t1.ancho/2-i.izquierda,p.y-c_textalto-i.alto/2-c_espacio/2-2,t1.texto,c_colazul);
	int z,x=p.x-c_margencond-rancho/2-i.izquierda-(2*c_margenint+t2.ancho)/2,y=p.y-i.alto/2-c_espacio/2;
	d_texto(d,x,y+3,t2.texto,c_colazul); x+=c_margenint*2+t2.ancho;
	z=(p.x-c_margencond-rancho/2-i.izquierda)-x;
//	d_linea(d,x-c_margenint,y,x-c_margenint,y+1+(int)sqrt((1.0*ralto*ralto/4-z*z/(1.0*rancho*rancho/ralto/ralto))),c_colnegro);
//	d_texto(d,x,y+3,t3.texto,c_colazul); x+=c_margenint*2+t3.ancho;
	z=(p.x-c_margencond-rancho/2-i.izquierda)-x;
//	d_linea(d,x-c_margenint,y,x-c_margenint,y+1+(int)sqrt((1.0*ralto*ralto/4-z*z/(1.0*rancho*rancho/ralto/ralto))),c_colnegro);
//	d_texto(d,x,y+3,t4.texto,c_colazul);
	
	d_linea(d,p.x,p.y,p.x-i.izquierda-rancho/2-c_margencond,p.y,c_colrojo);
	d_linea(d,p.x,p.y-i.alto-c_espacio,p.x-i.izquierda-rancho/2-c_margencond,p.y-i.alto-c_espacio,c_colrojo);
	d_linea(d,p.x-i.izquierda-rancho/2-c_margencond,p.y,p.x-i.izquierda-rancho/2-c_margencond,p.y-i.alto/2+ralto/2-c_espacio/2,c_colrojo);
	d_linea(d,p.x-i.izquierda-rancho/2-c_margencond,p.y-c_espacio-i.alto,p.x-i.izquierda-rancho/2-c_margencond,p.y-i.alto/2-ralto/2-c_espacio/2,c_colrojo);
	d_flecha_arriba(d,p.x-i.izquierda-c_margencond-rancho/2,p.y-i.alto/2+ralto/2+c_espacio/2+1,c_colrojo);
	d_flechab(d,p.x-c_flechaio*2, p.y-i.alto-c_espacio,c_colrojo);
	
	
	ret.alto=i.alto+2*c_espacio;
	ret.izquierda=i.izquierda+c_margencond+rancho;
	ret.derecha=i.derecha;
	ret.ancho=ret.izquierda+ret.derecha;
	
	return ret;
}

t_intret i_bloque(t_dibujo &d, list <t_instruccion>::iterator r, list <t_instruccion>::iterator q, t_pos p){
	t_intret ret,i; ret.ancho=0; ret.alto=0;	
	ret.derecha=ret.izquierda=0;
	if (r==q) return ret;
	int deep;
	string s;
	
	while (r!=q){
		s=(*r).nombre;
		i.ancho=i.alto=0;
		if (s=="ESCRIBIR") i=i_escribir(d,(*r).par1,p);
		else if (s=="LEER") i=i_leer(d,(*r).par1,p);
		else if (s=="ASIGNACION") i=i_asignacion(d,(*r).par1,(*r).par2,p);
		else if (s=="SECUENCIAL") i=i_secuencial(d,(*r).par1,p);
		else if (s=="MIENTRAS") {
			list <t_instruccion>::iterator r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINMIENTRAS"  && deep==0) ) {
				if ((*r).nombre=="MIENTRAS") deep++;
				else if ((*r).nombre=="FINMIENTRAS") deep--;
				r++;
			}
			i=i_mientras(d,r1,r,p);
		} else if (s=="REPETIR") {
			list <t_instruccion>::iterator r1=r++;
			deep=0;
			while ( deep || ((*r).nombre!="HASTAQUE" && (*r).nombre!="MIENTRASQUE") ) {
				if ((*r).nombre=="REPETIR") deep++;
				else if ((*r).nombre=="HASTAQUE"||(*r).nombre=="MIENTRASQUE") deep--;
				r++;
			}
			i=i_repetir(d,r1,r,p);
		} else if (s=="SEGUN") {
			list<list <t_instruccion>::iterator> its;
			its.insert(its.end(),r);
			deep=0;
			r++;
			while ( ! ((*r).nombre=="FINSEGUN" && deep==0) ) {
				if ((*r).nombre=="OPCION") 
					its.insert(its.end(),r);
				else if ((*r).nombre=="SEGUN") 
					deep++;
				else if ((*r).nombre=="FINSEGUN")
					deep--;
				r++;
			}
			its.insert(its.end(),r);
			i=i_segun(d,its,p);
		} else if (s=="PARA") {
			list <t_instruccion>::iterator r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINPARA"  && deep==0) ) {
				if ((*r).nombre=="PARA"||(*r).nombre=="PARACADA") deep++;
				else if ((*r).nombre=="FINPARA") deep--;
				r++;
			}
			i=i_para(d,r1,r,p);
		} else if (s=="PARACADA") {
			list <t_instruccion>::iterator r1=r++;
			deep=0;
			while ( ! ((*r).nombre=="FINPARA"  && deep==0) ) {
				if ((*r).nombre=="PARA"||(*r).nombre=="PARACADA") deep++;
				else if ((*r).nombre=="FINPARA") deep--;
				r++;
			}
			i=i_paracada(d,r1,r,p);
		} else if (s=="SI") {
			list <t_instruccion>::iterator r1=r++,r2;
			r++;
			deep=0;
			while ( ! ( ( (*r).nombre=="FINSI" || (*r).nombre=="SINO") && deep==0) ) {
				if ((*r).nombre=="SI") deep++;
				else if ((*r).nombre=="FINSI") deep--;
				r++;
			}
			r2=r;
			while ( ! ( (*r).nombre=="FINSI" && deep==0) ) {
				if ((*r).nombre=="SI") deep++;
				else if ((*r).nombre=="FINSI") deep--;
				r++;
			}
			i=i_si(d,r1,r2,r,p);
	

		}
		
		
		ret.ancho=max(i.ancho,ret.ancho);
		ret.derecha=max(i.derecha,ret.derecha);
		ret.izquierda=max(i.izquierda,ret.izquierda);
		ret.alto+=i.alto;
		p.y+=i.alto;
		r++;
	}
	
	return ret;
}

// otras funciones
t_programa cargar(string archivo){
	char buffer[1024];
	t_programa p;
	ifstream f(archivo.c_str());
	t_instruccion i;
	string s;
	size_t x;
	p.insert(p.begin(),i);
	if (!f.is_open()) {
		cerr<<"No se pudo abrir el archivo\n"; 
		(*(p.begin())).nombre="ERROR ERROR ERROR!";
		return p;
	}
	while (!f.eof()) {
		f.getline(buffer,256); s=buffer;
		if (s=="FINPROCESO"){ 
			break;
		} else if (s=="FINSI") {
			i.nombre="FINSI"; p.insert(p.end(),i);
		} else if (s=="ENTONCES") {
			i.nombre="ENTONCES"; p.insert(p.end(),i);
		} else if (s=="SINO") {
			i.nombre="SINO"; p.insert(p.end(),i);
		} else if (s=="FINSEGUN") {
			i.nombre="FINSEGUN"; p.insert(p.end(),i);
		} else if (s=="FINPARA") {
			i.nombre="FINPARA"; p.insert(p.end(),i);
		} else if (s=="FINMIENTRAS") {
			i.nombre="FINMIENTRAS"; p.insert(p.end(),i);
		} else if (s=="REPETIR") {
			i.nombre="REPETIR"; p.insert(p.end(),i);
		} else if (LeftCompare(s,"PROCESO ")) {
			(*(p.begin())).nombre=CutString(s,8);
		} else if (LeftCompare(s,"SEGUN ")) {
			i.nombre="SEGUN"; i.par1=CutString(s,6,6);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"PARACADA ")) {
			i.nombre="PARACADA"; s=CutString(s,9,6);
			x=s.find(" DE ",0);
			i.par1=s.substr(0,x);
			i.par2=s.substr(x+4);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"PARA ")) {
			i.nombre="PARA"; s=CutString(s,5,6);
			x=s.find("CON PASO  ",0);
			if (x==string::npos) x=s.find("CON PASO ",0); else x++;
			if (x!=string::npos) {
				i.par3=CutString(s,x+9);
				s=CutString(s,0,s.size()-x+1);
			} else i.par3="1";
			x=s.find(" ",0);
			i.par4=CutString(s,x+7);
			s=CutString(s,0,s.size()-x);
			x=s.find("<-",0);
			i.par2=CutString(s,x+3,1);
			i.par1=CutString(s,0,s.size()-x);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"HASTA QUE ")) {
			i.nombre="HASTAQUE"; i.par1=CutString(s,10);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"MIENTRAS QUE ")) {
			i.nombre="MIENTRASQUE"; i.par1=CutString(s,13);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"MIENTRAS ")) {
			i.nombre="MIENTRAS"; i.par1=CutString(s,9,6);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"SI ")) {
			i.nombre="SI"; i.par1=CutString(s,3);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"ESCRIBIR ")||LeftCompare(s,"ESCRIBNL ")) {
			i.nombre="ESCRIBIR"; i.par1=CutString(s,9,1);
			p.insert(p.end(),i);
		} else if (LeftCompare(s,"LEER ")) {
			i.nombre="LEER"; i.par1=CutString(s,5,1);
			p.insert(p.end(),i);
		} else if (s[s.size()-1]==':') {
			i.nombre="OPCION"; i.par1=CutString(s,0,1);
			p.insert(p.end(),i);
		} else if (s!="") {
			x=s.find("<-",0);
			if (x!=string::npos) {
				i.nombre="ASIGNACION"; 
				i.par1=CutString(s,0,s.size()-x);
				i.par2=CutString(s,x+3,2);
			} else {
				i.nombre="SECUENCIAL"; i.par1=s;
			}
			p.insert(p.end(),i);
		}
	}
	f.close();
	
	return p;
}

t_dibujo diagrama(t_programa programa){

	int dancho2;
	string s;
	t_pos p; 
	t_intret i;
	t_dibujo ret;
	list <t_instruccion>::iterator r=programa.begin(),q=programa.end();

	s="PROCESO ";
	s=s+(*r).nombre;
	int tancho=get_text_ancho(s);
	d_texto(ret,-tancho/2,c_borde+c_margen,s,c_colnegro);
	d_linea(ret,-tancho/2-c_flechaio,c_textalto+c_borde+c_margen+3,+tancho/2+c_flechaio,c_textalto+c_borde+c_margen+3,c_colnegro);
	d_linea(ret,-tancho/2-c_flechaio,c_textalto+c_borde+c_margen+5,+tancho/2+c_flechaio,c_textalto+c_borde+c_margen+5,c_colnegro);
	dancho2=tancho/2+c_flechaio;
	
	//dejar margen arriba
	p.y=c_borde+c_textalto+2*c_margen; p.x=0;
	
	//proceso nombre	
	i=i_proceso(ret,"INICIO",p);
	p.y=p.y+i.alto;


	//linea
	d_linea(ret,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	//cuerpo del proceso
	r++;
	i=i_bloque(ret,r,q,p);
	ret.derecha=max(dancho2,i.derecha);
	ret.izquierda=max(dancho2,i.izquierda);
	p.y=p.y+i.alto;
	ret.alto=ret.alto+i.alto;
	
	//linea
	d_linea(ret,p.x,p.y,p.x,p.y+c_espacio,c_colrojo);
	p.y=p.y+c_espacio;
	
	//flecha
	d_flecha_abajo(ret,p.x,p.y,c_colrojo);
	p.y=p.y+c_espacio;

	//finproceso
	i=i_proceso(ret,"FIN",p);
	p.y=p.y+i.alto;
	
	//margenes abajo y laterales
	ret.alto=p.y+c_borde+c_margen;
	ret.izquierda+=c_borde+c_margen;
	ret.derecha+=c_borde+c_margen;
	ret.ancho=ret.derecha+ret.izquierda;
	//ret.izquierda=ret.derecha=ret.ancho/2;
	d_cuadro(ret,-ret.izquierda+c_borde,c_borde,ret.derecha-c_borde,ret.alto-c_borde,c_colnegro);
	ret.x=ret.izquierda; ret.y=0;
	return ret;
	
}
