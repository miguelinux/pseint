// Para probar este programa hay que generar un archivo .psd con wxPSeInt y pasarlo como argumento a este ejecutable
// Este archivo se genera al visualizar el diagrama viejo en $home/.pseint/temp.psd
//
// El menú no funciona, el botón de ayuda en realidad imprime el pseudocódigo en la consola
// La estructura para no escribe lo que va en el circulo
// Hay muchos errores cuando se carga el código de un archivo .psd

#include <GL/glut.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <stack>
#include "../pseint/zockets.h"
#include <sstream>
using namespace std;

void dibujar_caracter(const char chr);

enum ETYPE { ET_LEER, ET_PROCESO, ET_ESCRIBIR, ET_SI, ET_SEGUN, ET_OPCION, ET_PARA, ET_MIENTRAS, ET_REPETIR, ET_ASIGNAR };

class Entity;

bool edit_on=true; // indica si se puede editar el diagrama
string fname; // archivo que recibe como argumento
ZOCKET zocket=ZOCKET_ERROR; // para comunicarse con wxPSeInt

Entity *edit=NULL; // entidad seleccionado para editar su texto
Entity *mouse=NULL; // entidad seleccionado por el mouse
Entity *start=NULL; // entidad donde comienza el algoritmo
Entity *entity_to_del=NULL; // para un delayed delete (cuando suelta uno que sale del shapebar y no queda en ningun lado)
Entity *all_any=NULL;
int m_x0=0; // click del mouse, para referencia en el motion, se setea en el mouse
int m_y0=0; // click del mouse, para referencia en el motion, se setea en el mouse
bool panning=false; // indica si se esta moviendo el dibujo, para el motion
bool shapebar=false; // indica si el mouse esta sobre la barra de formas
bool selecting_zoom=false; // para hacer zoom en un area marcada, con el boton del medio, m_x0 y m_y0 guardan la primer esquina
int shapebar_size=0; // ancho para dibujo de la shapebar
#define shapebar_size_min 25
#define shapebar_size_max 150
bool trash=false; // indica si el mouse esta sobre la papelera
int menu_size_h=0; // ancho para dibujo de la shapebar
int menu_size_w=0; // ancho para dibujo de la shapebar
bool menu=false; // indica si el mouse esta sobre la papelera
int edit_pos; // posición del cursor cuando se edita un texto
int blink=0; // cuando se esta editando texto, indica si se muestra o no el cursor, para que parpadee
int shapebar_sel=0; // indica cual tipo de entidad esta seleccionada (1...n, 0 es ninguna) si shapebar==true
int menu_sel=0; // indica cual elemento del menu esta seleccionado (1...n, 0 es ninguno) si menu==true
int trash_size=0; // ancho y alto de dibujo de la papelera
#define trash_size_max 100
#define trash_size_min 30
int d_dx=0; // "paning" del dibujo
int d_dy=0; // "paning" del dibujo
float d_zoom=1; // zoom del dibujo
float zoom=1; // zoom final
const int flecha_h=25; // separacion entre bloques consecutivos
const int flecha_w=20; // separacion entre bloques hermanos
const int flecha_d=5; // tamaño de la punta de la flecha
const int margin=6; // margen entre cuadro y texto en un bloque
const int selection_tolerance_y=12; // tolerancia en y para la seleccion de puntos
const int selection_tolerance_x=30; // tolerancia en x para la seleccion de puntos
const int shadow_delta_x=4; // diferencia entre la posicion de un objeto y su sombra
const int shadow_delta_y=5; // diferencia entre la posicion de un objeto y su sombra

const float color_border[3]={0,0,0};
const float color_label[3]={0,0,.9};
const float color_arrow[3]={.9,0,0};
const float color_selection[3]={0,.4,0};
const float color_shape[3]={1,1,.9};
const float color_shadow[3]={.7,.7,.7};
const float color_back[3]={.9,1,.9};
const float color_ghost[3]={.7,.7,.7};
const float color_menu[3]={.7,.2,.2};
const float color_menu_back[3]={.7,.9,.9};

const int circle_steps=20; // cantidad de tramos en los que aproximo el circulo para dibujarlo como poligonal
double cosx[circle_steps+1], sinx[circle_steps+1];

int win_h=400,win_w=600; // tamaño de la ventana
int cur_x, cur_y; // ubicacion del raton (en coord del dibujo)


// para interpolar en las animaciones
#define interpolate(a,b) a=(2*a+b)/3
//#define interpolate_good(a,b) a=(2*a+b)/3
#define interpolate_good(a,b) if ((a)+3>(b) && (a)-3<(b)) a=b; else a=(2*a+b)/3

void Salir() {
	if (zocket!=ZOCKET_ERROR) {
		zocket_cerrar(zocket);
	}
	exit(0);
}

struct Entity {
	Entity *all_next, *all_prev;
	static Entity *all_any;
	ETYPE type;
	int x,y; // posiciones reales absolutas, independientes del dibujo, del punto de entrada al bloque completo
	int fx,fy; // posiciones reales absolutas del punto de entrada a la forma de la entidad
	int d_fx,d_fy; // posiciones para el dibujo del punto de entrada a la forma de la entidad
	int w,h; // tamaño real
	int t_w,t_h; // tamaño del texto de la etiqueta
	int d_w, d_h; // tamaño de la forma en el dibujo, tiende a w,h con el tiempo
	int d_x, d_y; // posiciones del dibujo, tienden a x+d_dx,y+d_dy con el tiempo
	int m_x,m_y; // coordenadas del mouse relativas a x,y, para cuando se arrastra
	int bwl, bwr,bh; // tamaño del bloque completo real, incluyendo hijos y la cfl
	int d_bwl, d_bwr,d_bh; // tamaño del bloque completo en el dibujo, incluyendo hijos
	Entity *next,*prev,*parent;
	int child_id; // indice de this en el arreglo child del parent
	Entity **child; // arreglo de hijos (primer entidad de los sub-bloques, 1 para repetitivas, 2 para si, n para segun)
	int *child_dx; // posicion de los hijos relativa al x del padre(this)
	int *child_bh; // arreglo con el alto tiene cada hijo para que el padre sepa desde donde cerrar las flechas
	int n_child; // cantidad de hijos
	int flecha_in; // si las flechas de entrada son mas largas que lo normal (ej, entrada en un repetitivo), se pone aca la diferencia (esto se podria sacar, no?)
	Entity *nolink; // elemento seleccionado, para que los hijos se escondan atras del padre mientras se mueve al padre
	string label,lini,lfin,lpaso;
	Entity(ETYPE _type, string _label) :type(_type),label(_label) {
		if (label.size() && label[label.size()-1]==';') label=label.substr(0,label.size()-1);
		if (!all_any) { 
			all_any=this; 
			all_next=all_prev=this;
		} else { 
			all_next=all_any->all_next; 
			if (all_next) all_next->all_prev=this;
			all_any->all_next=this; 
			all_prev=all_any;
		}
		fx=x=0; fy=y=0; flecha_in=0;
		d_fx=d_fy=d_y=d_x=100;
		d_w=d_bh=d_h=d_bwl=d_bwr=bwl=bwr=0;
		nolink=NULL;
		SetLabel(label);
		parent=prev=next=NULL; child=NULL;
		if (type==ET_SI) { // dos hijos
			n_child=2;
			child_bh=(int*)malloc(sizeof(int)*2);
			child_dx=(int*)malloc(sizeof(int)*2);
			child=(Entity**)malloc(sizeof(Entity*)*2);
			child[0]=child[1]=NULL;
		} else if (type==ET_PARA||type==ET_MIENTRAS||type==ET_REPETIR||type==ET_OPCION||type==ET_SEGUN) { // un hijo
			n_child=1;
			child_bh=(int*)malloc(sizeof(int)*1);
			child_dx=(int*)malloc(sizeof(int)*1);
			child=(Entity**)malloc(sizeof(Entity*)*1);
			child[0]=NULL;
			if (type!=ET_OPCION && type!=ET_SEGUN) flecha_in=flecha_h; 
			else if (type==ET_SEGUN) LinkChild(0,new Entity(ET_OPCION,"De Otro Modo"));
		} else {
			n_child=0;
		}
	}
	~Entity() {
		if (entity_to_del==this) entity_to_del=NULL;
		for (int i=0;i<n_child;i++) {
			Entity *aux=child[i],*aux2;
			while (aux) {
				aux2=aux->next;
				delete aux;
				aux=aux2;
			}
		}
		UnLink();
		if (n_child) {
			free(child);
			free(child_dx);
			free(child_bh);
		}
		if (all_prev) all_prev->all_next=all_next;
		if (all_next) all_next->all_prev=all_prev;
		if (this==all_any) all_any=all_next?all_next:all_prev;
		if (this==mouse) UnSetMouse();
	}
	void SetEdit() {
		edit=this;
		edit_pos=label.size();
	}
	void SetMouse() {
		mouse=this;
		SetNolink(this,false);
	}
	void UnSetMouse() {
		mouse=NULL;
		SetNolink(NULL,false);
		if (!parent && !prev) {
			if (entity_to_del) delete entity_to_del;
			entity_to_del=this;
		}
	}
	void SetNolink(Entity *m,bool n) {
		nolink=m;
		for (int i=0;i<n_child;i++)
			if (child[i]) child[i]->SetNolink(m,true);
		if (n && next) next->SetNolink(m,true);
			
	}
	void EditSpecialLabel(unsigned char key) {
		if (key==GLUT_KEY_LEFT) {
			if (edit_pos>0) edit_pos--;
		}
		else if (key==GLUT_KEY_RIGHT) {
			if (edit_pos<int(label.size())) edit_pos++;
		}
		else if (key==GLUT_KEY_HOME) {
			edit_pos=0;
		}
		else if (key==GLUT_KEY_END) {
			edit_pos=label.size();
		}
	}
	void EditLabel(unsigned char key) {
		if (key=='\b') {
			if (edit_pos>0) {
				label.erase(--edit_pos,1);
				SetLabel(label,true);
			}
		} else if (key==127) {
			if (edit_pos<int(label.size())) {
				label.erase(edit_pos,1);
				SetLabel(label,true);
			}
		} else if (key==13 || key==27) {
			edit=NULL;
		} else {
			label.insert(edit_pos++,string(1,key));
			SetLabel(label,true);
		}
	}
	void SetLabel(string _label, bool recalc=false) {
		for (int i=0;i<label.size();i++) if (label[i]=='\'') label[i]='\"';
		label=_label; GetTextSize(t_w,t_h); w=t_w; h=t_h;
		if (!w) w=margin*6;
		h+=2*margin; if (type!=ET_PROCESO) w+=2*margin;
		if (type==ET_REPETIR||type==ET_MIENTRAS||type==ET_SI) {
			w*=2; h*=2;
		} else if (type==ET_ESCRIBIR||type==ET_LEER) {
			w+=2*margin;
		} else if (type==ET_PARA) {
			h*=2; w+=2*margin;
		} else if (type==ET_SEGUN) {
			h*=2;
		}
		if (recalc) Calculate();
		if (recalc && parent) parent->Calculate(true);
	}
	void GetTextSize(int &w, int &h) {
		w=label.size()*12;
		h=18;
	}
	int CheckLinkChild(int x, int y) {
		if (nolink||!n_child||type==ET_SEGUN) return -1;
		for (int i=0;i<n_child;i++) {
			if (child[i]!=mouse && d_x+child_dx[i]+selection_tolerance_x>x && d_x+child_dx[i]-selection_tolerance_x<x && y>d_y-d_h-2*flecha_h/3-selection_tolerance_y && y<d_y-d_h-2*flecha_h/3+selection_tolerance_y)
				return i;
		}
		return -1;
	}
	int CheckLinkOpcion(int x, int y) {
		if (y<d_dy+child[0]->y-child[0]->h || y>d_dy+child[0]->y+2*child[0]->h) return -1;
		if (x<d_dx+child[0]->x-child[0]->w || x>d_dx+child[n_child-1]->x+child[n_child-1]->w) return -1;
		for (int i=0;i<n_child-1;i++) {
			if (x<d_dx+child[i]->x) {
				if (i==mouse->child_id) return -1; else return i;
			}
		}
		return -1;
		
	}
	bool CheckLinkNext(int x, int y) {
		if (type==ET_OPCION || (type==ET_PROCESO && !next) || mouse==next || nolink) return false;
		return (x>d_x-d_w/2 && x<d_x+d_w/2 && y>d_y-d_bh-2*flecha_h/3-selection_tolerance_y && y<d_y-d_bh-2*flecha_h/3+selection_tolerance_y);
	}
	void UnLink() {
		if (next) next->prev=prev;
		if (prev) prev->next=next;
		if (parent) {
			if (type==ET_OPCION) {
				parent->RemoveChild(child_id);
			} else {
				if (parent->child[child_id]==this)
					parent->child[child_id]=next;
			}
		}
		parent=next=prev=NULL;
	}
	void LinkNext(Entity *e) {
		e->prev=this; e->next=next;
		if (next) next->prev=e;
		next=e;
		e->parent=parent;
		e->child_id=child_id;
	}
	
	void RemoveChild(int j) { // elimina un hijo de la lista, reduciendo n_child
		for (int i=j;i<n_child-1;i++) {
			child[i]=child[i+1];
			child_dx[i]=child_dx[i+1];
			child_bh[i]=child_bh[i+1];
			if (child[i]) child[i]->child_id=i;
		}
		n_child--;
	}
	void InsertChild(int i, Entity *e) { // similar a LinkChild, pero agrega un hijo, no reemplaza a uno que ya estaba
		LinkChild(n_child,e); // agrega al final
		MoveChild(n_child-1,i);
	}
	void MoveChild(int i0, int i1) {
		if (i0==i1) return;
		Entity *e0=child[i0];
		if (i0>i1) {
			for (int j=i0;j>i1;j--)
				child[j]=child[j-1];
		} else {
			for (int j=i0;j<i1;j++)
				child[j]=child[j+1];
		}
		child[i1]=e0;
		for (int j=0;j<n_child;j++) { // avisa a los hijos del movimiento
			Entity *aux=child[j];
			while (aux) {
				aux->child_id=j;
				aux=aux->next;
			}
		}
	}
	void LinkChild(int i, Entity *e) { // i esta en base 0 y no puede ser negativo ni mayor a n_child
		if (!n_child) { // si no tenia hijos, inicializar los arreglos child y child_dx
			n_child=1;
			child=(Entity**)malloc(sizeof(Entity*));
			child_dx=(int*)malloc(sizeof(int));
			child_bh=(int*)malloc(sizeof(int));
			child_dx[i]=child_bh[i]=0;
			child[i]=NULL;
		} else if (i==n_child) { // si tenia hijos, pero no tantos, agregar al final
			++n_child;
			child=(Entity**)realloc(child,sizeof(Entity*)*n_child);
			child_dx=(int*)realloc(child_dx,sizeof(int)*n_child);
			child_bh=(int*)realloc(child_bh,sizeof(int)*n_child);
			child_dx[i]=child_bh[i]=0;
			child[i]=NULL;
		}
		if (e) {
			e->prev=NULL; if (child[i]) child[i]->prev=e; 
			e->next=child[i]; child[i]=e; 
			e->parent=this;	e->child_id=i;
		}
	}
	void Tick() {
		// actualiza las variables que dicen como dibujar
		interpolate_good(d_bh,bh);
		interpolate(d_bwl,bwl); 
		interpolate(d_bwr,bwr);
		if (this==mouse) {
			interpolate(d_h,h);
			interpolate(d_w,w);
			return; // si se esta moviendo con el mouse, el mouse manda, asi que aca no se hace nada
		}
		if (nolink) { // si se esta moviendo como parte de un bloque, se esconde detras de ese bloque
			interpolate(d_x,nolink->d_x);
			interpolate(d_y,nolink->d_y-nolink->h/2);
			interpolate(d_fx,nolink->d_fx);
			interpolate(d_fy,nolink->d_fy-nolink->h/2);
			interpolate(d_h,0);
			interpolate(d_w,0);
		} else { // caso normal, tiende a lo que dijo calculate
			interpolate_good(d_x,+d_dx+x);
			interpolate_good(d_y,+y+d_dy);
			interpolate_good(d_fx,+d_dx+fx);
			interpolate_good(d_fy,+fy+d_dy);
			interpolate_good(d_h,h);
			interpolate(d_w,w);
		}
	}
	void DrawShapeSolid(const float *color,int x, int y, int w, int h) {
		glColor3fv(color);
		glBegin(GL_QUAD_STRIP);
		if (type==ET_PARA) {
			w=w/2; h=h/2; y-=h;
			for(int i=1;i<circle_steps;i+=2) { 
				glVertex2i(x,y);
				glVertex2i(x+cosx[i-1]*w,y+sinx[i-1]*h);
				glVertex2i(x+cosx[i+1]*w,y+sinx[i+1]*h);
				glVertex2i(x+cosx[i]*w,y+sinx[i]*h);
			}
		} else if (type==ET_PROCESO) {
			glVertex2i(x-w/2-5*h/8,y-5*h/8); glVertex2i(x-w/2-5*h/8,y-3*h/8);
			glVertex2i(x-w/2-h/2,y-1*h/8); glVertex2i(x-w/2-h/2,y-7*h/8);
			glVertex2i(x-w/2,y); glVertex2i(x-w/2,y-h);
			glVertex2i(x+w/2,y); glVertex2i(x+w/2,y-h); 
			glVertex2i(x+w/2+h/2,y-1*h/8);
			glVertex2i(x+w/2+h/2,y-7*h/8);
			glVertex2i(x+w/2+5*h/8,y-3*h/8);
			glVertex2i(x+w/2+5*h/8,y-5*h/8);
		} else if (type==ET_REPETIR||type==ET_MIENTRAS||type==ET_SI) {
			glVertex2i(x,y); glVertex2i(x+w/2,y-h/2);
			glVertex2i(x-w/2,y-h/2); glVertex2i(x,y-h);
		} else if (type==ET_ESCRIBIR||type==ET_LEER) {
			glVertex2i(x+w/2+margin,y); glVertex2i(x-w/2+margin,y); 
			glVertex2i(x+w/2-margin,y-h); glVertex2i(x-w/2-margin,y-h);
		} else if (type==ET_SEGUN) {
			glVertex2i(x,y); glVertex2i(x+w/2,y-h); glVertex2i(x-w/2,y-h); glVertex2i(x,y-h);
		} else {
			glVertex2i(x-w/2,y); glVertex2i(x+w/2,y);
			glVertex2i(x-w/2,y-h); glVertex2i(x+w/2,y-h);
		}
		glEnd();
	}
	void DrawShapeBorder(const float *color,int x, int y, int w, int h) {
		glColor3fv(color);
		glBegin(GL_LINE_LOOP);
		if (type==ET_PARA) {
			w=w/2; h=h/2; y-=h;
			for(int i=0;i<circle_steps;i++) { 
				glVertex2i(x+cosx[i]*w,y+sinx[i]*h);
				glVertex2i(x+cosx[i+1]*w,y+sinx[i+1]*h);
			}
		} else if (type==ET_PROCESO) {
			glVertex2i(x-w/2,y); glVertex2i(x+w/2,y);
			glVertex2i(x+w/2+h/2,y-1*h/8);
			glVertex2i(x+w/2+5*h/8,y-3*h/8);
			glVertex2i(x+w/2+5*h/8,y-5*h/8);
			glVertex2i(x+w/2+h/2,y-7*h/8);
			glVertex2i(x+w/2,y-h); glVertex2i(x-w/2,y-h);
			glVertex2i(x-w/2-h/2,y-7*h/8);
			glVertex2i(x-w/2-5*h/8,y-5*h/8);
			glVertex2i(x-w/2-5*h/8,y-3*h/8);
			glVertex2i(x-w/2-h/2,y-1*h/8);
		} else if (type==ET_REPETIR||type==ET_MIENTRAS||type==ET_SI) {
			glVertex2i(x,y); glVertex2i(x+w/2,y-h/2);
			glVertex2i(x,y-h); glVertex2i(x-w/2,y-h/2);
		} else if (type==ET_ESCRIBIR||type==ET_LEER) {
			glVertex2i(x-w/2+margin,y); glVertex2i(x+w/2+margin,y);
			glVertex2i(x+w/2-margin,y-h); glVertex2i(x-w/2-margin,y-h);
		} else if (type==ET_SEGUN) {
			glVertex2i(x,y); glVertex2i(x+w/2,y-h); glVertex2i(x-w/2,y-h);
		} else {
			glVertex2i(x-w/2,y); glVertex2i(x+w/2,y);
			glVertex2i(x+w/2,y-h); glVertex2i(x-w/2,y-h);
			if (edit_on && type==ET_OPCION && mouse!=this) {
				glVertex2i(x-w/2,y); glVertex2i(x-w/2+flecha_w,y); glVertex2i(x-w/2+flecha_w,y-h); glVertex2i(x-w/2,y-h);
			}
		}
		glEnd();
	}
	void DrawFlechaDown(int x, int y1, int y2) {
		glVertex2d(x,y1); glVertex2d(x,y2);
		glVertex2d(x-flecha_d,y2+flecha_d); glVertex2d(x,y2);
		glVertex2d(x+flecha_d,y2+flecha_d); glVertex2d(x,y2);
	}
	void DrawFlechaDownHead(int x, int y2) {
		glVertex2d(x-flecha_d,y2+flecha_d); glVertex2d(x,y2);
		glVertex2d(x+flecha_d,y2+flecha_d); glVertex2d(x,y2);
	}
	void DrawFlechaUp(int x, int y1, int y2) {
		glVertex2d(x,y1); glVertex2d(x,y2);
		glVertex2d(x-flecha_d,y2-flecha_d); glVertex2d(x,y2);
		glVertex2d(x+flecha_d,y2-flecha_d); glVertex2d(x,y2);
	}
	void DrawFlechaR(int x1, int x2, int y) {
		glVertex2d(x1,y); glVertex2d(x2,y);
		glVertex2d(x2-flecha_d,y-flecha_d); glVertex2d(x2,y);
		glVertex2d(x2-flecha_d,y+flecha_d); glVertex2d(x2,y);
	}
	void DrawFlechaL(int x1, int x2, int y) {
		glVertex2d(x1,y); glVertex2d(x2,y);
		glVertex2d(x2+flecha_d,y-flecha_d); glVertex2d(x2,y);
		glVertex2d(x2+flecha_d,y+flecha_d); glVertex2d(x2,y);
	}
	void Draw() {
		if (this==mouse && (prev||parent)) // si se esta moviendo con el mouse, dibujar un ghost donde lo agregariamos al soltar
			DrawShapeBorder(color_ghost,d_dx+x,d_dy+y,bwr+bwl,h);
		else if (type!=ET_OPCION && type!=ET_SEGUN) // sombra
			DrawShapeSolid(color_shadow,d_fx+shadow_delta_x,d_fy-shadow_delta_y,d_w,d_h);
		// flechas
		glBegin(GL_LINES); 
			glColor3fv(color_arrow);
			if (!nolink) {
				if (type==ET_OPCION) {
					glVertex2i((child[0]?child[0]->d_x:d_x),d_y-d_h); glVertex2i((child[0]?child[0]->d_x:d_x),d_y-d_h-flecha_h);
				} else if (type==ET_SEGUN) {
					for(int i=0;i<n_child;i++) {
						if (!child[i]->child[0]) {
							DrawFlechaDownHead(d_x+child_dx[i],d_y-d_h-child[i]->bh);
							DrawFlechaDown(d_x+child_dx[i],d_y-d_h-child_bh[i],d_y-d_bh+flecha_h); 
						}
						else if (child[i]!=mouse) {
							DrawFlechaDown(child[i]->child[0]->d_x,d_y-d_h-child_bh[i],d_y-d_bh+flecha_h); 
							DrawFlechaDownHead(child[i]->child[0]->d_x,child[i]->child[0]->d_y-child[i]->child[0]->d_bh); 
						}
					}
					// linea horizontal de abajo
					glVertex2d(d_x+child_dx[0],d_y-d_bh+flecha_h); glVertex2d(d_x+child_dx[n_child-1],d_y-d_bh+flecha_h);
				} else if (type==ET_MIENTRAS) {
					glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra
					glVertex2d(d_x,d_y-d_bh+3*flecha_h); glVertex2d(d_x,d_y-d_bh+2*flecha_h); // sale de la ultima instruccion
					DrawFlechaL(d_x,d_x-d_bwl,d_y-d_bh+2*flecha_h); // sigue a la izquierda
					DrawFlechaUp(d_x-d_bwl,d_y-d_bh+2*flecha_h,d_y); // sube
					DrawFlechaR(d_x-d_bwl,d_x,d_y); // entra arriba de la condicion
					glVertex2d(d_x,d_y-flecha_h-d_h/2); glVertex2d(d_x+d_bwr,d_y-flecha_h-d_h/2); // sale de la condicion para la derecha
					DrawFlechaDown(d_x+d_bwr,d_y-flecha_h-d_h/2,d_y-d_bh+flecha_h); // baja
					glVertex2d(d_x+d_bwr,d_y-d_bh+flecha_h); glVertex2d(d_x,d_y-d_bh+flecha_h); // va al punto de salida
					glVertex2i(d_fx,d_fy-d_h); glVertex2i(d_fx,d_fy-d_h-flecha_h);
				} else if (type==ET_PARA) {
					glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra del bloque
					glVertex2i(d_x,d_y-child_bh[0]-flecha_h); glVertex2i(d_x,d_y-d_bh+flecha_h); // flecha que sale del bloque
					glVertex2i(d_fx,d_y); glVertex2i(d_fx,d_fy); // flecha que sale del circulo
					DrawFlechaR(d_fx,d_x,d_y); 
					glVertex2i(d_fx,d_y); glVertex2i(d_fx,d_fy); // flecha que sale del circulo
					DrawFlechaUp(d_fx,d_y-d_bh+flecha_h,d_fy-d_h); // flecha que entra al circulo
					glVertex2i(d_fx,d_y-d_bh+flecha_h); glVertex2i(d_x,d_y-d_bh+flecha_h);
				} else if (type==ET_REPETIR) {
					glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra
					DrawFlechaL(d_x,d_x-d_bwl,d_fy-d_h/2); // sigue a la izquierda
					DrawFlechaUp(d_x-d_bwl,d_fy-d_h/2,d_y); // sube
					DrawFlechaR(d_x-d_bwl,d_x,d_y); // entra arriba de la condicion
					glVertex2i(d_fx,d_fy+flecha_h); glVertex2i(d_fx,d_fy); // flecha a la siguiente instruccion
				} else if (type==ET_SI) {
					// linea horizontal de arriba
					glVertex2d(d_x+child_dx[0],d_y-d_h/2); glVertex2d(d_x+child_dx[1],d_y-d_h/2);
					// flechas que bajan por el verdadero
					if (child[0]) { glVertex2i(d_x+child_dx[0],d_y-d_h/2); glVertex2i(d_x+child_dx[0],d_y-d_h-flecha_h); }
					else DrawFlechaDown(d_x+child_dx[0],d_y-d_h/2,d_y-d_h-flecha_h); 
					DrawFlechaDown(d_x+child_dx[0],d_y-d_h-child_bh[0]-flecha_h,d_y-d_bh+flecha_h); 
					// flechas que bajan por el falso
					if (child[1]) { glVertex2i(d_x+child_dx[1],d_y-d_h/2); glVertex2i(d_x+child_dx[1],d_y-d_h-flecha_h); }
					else DrawFlechaDown(d_x+child_dx[1],d_y-d_h/2,d_y-d_h-flecha_h); 
					DrawFlechaDown(d_x+child_dx[1],d_y-d_h-child_bh[1]-flecha_h,d_y-d_bh+flecha_h); 
					// linea horizontal de abajo
					glVertex2d(d_x+child_dx[0],d_y-d_bh+flecha_h); glVertex2d(d_x+child_dx[1],d_y-d_bh+flecha_h);
				}
				// punta de flecha que viene del anterior
				if (prev||parent) DrawFlechaDownHead(d_x,d_y-flecha_in); // no en inicio
				// linea de flecha que va al siguiente
				if ((next||parent)&&(type!=ET_OPCION)) { glVertex2i(d_x,d_y-d_bh); glVertex2i(d_x,d_y-d_bh+flecha_h); } // no en fin
			} else if (mouse==this && (next||parent)) {
				// flecha que va al siguiente item cuando este esta flotando
				glVertex2i(d_dx+x,d_dy+y-bh); glVertex2i(d_dx+x,d_dy+y-bh+flecha_h);
				if (type!=ET_OPCION) DrawFlechaDownHead(d_dx+x,d_dy+y); // no en inicio
			}
		glEnd();
		// relleno de la forma
		DrawShapeSolid(color_shape,d_fx,d_fy,d_w,d_h);
		// borde de la forma
		DrawShapeBorder(mouse==this?color_selection:color_border,d_fx,d_fy,d_w,d_h);
		
		if (type==ET_OPCION) { // + para agregar opciones
			if (edit_on && mouse!=this) {
				glBegin(GL_LINES);
				glColor3fv(color_label);
				glVertex2i(d_x-d_bwl+3*flecha_w/4,d_y-d_h/2); glVertex2i(d_x-d_bwl+1*flecha_w/4,d_y-d_h/2);
				glVertex2i(d_x-d_bwl+flecha_w/2,d_y-1*d_h/3); glVertex2i(d_x-d_bwl+flecha_w/2,d_y-2*d_h/3);
				glEnd();
			}
		} else 
		if (!nolink && (type==ET_ESCRIBIR||type==ET_LEER) ) { // flecha en la esquina
			glBegin(GL_LINES);
			glColor3fv(color_label);
			glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2+margin,d_y+margin);
			if (type==ET_LEER) {
				glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2-margin+margin,d_y-margin);
				glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2-margin,d_y-margin+margin);
			} else {
				glVertex2d(d_x+d_w/2+margin,d_y+margin); glVertex2d(d_x+d_w/2+margin-margin,d_y+margin);
				glVertex2d(d_x+d_w/2+margin,d_y+margin); glVertex2d(d_x+d_w/2+margin,d_y+margin-margin);
			}
			
			glEnd();
		}
		// texto;
		glColor3fv(edit==this?color_selection:(type==ET_PROCESO?color_arrow:color_label));
		glPushMatrix();
		glTranslated(d_fx-t_w/2+(edit_on&&type==ET_OPCION?flecha_w/2:0),d_fy-(type==ET_SEGUN?d_h-2*margin:d_h/2+margin),0);
		glScaled((.105*d_w)/w,(.15*d_h)/h,.1);
		for (unsigned int i=0;i<label.size();i++)
			dibujar_caracter(label[i]);
		glPopMatrix();
		if (edit==this && mouse!=this) {
			blink++; if (blink==20) blink=0;
			if (blink<10) {
				glBegin(GL_LINES);
				int lz=label.size(); if (!lz) lz=1;
				lz= d_fx-t_w/2+ t_w*edit_pos*d_w/lz/w;
				glVertex2i(lz,d_fy-h/2-t_h/2-margin/2);
				glVertex2i(lz,d_fy-h/2+t_h/2+margin/2);
				glEnd();
			}
		}
	}
	void Calculate(bool also_parent=false) { // devuelve el tamaño total del bloque
		int awl,awr,ah;
		Calculate(awl,awr,ah);
		if (also_parent && parent) parent->Calculate(true);
	}
	void MoveX(int dx) { // mueve al item y todos sus hijos en x
		x+=dx; fx+=dx;
		for (int i=0;i<n_child;i++)
			if (child[i]) child[i]->MoveX(dx);
		if (next) next->MoveX(dx);
	}
	void Calculate(int &gwl, int &gwr, int &gh) { // calcula lo propio y manda a calcular al siguiente y a sus hijos, y acumula en gw,gh el tamaño de este item (para armar el tamaño del bloque)
		fx=x; fy=y; bh=h+flecha_h; bwr=bwl=w/2; // esto es si fuera solo la forma
		// si son estructuras de control, es un viaje
		if (!nolink && n_child) {
			if (type==ET_OPCION) {
				bwr=bwl=(w=t_w+2*margin)/2;
				if (edit_on) 
					{ bwr+=flecha_w; bwl+=flecha_w; } // el + para agregar opciones
				child_dx[0]=0; child_bh[0]=0;
				if (child[0]) {
					child[0]->x=x; child[0]->y=y-bh;
					int cwl=0,cwr=0,ch=0;
					child[0]->Calculate(cwl,cwr,ch);
					bh+=ch; 
					if (cwl+cwr>bwl+bwr) bwl=bwr=(cwl+cwr)/2;
					bwl+=flecha_w/2;
					bwr+=flecha_w/2;
					child_dx[0]-=(cwr-cwl)/2;
					child[0]->MoveX(child_dx[0]);
				}
				// el ancho se lo define el segun padre
				w=bwl+bwr;
			} else if (type==ET_SEGUN) {
				bwr=bwl=(w=t_w*2)/2;
//				w=bwr=bwl=0; // todo: ver como corregir esto
				int sw=0, sh=0;
				for (int i=0;i<n_child;i++) {
					int cwl=0,cwr=0,ch=0;
					child[i]->x=0; child[i]->y=y-h;
					child[i]->Calculate(cwl,cwr,ch);
					child_bh[i]=ch; child_dx[i]=sw+cwl;
					sw+=cwl+cwr-2;
					if (ch>sh) sh=ch;
				}
				if (sw>w) w=sw; 
				bwr=bwl=w/2; 
				bh+=sh;
				for (int i=0;i<n_child;i++) {
					child_dx[i]-=w/2;
					child[i]->MoveX(child_dx[i]);
				}
				Entity *dom=child[n_child-1];
				if (dom->x+dom->bwr<x+bwr) {
					int dif=(x+bwr)-(dom->x+dom->bwr);
					dom->bwl+= dif/2;
					dom->bwr+= dif/2;
					dom->w+= dif;
					dom->MoveX(dif/2);
					child_dx[n_child-1]+=dif/2;
				}
			} if (type==ET_SI) {
				int c1l=0,c1r=0,c1h=0;
				if (child[0]) {
					child[0]->y=y-bh; child[0]->x=x;
					child[0]->Calculate(c1l,c1r,c1h);
				} 
				int c2l=0,c2r=0,c2h=0;
				if (child[1]) {
					child[1]->y=y-bh; child[1]->x=x;
					child[1]->Calculate(c2l,c2r,c2h);
				} 
				bh += (c1h>c2h?c1h:c2h) + flecha_h;
				child_bh[0]=c1h; child_bh[1]=c2h;
				child_dx[0] = -flecha_w-(c1r>bwl?c1r:bwl);
				child_dx[1] =  flecha_w+(c2l>bwr?c2l:bwr);
				if (child[0]) child[0]->MoveX(child_dx[0]);
				if (child[1]) child[1]->MoveX(child_dx[1]);
				if (c1l-child_dx[0]>bwl) bwl=c1l-child_dx[0];
				if (c2r+child_dx[1]>bwr) bwr=c2r+child_dx[1];
			} else if (type==ET_MIENTRAS||type==ET_REPETIR) {
				int c1l=0,c1r=0,c1h=0;
				if (child[0]) {
					child[0]->y=y-(type==ET_MIENTRAS?bh+flecha_h:flecha_h); child[0]->x=x;
					child[0]->Calculate(c1l,c1r,c1h);
				} 
				child_dx[0]=0; child_bh[0]=c1h;
				if (c1l>bwl) bwl=c1l;
				if (c1r>bwr) bwr=c1r;
				bwr+=flecha_w; bwl+=flecha_w; bh+=c1h;
				fy-= (type==ET_MIENTRAS)?flecha_h:c1h+flecha_h;
				bh+= (type==ET_MIENTRAS)?flecha_h*3:flecha_h;
			} else if (type==ET_PARA) {
				bwr=0; bwl=w;
				int c1l=0,c1r=0,c1h=0;
				if (child[0]) {
					child[0]->y=y-flecha_h; child[0]->x=x;
					child[0]->Calculate(c1l,c1r,c1h);
				} 
				child_dx[0]=0; child_bh[0]=c1h;
				if (c1r>bwr) bwr=c1r;
				bwl=bwl+c1l+flecha_w;
				if (c1h>bh) bh=c1h;
				bh+=2*flecha_h;
				fx=-c1l-w/2-flecha_w;
				fy=y+(flecha_h-bh+h)/2;
			}
		}
		// pasar a la siguiente entidad
		if (next) {
			next->x=x;
			next->y=y-bh;
			next->Calculate(gwl,gwr,gh);
		}
		// actualizar el bb global del bloque
		gh+=bh;	
		if (gwl<bwl) gwl=bwl;
		if (gwr<bwr) gwr=bwr;
	}
	void CopyPos(Entity *o) {
		x=o->x; y=o->y;
		fx=o->fx; fy=o->fy;
		d_x=o->d_x; d_y=o->d_y;
		d_fx=o->d_fx; d_fy=o->d_fy;
	}
	bool CheckMouse(int x, int y) {
		if (!edit_on) return false;
		if (type==ET_OPCION) {
			if (x>=d_fx-d_w/2 && x<=d_fx-d_w/2+flecha_w && y<=d_fy && y>=d_fy-d_h) { // agregar una opción más
				parent->InsertChild(child_id,new Entity(ET_OPCION,""));
				parent->child[child_id-1]->SetEdit();
				parent->child[child_id-1]->CopyPos(this);
				return false;
			}
			if (child_id==parent->n_child-1) return false;
		}
		if (x>=d_fx-d_w/2 && x<=d_fx+d_w/2 && y<=d_fy && y>=d_fy-d_h) {
			m_x=x-d_fx;
			m_y=y-d_fy;
			return true;
		}
		return false;
	}
	void Print(ostream &out, string tab="") {
		bool add_tab=false;
		if (type==ET_PROCESO) {
			add_tab=true;
			if (next) {
				out<<tab<<"Proceso SinTitulo"<<endl;
				if (next) next->Print(out,add_tab?tab+"   ":tab);
				out<<tab<<"FinProceso"<<endl;
				return;
			}
		} else if (type==ET_ESCRIBIR) {
			out<<tab<<"Escribir "<<label<<";"<<endl;
		} else if (type==ET_LEER) {
			out<<tab<<"Leer "<<label<<";"<<endl;
		} else if (type==ET_MIENTRAS) {
			out<<tab<<"Mientras "<<label<<" Hacer"<<endl;
			if (child[0]) child[0]->Print(out,tab+"   ");
			out<<tab<<"FinMientras"<<endl;
		} else if (type==ET_REPETIR) {
			out<<tab<<"Repetir"<<endl;
			if (child[0]) child[0]->Print(out,tab+"   ");
			out<<tab<<"Hasta Que "<<label<<endl;
		} else if (type==ET_PARA) {
			out<<tab<<"Para "<<label<<" Hacer"<<endl;
			if (child[0]) child[0]->Print(out,tab+"   ");
			out<<tab<<"FinPara"<<endl;
		} else if (type==ET_SEGUN) {
			out<<tab<<"Segun "<<label<<" Hacer"<<endl;
			for(int i=0;i<n_child-1;i++) { 
				child[i]->Print(out,tab+"   ");
			}
			if (child[n_child-1]->child[0]) // de otro modo
				child[n_child-1]->Print(out,tab+"   ");
			out<<tab<<"FinSegun"<<endl;
		} else if (type==ET_OPCION) {
			add_tab=true;
			out<<tab<<label<<":"<<endl;
			if (child[0]) child[0]->Print(out,tab+"   ");
		} else if (type==ET_SI) {
			out<<tab<<"Si "<<label<<" Entonces"<<endl;
			if (child[0]) child[0]->Print(out,tab+"   ");
			if (child[1]) out<<tab<<"Sino"<<endl;
			if (child[1]) child[1]->Print(out,tab+"   ");
			out<<tab<<"FinSi"<<endl;
		} else if (type==ET_ASIGNAR) {
			out<<tab<<label<<";";
		}
		if (next) next->Print(out,add_tab?tab+"   ":tab);
	}
};
Entity *Entity::all_any=NULL;

bool StartsWith(const string &s1, string s2) {
	if (s1.length()<s2.length()) return false;
	else return s1.substr(0,s2.length())==s2;
}

Entity *Add(stack<int> &ids, Entity *vieja, Entity *nueva, int id=-1) {
	int mid=ids.top(); 
	ids.pop(); 
	ids.push(-1);
	if (mid==-1) 
		vieja->LinkNext(nueva);
	else 
		vieja->LinkChild(mid,nueva);
	if (id!=-1) 
		ids.push(id);
	return nueva;
}

void Load(const char *fname) {
	ifstream file(fname);
	string str;
	start = new Entity(ET_PROCESO,"Inicio");
	Entity *aux=start;
	stack<int> ids; ids.push(-1);
	while (getline(file,str)) {
		if (!str.size()||StartsWith(str,"PROCESO ")||str=="FINPROCESO"||str=="ENTONCES") {
			continue;
		}
		else if (StartsWith(str,"ESCRIBIR ")) {
			aux=Add(ids,aux,new Entity(ET_ESCRIBIR,str.substr(9)));
		}
		else if (StartsWith(str,"LEER ")) {
			aux=Add(ids,aux,new Entity(ET_LEER,str.substr(5)));
		}
		else if (StartsWith(str,"MIENTRAS ")) {
			aux=Add(ids,aux,new Entity(ET_MIENTRAS,str.substr(9)),0);
		}
		else if (str=="REPETIR") {
			aux=Add(ids,aux,new Entity(ET_REPETIR,""),0);
		}
		else if (StartsWith(str,"PARA ")) {
			aux=Add(ids,aux,new Entity(ET_PARA,str.substr(5)),0);
		}
		else if (StartsWith(str,"SI ")) {
			aux=Add(ids,aux,new Entity(ET_SI,str.substr(3)),0);
		}
		else if (StartsWith(str,"SINO")) {
			aux=aux->parent; ids.pop(); ids.push(1);
		}
		else if (StartsWith(str,"SEGUN ")) {
			aux=Add(ids,aux,new Entity(ET_SEGUN,str.substr(6)),0);
		}
		else if (StartsWith(str,"OPCION ")) {
			if (aux->type==ET_SEGUN) { // si esta despues del segun, es el primer hijo
				aux=Add(ids,aux,new Entity(ET_OPCION,str.substr(7)),0);
			} else if (aux->type==ET_OPCION) {  // si esta despues de una opcion vacia, subir al segun
				aux=aux->parent; ids.pop();
				aux=Add(ids,aux,new Entity(ET_OPCION,str.substr(7)),aux->n_child);
			} else if (aux->type==ET_OPCION) { // si esta despues de una instruccion comun, subir a la opcion, subir al segun
				aux=aux->parent; ids.pop();
				aux=aux->parent; ids.pop();
				aux=Add(ids,aux,new Entity(ET_OPCION,str.substr(7)),aux->n_child);
			}
		}
		else if (StartsWith(str,"HASTA QUE ")) {
			aux=aux->parent; ids.pop(); aux->SetLabel(str.substr(10),false);
		}
		else if (str=="FINPARA"||str=="FINSI"||str=="FINMIENTRAS"||str=="FINSEGUN") {
			aux=aux->parent; ids.pop();
		}
		else { // asignacion, dimension, definicion
			aux=Add(ids,aux,new Entity(ET_ASIGNAR,str));
		}
	}
	aux->LinkNext(new Entity(ET_PROCESO,"FinProceso"));
	start->Calculate();
	d_dx=win_w/2;
	d_dy=win_h-50;
}

void ProcessMenu(int op);

void Load() {
	d_dx=300; d_dy=680;
	
	start = new Entity(ET_PROCESO,"Inicio");
	
//	Entity *aux1 = new Entity(ET_SEGUN,"VARIABLE DE DESICION"); start->LinkNext(aux1);
//	Entity *aux1a = new Entity(ET_OPCION,"1"); aux1->InsertChild(0,aux1a);
//	Entity *aux1aa = new Entity(ET_ESCRIBIR,"1"); aux1a->LinkChild(0,aux1aa);
//	Entity *aux1b = new Entity(ET_OPCION,"2"); aux1->InsertChild(1,aux1b);
//	Entity *aux1ba = new Entity(ET_ESCRIBIR,"1"); aux1b->LinkChild(0,aux1ba);
//	Entity *aux1c = new Entity(ET_OPCION,"3"); aux1->InsertChild(2,aux1c);
////	Entity *aux1ca = new Entity(ET_PARA,"1aslkfj asklhdskhfakjfhdskjfhdslkjhldsfa"); aux1c->LinkChild(0,aux1ca);
//	Entity *aux2 = new Entity(ET_PROCESO,"Fin"); aux1->LinkNext(aux2);
	
	Entity *aux1 = new Entity(ET_LEER,"A,B"); start->LinkNext(aux1);
	Entity *aux2 = new Entity(ET_ASIGNAR,"C<-RC(A^2+B^2)"); aux1->LinkNext(aux2);
	Entity *aux3 = new Entity(ET_ESCRIBIR,"'Hipotenusa=',C"); aux2->LinkNext(aux3);
	Entity *aux4 = new Entity(ET_SI,"SI"); aux3->LinkNext(aux4);
	Entity *aux4a = new Entity(ET_ESCRIBIR,"LALA LALA"); aux4->LinkChild(0,aux4a);
	Entity *aux4b = new Entity(ET_ESCRIBIR,"BOOGA"); aux4a->LinkNext(aux4b);
	Entity *aux4c = new Entity(ET_ESCRIBIR,"FOO FOO FOO"); aux4->LinkChild(1,aux4c);
	Entity *aux5 = new Entity(ET_PARA,"PARA"); aux4->LinkNext(aux5);
	Entity *aux5a = new Entity(ET_ESCRIBIR,"HOLA"); aux5->LinkChild(0,aux5a);
	Entity *aux6 = new Entity(ET_MIENTRAS,"MIENTRAS"); aux5->LinkNext(aux6);
	Entity *aux6a = new Entity(ET_ESCRIBIR,"HOLA"); aux6->LinkChild(0,aux6a);
	Entity *aux7 = new Entity(ET_REPETIR,"REPETIR"); aux6->LinkNext(aux7);
	Entity *aux7a = new Entity(ET_ESCRIBIR,"HOLA"); aux7->LinkChild(0,aux7a);
	Entity *aux8 = new Entity(ET_PROCESO,"Fin"); aux7->LinkNext(aux8);
	start->Calculate();
	ProcessMenu(1);
}

void reshape_cb (int w, int h) {
	if (w==0||h==0) return;
	win_h=h; win_w=w;
	glViewport(0,0,w,h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(0,w,0,h);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

void DrawText(const float *color, int x, int y, const char *t) {
	glColor3fv(color);
	glPushMatrix();
	glTranslated(x,y,0);
	glScaled(.1,.15,.1);
	while (*t) 
		dibujar_caracter(*(t++));
	glPopMatrix();
}

void DrawMenus() {
	glColor3fv(color_menu_back);
	glBegin(GL_QUADS);
		// shapebar
		glVertex2i(win_w-shapebar_size,0);
		glVertex2i(win_w-shapebar_size,win_h);
		glVertex2i(win_w,win_h);
		glVertex2i(win_w,0);
		// trash
		glVertex2i(0,trash_size);
		glVertex2i(trash_size,trash_size);
		glVertex2i(trash_size,0);
		glVertex2i(0,0);
		// menu
		glVertex2i(0,win_h);
		glVertex2i(menu_size_w,win_h);
		glVertex2i(menu_size_w,win_h-menu_size_h);
		glVertex2i(0,win_h-menu_size_h);
	glEnd();
	glColor3fv(color_menu);
	glBegin(GL_LINES);
		// trash
		glVertex2i(0,trash_size); glVertex2i(trash_size,trash_size);
		glVertex2i(trash_size,0); glVertex2i(trash_size,trash_size);
		if (trash) {
			glColor3fv(color_selection);
			int tm=trash_size/6;
			// borde
			glVertex2i(2*tm,tm); glVertex2i(trash_size-tm*2,tm);
			glVertex2i(2*tm,tm); glVertex2i(tm,trash_size-tm); 
			glVertex2i(trash_size-tm*2,tm);	glVertex2i(trash_size-tm,trash_size-tm);
			glVertex2i(tm,trash_size-tm); glVertex2i(trash_size-tm,trash_size-tm);
			// tramado
			glVertex2i(tm+2*tm  ,  tm); glVertex2i(4*tm+2*tm/4,trash_size-3*tm);
			glVertex2i(tm+  tm  ,  tm); glVertex2i(trash_size-tm,trash_size-tm);
			glVertex2i(tm+3*tm/4,2*tm); glVertex2i(trash_size-2*tm,trash_size-tm);
			glVertex2i(tm+2*tm/4,3*tm); glVertex2i(trash_size-3*tm,trash_size-tm);
			glVertex2i(tm+1*tm/4,4*tm); glVertex2i(trash_size-4*tm,trash_size-tm);
			glVertex2i(trash_size-tm-2*tm  ,  tm); glVertex2i(tm+2*tm/4,trash_size-3*tm);
			glVertex2i(trash_size-tm-  tm  ,  tm); glVertex2i(tm,trash_size-tm);
			glVertex2i(trash_size-tm-3*tm/4,2*tm); glVertex2i(2*tm,trash_size-tm);
			glVertex2i(trash_size-tm-2*tm/4,3*tm); glVertex2i(3*tm,trash_size-tm);
			glVertex2i(trash_size-tm-1*tm/4,4*tm); glVertex2i(4*tm,trash_size-tm);
			glColor3fv(color_menu);
		} else {
			int t6=trash_size/5;
			glVertex2i(t6,t6); glVertex2i(4*t6,4*t6);
			glVertex2i(t6,4*t6); glVertex2i(4*t6,t6);
		}
		// menu
		glVertex2i(0,win_h-menu_size_h); glVertex2i(menu_size_w,win_h-menu_size_h);
		glVertex2i(menu_size_w,win_h); glVertex2i(menu_size_w,win_h-menu_size_h);
		// shapebar
		glVertex2i(win_w-shapebar_size,0);
		glVertex2i(win_w-shapebar_size,win_h);
		if (shapebar) {
			int y=win_h, x=win_w-shapebar_size, sh=win_h/8;
			int x0,x1,y0,y1,delta,xm,ym;
			x0=x+shapebar_size/5; x1=x+shapebar_size-shapebar_size/5;
			y0=y-3*sh/10; y1=y-7*sh/10; xm=(x0+x1)/2; ym=(y0+y1)/2;
			// asignar
			if (shapebar_sel==1) glColor3fv(color_selection);
			glVertex2i(x0,y0); glVertex2i(x0,y1);
			glVertex2i(x1,y1); glVertex2i(x0,y1);
			glVertex2i(x1,y1); glVertex2i(x1,y0);
			glVertex2i(x0,y0); glVertex2i(x1,y0);
			glColor3fv(color_menu); 
			// leer
			y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
			if (shapebar_sel==2) glColor3fv(color_selection);
			delta=shapebar_size/16;
			glVertex2i(x0+delta,y0); glVertex2i(x0-delta,y1);
			glVertex2i(x1-delta,y1); glVertex2i(x0-delta,y1);
			glVertex2i(x1-delta,y1); glVertex2i(x1+delta,y0);
			glVertex2i(x0+delta,y0); glVertex2i(x1+delta,y0);
			glVertex2i(x1-2*delta,y0-sh/10); glVertex2i(x1-2*delta,y0+sh/5);
			glVertex2i(x1-2*delta,y0+sh/5); glVertex2i(x1-1*delta,y0+sh/10);
			glVertex2i(x1-2*delta,y0+sh/5); glVertex2i(x1-3*delta,y0+sh/10);
			glColor3fv(color_menu); 
			// escribir
			y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
			if (shapebar_sel==3) glColor3fv(color_selection);
			y0=y-3*sh/10; y1=y-7*sh/10;
			glVertex2i(x0+delta,y0); glVertex2i(x0-delta,y1);
			glVertex2i(x1-delta,y1); glVertex2i(x0-delta,y1);
			glVertex2i(x1-delta,y1); glVertex2i(x1+delta,y0);
			glVertex2i(x0+delta,y0); glVertex2i(x1+delta,y0);
			glVertex2i(x1-2*delta,y0-sh/5); glVertex2i(x1-2*delta,y0+sh/10);
			glVertex2i(x1-2*delta,y0-sh/5); glVertex2i(x1-3*delta,y0-sh/10);
			glVertex2i(x1-2*delta,y0-sh/5); glVertex2i(x1-1*delta,y0-sh/10);
			glColor3fv(color_menu); 
			// si
			y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
			if (shapebar_sel==4) glColor3fv(color_selection);
			glVertex2i(xm,y0+delta); glVertex2i(xm,y0); // entra
			glVertex2i(xm,y0); glVertex2i(x0+delta,ym);
			glVertex2i(xm,y1); glVertex2i(x0+delta,ym);
			glVertex2i(xm,y1); glVertex2i(x1-delta,ym);
			glVertex2i(xm,y0); glVertex2i(x1-delta,ym);
			glVertex2i(x0+delta,ym); glVertex2i(x0-delta,ym); // izquierda
			glVertex2i(x0-delta,ym); glVertex2i(x0-delta,y1-delta);
			glVertex2i(x1-delta,ym); glVertex2i(x1+delta,ym); // derecha
			glVertex2i(x1+delta,ym); glVertex2i(x1+delta,y1-delta);
			glColor3fv(color_menu); 
			// segun
			if (shapebar_sel==5) glColor3fv(color_selection);
			y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
			glVertex2i(xm,y0+delta); glVertex2i(xm,y0); // entra
			glVertex2i(xm,y0); glVertex2i(x0,y1); // rombo
			glVertex2i(xm,y0); glVertex2i(x1,y1);
			glVertex2i(x0,y1); glVertex2i(x1,y1);
			glVertex2i(x0+delta,y1); glVertex2i(x0+delta,y1-delta); // salidas
			glVertex2i(x1-delta,y1); glVertex2i(x1-delta,y1-delta);
			glVertex2i(x0+4*delta,y1); glVertex2i(x0+4*delta,y1-delta);
			glVertex2i(x1-4*delta,y1); glVertex2i(x1-4*delta,y1-delta);
			glColor3fv(color_menu); 
			// mientras
			y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
			if (shapebar_sel==6) glColor3fv(color_selection);
			glVertex2i(xm,y0+2*delta); glVertex2i(xm,y0); // entra
			glVertex2i(xm,y0); glVertex2i(x0+delta,ym+delta); // rombo
			glVertex2i(xm,y1+2*delta); glVertex2i(x0+delta,ym+delta);
			glVertex2i(xm,y1+2*delta); glVertex2i(x1-delta,ym+delta);
			glVertex2i(xm,y0); glVertex2i(x1-delta,ym+delta);
			glVertex2i(xm,y1+2*delta); glVertex2i(xm,y1); // verdadero
			glVertex2i(xm,y1); glVertex2i(x0,y1); 
			glVertex2i(x0,y1); glVertex2i(x0,y0+delta); 
			glVertex2i(xm,y0+delta); glVertex2i(x0,y0+delta); 
			glVertex2i(x1-delta,ym+delta); glVertex2i(x1,ym+delta); // falso
			glVertex2i(x1,ym+delta); glVertex2i(x1,y1-delta);
			glVertex2i(xm,y1-delta); glVertex2i(x1,y1-delta);
			glVertex2i(xm,y1-delta); glVertex2i(xm,y1-2*delta); 
			glColor3fv(color_menu); 
			// repetir
			y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
			if (shapebar_sel==7) glColor3fv(color_selection);
			glVertex2i(xm,y0+2*delta); glVertex2i(xm,y0-2*delta); // entra
			glVertex2i(xm,y0-2*delta); glVertex2i(x0+delta,ym-delta); // rombo
			glVertex2i(xm,y1); glVertex2i(x0+delta,ym-delta);
			glVertex2i(xm,y1); glVertex2i(x1-delta,ym-delta);
			glVertex2i(xm,y0-2*delta); glVertex2i(x1-delta,ym-delta);
			glVertex2i(xm,y1); glVertex2i(xm,y1-delta); // verdadero
			glVertex2i(x0+delta,ym-delta); glVertex2i(x0-delta,ym-delta); // falso
			glVertex2i(x0-delta,ym-delta); glVertex2i(x0-delta,y0);
			glVertex2i(xm,y0); glVertex2i(x0-delta,y0); 
			glColor3fv(color_menu); 
			// para
			y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
			if (shapebar_sel==8) glColor3fv(color_selection);
			glVertex2i(xm-  delta,ym-  delta); glVertex2i(xm-  delta,ym+  delta); // forma
			glVertex2i(xm-  delta,ym+  delta); glVertex2i(xm-2*delta,ym+2*delta);
			glVertex2i(xm-4*delta,ym+2*delta); glVertex2i(xm-2*delta,ym+2*delta);
			glVertex2i(xm-5*delta,ym+  delta); glVertex2i(xm-4*delta,ym+2*delta);
			glVertex2i(xm-5*delta,ym+  delta); glVertex2i(xm-5*delta,ym-  delta);
			glVertex2i(xm-4*delta,ym-2*delta); glVertex2i(xm-5*delta,ym-  delta);
			glVertex2i(xm-4*delta,ym-2*delta); glVertex2i(xm-2*delta,ym-2*delta);
			glVertex2i(xm-  delta,ym-  delta); glVertex2i(xm-2*delta,ym-2*delta);
			glVertex2i(xm-3*delta,ym+2*delta); glVertex2i(xm-3*delta,y0+  delta); // flecha arriba
			glVertex2i(xm-3*delta,y0  +delta); glVertex2i(xm+2*delta,y0+  delta); 
			glVertex2i(xm-3*delta,ym-2*delta); glVertex2i(xm-3*delta,y1-  delta); // flecha abajo
			glVertex2i(xm-3*delta,y1-  delta); glVertex2i(xm+2*delta,y1-  delta); 
			glVertex2i(xm+2*delta,y1-2*delta); glVertex2i(xm+2*delta,y0+2*delta); // line vertical
			glColor3fv(color_menu); 
		} else { 
			// en la shapebar cerrada se dibuja una flecha para abrirla
			glVertex2i(win_w-2*shapebar_size/3,win_h/2); glVertex2i(win_w-shapebar_size/3,win_h/2-2*shapebar_size/3);
			glVertex2i(win_w-2*shapebar_size/3,win_h/2); glVertex2i(win_w-shapebar_size/3,win_h/2+2*shapebar_size/3);
		}
	glEnd();
	// menu
	DrawText(menu_sel==1?color_selection:color_menu,menu_size_w-240,win_h-menu_size_h+260,"Auto-ajustar Zoom");
	DrawText(menu_sel==2?color_selection:color_menu,menu_size_w-240,win_h-menu_size_h+220,"Guardar Cambios");
	DrawText(menu_sel==3?color_selection:color_menu,menu_size_w-240,win_h-menu_size_h+180,"Guardar y Ejecutar");
	DrawText(menu_sel==4?color_selection:color_menu,menu_size_w-240,win_h-menu_size_h+140,"Guardar y Cerrar");
	DrawText(menu_sel==5?color_selection:color_menu,menu_size_w-240,win_h-menu_size_h+100,"Cerrar Sin Guardar");
	DrawText(menu_sel==6?color_selection:color_menu,menu_size_w-240,win_h-menu_size_h+60,"Ayuda...");
	DrawText(color_menu,menu_size_w-70,win_h-menu_size_h+10,"Menu");
	// shapebar
	if (shapebar) {
		if (shapebar_sel==1) DrawText(color_selection,10,10,"Asiganacion/Dimension/Definicion");
		if (shapebar_sel==2) DrawText(color_selection,10,10,"Escribir");
		if (shapebar_sel==3) DrawText(color_selection,10,10,"Leer");
		if (shapebar_sel==4) DrawText(color_selection,10,10,"Si-Entonces");
		if (shapebar_sel==5) DrawText(color_selection,10,10,"Segun");
		if (shapebar_sel==6) DrawText(color_selection,10,10,"Mientras");
		if (shapebar_sel==7) DrawText(color_selection,10,10,"Repetir-Hasta que");
		if (shapebar_sel==8) DrawText(color_selection,10,10,"Para");
	} else if (trash) DrawText(color_selection,10+trash_size_max,10,"Eliminar");
}

void display_cb() {
	if (entity_to_del) delete entity_to_del;
	glClear(GL_COLOR_BUFFER_BIT);
	// dibujar el diagrama
	Entity *aux=start;
	bool found=false;
	glLineWidth(zoom*2);
	glPushMatrix();
	glScalef(d_zoom,d_zoom,1);
	do {
		if (!found && mouse && mouse->type!=ET_OPCION) {
			if (aux!=mouse && aux->CheckLinkNext(cur_x,cur_y)) {
				mouse->UnLink();
				aux->LinkNext(mouse);
				start->Calculate();
				found=true;
			} else if (aux->child) {
				int i=aux->CheckLinkChild(cur_x,cur_y);
				if (i!=-1) {
					mouse->UnLink();
					aux->LinkChild(i,mouse);
					start->Calculate();
					found=true;
				}
			}
		}
		aux->Draw();
		aux=aux->all_next;
	} while (aux!=start);
	if (mouse && mouse->type==ET_OPCION) {
		int i=mouse->parent->CheckLinkOpcion(cur_x,cur_y);
		if (i!=-1) {
			mouse->parent->MoveChild(mouse->child_id,i);
			mouse->parent->Calculate();
		}
	}
	if (selecting_zoom) {
		glColor3fv(color_menu);
		glBegin(GL_LINE_LOOP);
		glVertex2i(m_x0,m_y0);
		glVertex2i(m_x0,cur_y);
		glVertex2i(cur_x,cur_y);
		glVertex2i(cur_x,m_y0);
		glEnd();
	}
	glPopMatrix();
	// dibujar menues y demases
	glLineWidth(2);
	if (edit_on) DrawMenus();
	// dibujar la seleccion para que quede delante de todo
	if (mouse) {
		glLineWidth(zoom*2);
		glPushMatrix();
		glScalef(d_zoom,d_zoom,1);
		mouse->Draw();
		glPopMatrix();
	}
	glutSwapBuffers();
}

void Raise() {
	glutHideWindow();
	glutShowWindow();
}

void idle_func() {
	static char *rec=new char[256];
	int cant=256;
	if (zocket!=ZOCKET_ERROR && zocket_leer(zocket,rec,cant)) {
		rec[cant]=0;
		if (string(rec)=="edit") { edit_on=true; Raise(); }
		else if (string(rec)=="noedit") { edit_on=false; Raise(); }
		else if (string(rec)=="raise") Raise();
	}
	usleep(25000);
	Entity *aux=start;
	do {
		aux->Tick();
		aux=aux->all_next;
	} while (aux!=start);
	if (mouse) { 
		interpolate(shapebar_size,0);
		interpolate(menu_size_h,0);
		interpolate(menu_size_w,0);
		if (trash) interpolate(trash_size,trash_size_max);
		else interpolate(trash_size,trash_size_min);
		menu=shapebar=false;
	} else {
		if (shapebar) interpolate(shapebar_size,shapebar_size_max);
		else interpolate(shapebar_size,shapebar_size_min);
		if (menu) { interpolate(menu_size_h,290); interpolate(menu_size_w,260); }
		else { interpolate(menu_size_h,30); interpolate(menu_size_w,80); }
		interpolate(trash_size,0); trash=false;
	}
	interpolate(d_zoom,zoom);
	display_cb();
}

void passive_motion_cb(int x, int y) {
	if (!win_h || !edit_on) return;
	if (mouse) {
		shapebar=false; menu=false;
		return;
	}
	menu=x<menu_size_w&&y<menu_size_h;
	shapebar=x>win_w-shapebar_size;
	if (menu) {
		y=win_h-y;
		if (y>win_h-menu_size_h+245) menu_sel=1;
		else if (y>win_h-menu_size_h+205) menu_sel=2;
		else if (y>win_h-menu_size_h+165) menu_sel=3;
		else if (y>win_h-menu_size_h+125) menu_sel=4;
		else if (y>win_h-menu_size_h+85) menu_sel=5;
		else if (y>win_h-menu_size_h+45) menu_sel=6;
		else menu_sel=0;
	} else if (shapebar) {
		shapebar_sel=y/(win_h/8)+1;
		if (y==9) y=0;
	}
}
void motion_cb(int x, int y) {
	y=win_h-y; 
	trash=x<trash_size && y<trash_size;
	y/=zoom; x/=zoom;
	if (selecting_zoom) {
		cur_x=x; cur_y=y;
		return;
	}
	if (panning) { 
		d_dx+=x-m_x0; m_x0=x;
		d_dy+=y-m_y0; m_y0=y;
	} if (mouse) { 
		cur_y=y; cur_x=mouse->d_x;
		mouse->d_x=x-mouse->m_x;
		mouse->d_y=y-mouse->m_y;
		mouse->d_fx=x-mouse->m_x;
		mouse->d_fy=y-mouse->m_y;
	}
	if (trash && mouse) {
		if (mouse->type!=ET_OPCION && (mouse->parent||mouse->prev)) {
			mouse->UnLink();
			start->Calculate();
		}
	}
}

void ZoomExtend(int x0, int y0, int x1, int y1) {
	if (x1<x0) { int aux=x1; x1=x0; x0=aux; }
	if (y0<y1) { int aux=y1; y1=y0; y0=aux; }
	if (x1-x0<10||y0-y1<10) return;
	int h=y0-y1, w=x1-x0;
	double zh=float(win_h-2*flecha_w)/h; // zoom para ajustar alto
	double zw=float(win_w-shapebar_size_min-2*flecha_w)/w; // zoom para ajustar ancho
	if (zw>zh) zoom=zh; else zoom=zw; // ver cual tamaño manda
	d_dx=win_w/zoom/2-(x1+x0)/2;
	d_dy=win_h/zoom/2-(y1+y0)/2/*+h/2/zoom*/;
}

bool SendUpdate(bool run=false);
bool SendHelp();
bool SendRun();

void ProcessMenu(int op) {
	if (op==1) {
		int h=0,wl=0,wr=0;
		start->Calculate(wl,wr,h); // calcular tamaño total
		ZoomExtend(start->x-wl,start->y,start->x+wr,start->y-h);
	} else if (op==2) {
		SendUpdate();
	} else if (op==3) {
		SendUpdate(true);
	} else if (op==4) {
		SendUpdate(); Salir();
	} else if (op==5) {
		Salir();
	} else if (op==6) {
		SendHelp();
	}
}

Entity *DuplicateEntity(Entity *orig) {
	Entity *nueva=new Entity(orig->type,orig->label);
	nueva->m_x=orig->m_x; nueva->m_y=orig->m_y;
	nueva->x=orig->x; nueva->y=orig->y;
	nueva->fx=orig->fx; nueva->fy=orig->fy;
	nueva->d_x=orig->d_x; nueva->d_y=orig->d_y;
	nueva->d_fx=orig->d_fx; nueva->d_fy=orig->d_fy;
	for(int i=0;i<orig->n_child;i++) { 
		if (orig->child[i]) nueva->LinkChild(i,DuplicateEntity(orig->child[i]));
		else nueva->LinkChild(i,NULL);
	}
	return nueva;
}

void mouse_cb(int button, int state, int x, int y) {
	y=win_h-y; y/=zoom; x/=zoom;
	if (button==4||button==3) {
		double f=button==4?1.05:1.0/1.05;
		zoom*=f;
		d_dx-=x*f-x;
		d_dy-=y*f-y;
	} else if (state==GLUT_DOWN) {
		if (button==GLUT_MIDDLE_BUTTON) { // click en el menu
			cur_x=m_x0=x; cur_y=m_y0=y; selecting_zoom=true;
			return;
		}
		if (menu && button==GLUT_LEFT_BUTTON) { // click en el menu
			if (menu_sel) ProcessMenu(menu_sel);
			return;
		}
		if (shapebar && button==GLUT_LEFT_BUTTON) { // click en la barra de entidades
			if (!shapebar_sel) return;
			shapebar=false;
			Entity*aux=NULL;
			switch (shapebar_sel) {
				case 1: aux = new Entity(ET_ASIGNAR,""); break;
				case 2: aux = new Entity(ET_ESCRIBIR,""); break;
				case 3: aux = new Entity(ET_LEER,""); break;
				case 4: aux = new Entity(ET_SI,""); break;
				case 5: aux = new Entity(ET_SEGUN,""); break;
				case 6: aux = new Entity(ET_MIENTRAS,""); break;
				case 7: aux = new Entity(ET_REPETIR,""); break;
				case 8: aux = new Entity(ET_PARA,""); break;
			}
			if (!aux) return;
			aux->m_x=0; aux->m_y=0;
			aux->SetMouse();
			aux->SetEdit();
			return;
		}
		// click en una entidad? izquierdo=mover, derecho=editar label
		Entity *aux=start;
		if (mouse) mouse->UnSetMouse();
		do {
			if (aux->CheckMouse(x,y)) { 
				if (aux->type==ET_PROCESO) break;
				if (button==GLUT_RIGHT_BUTTON) {
					aux->SetEdit(); return;
				} else {
					if (glutGetModifiers()==GLUT_ACTIVE_SHIFT) {
						aux=DuplicateEntity(aux);
					}
					aux->SetMouse();
					start->Calculate();
					edit=NULL;
					return;
				}
				break;
			}
			aux=aux->all_next;
		} while (aux!=start);
//		if (button==GLUT_RIGHT_BUTTON) { // click en el aire, derecho=pan
			m_x0=x; m_y0=y; panning=true;
//		}
	} else {
		if (button==GLUT_MIDDLE_BUTTON) {
			ZoomExtend(m_x0-d_dx,m_y0-d_dy,x-d_dx,y-d_dy);
			selecting_zoom=false;
			return;
		}
		panning=false;
		if (mouse) {
			if (trash && mouse->type==ET_OPCION) {
				Entity *p=mouse->parent;
				mouse->UnLink(); 
				p->Calculate();
			} 
			mouse->UnSetMouse();
		}
		start->Calculate();
	}
}

void keyboard_cb(unsigned char key, int x, int y) {
	if (!edit) {
		if (key==27) Salir();
		return;
	} else {
		edit->EditLabel(key);
	}
}

void keyboard_esp_cb(int key, int x, int y) {
	if (edit) edit->EditSpecialLabel(key);
}

void initialize() {
	glutInitDisplayMode (GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize (win_w,win_h);
	glutInitWindowPosition (100,100);
	glutCreateWindow ("PSDraw v2");
	glutIdleFunc (idle_func);
	glutDisplayFunc (display_cb);
	glutReshapeFunc (reshape_cb);
	glutMouseFunc(mouse_cb);
	glutMotionFunc(motion_cb);
	glutKeyboardFunc(keyboard_cb);
	glutSpecialFunc(keyboard_esp_cb);
	glutPassiveMotionFunc(passive_motion_cb);
	glClearColor(color_back[0],color_back[1],color_back[2],1.f);
}

bool Connect(int port, int id) {
	zocket = zocket_llamar(port,"127.0.0.1");
	if (zocket==ZOCKET_ERROR) return false;
	stringstream ss;
	ss<<"hello "<<id<<"\n";
	string s=ss.str();
	zocket_escribir(zocket,s.c_str(),s.size());
	if (zocket==ZOCKET_ERROR) return false;
	
}

bool SendUpdate(bool run) {
	ofstream fout(fname.c_str());
	if (!fout.is_open()) return false;
	start->Print(fout);
	fout.close();
	if (zocket==ZOCKET_ERROR) return false;
	if (run)
		zocket_escribir(zocket,"run\n",4);
	else
		zocket_escribir(zocket,"reload\n",7);
	if (zocket==ZOCKET_ERROR) return false;
	return true;
}

bool SendHelp() {
	if (zocket==ZOCKET_ERROR) return false;
	zocket_escribir(zocket,"help\n",5);
	if (zocket==ZOCKET_ERROR) return false;
	return true;
}

int main(int argc, char **argv) {
	for (int i=0;i<circle_steps;i++) {
		cosx[i]=cos((i*2*M_PI)/circle_steps);
		sinx[i]=sin((i*2*M_PI)/circle_steps);
	}
	cosx[circle_steps]=cosx[0];
	sinx[circle_steps]=sinx[0];
	glutInit (&argc, argv);
	initialize();
	int id=-1, port=-1;
	for(int i=1;i<argc;i++) { 
		string a(argv[i]);
		if (a=="--noedit") edit_on=false;
		else if (a.size()>=5 && a.substr(0,5)=="--id=") {
			id=atoi(a.substr(5).c_str());
		} else if (a.size()>=7 && a.substr(0,7)=="--port=") {
			port=atoi(a.substr(7).c_str());
		}
		else fname=a;
	}
	if (port!=-1 && id!=-1) Connect(port,id);
	if (fname.length()) Load(fname.c_str());
	else Load();
	glutMainLoop();
	return 0;
}
