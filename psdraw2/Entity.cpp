#include "Entity.h"
#include "Global.h"
#include "Draw.h"
#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
using namespace std;

static int edit_pos; // posición del cursor cuando se edita un texto

static const int flecha_h=25; // separacion entre bloques consecutivos
static const int flecha_w=20; // separacion entre bloques hermanos
static const int flecha_d=5; // tamaño de la punta de la flecha
static const int selection_tolerance_y=12; // tolerancia en y para la seleccion de puntos
static const int selection_tolerance_x=30; // tolerancia en x para la seleccion de puntos
static const int shadow_delta_x=4; // diferencia entre la posicion de un objeto y su sombra
static const int shadow_delta_y=5; // diferencia entre la posicion de un objeto y su sombra
int margin=6; // margen entre cuadro y texto en un bloque (y para los botones de confirm, por eso no es static, ¿ni const?)
static const int vf_size=5;

#ifdef DrawText
// maldito windows.h
#undef DrawText
#endif

void Entity::GetTextSize(const string &label, int &w, int &h) {
	w=label.size()*12;
	h=18;
}

Entity::Entity(ETYPE _type, string _label, bool reg_in_all) :type(_type),label(_label) {
	variante=false;
	if (reg_in_all) {
		if (!all_any) { 
			all_any=this; 
			all_next=all_prev=this;
		} else { 
			all_next=all_any->all_next; 
			if (all_next) all_next->all_prev=this;
			all_any->all_next=this; 
			all_prev=all_any;
		}
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
		child_bh[0]=child_bh[1]=0;
		child_dx[0]=child_dx[1]=0;
	} else if (type==ET_PARA) { // cuatro hijos
		n_child=4; flecha_in=flecha_h;
		child_bh=(int*)malloc(sizeof(int)*4);
		child_dx=(int*)malloc(sizeof(int)*4);
		child=(Entity**)malloc(sizeof(Entity*)*4);
		child[0]=child[1]=child[2]=child[3]=NULL;
		child_bh[0]=child_bh[1]=child_bh[2]=child_bh[3]=0;
		child_dx[0]=child_dx[1]=child_dx[2]=child_dx[3]=0;
		LinkChild(1,new Entity(ET_AUX_PARA,""));
		LinkChild(2,new Entity(ET_AUX_PARA,""));
		LinkChild(3,new Entity(ET_AUX_PARA,""));
	} else if (type==ET_MIENTRAS||type==ET_REPETIR||type==ET_OPCION||type==ET_SEGUN) { // un hijo
		n_child=1;
		child_bh=(int*)malloc(sizeof(int)*1);
		child_dx=(int*)malloc(sizeof(int)*1);
		child=(Entity**)malloc(sizeof(Entity*)*1);
		child[0]=NULL; child_dx[0]=child_bh[0]=0;
		if (type!=ET_OPCION && type!=ET_SEGUN) flecha_in=flecha_h; 
		else if (type==ET_SEGUN) LinkChild(0,new Entity(ET_OPCION,"De Otro Modo"));
	} else {
		n_child=0;
	}
	t_dy=(type==ET_SEGUN?-t_h/2-margin:(type==ET_PARA?(t_h+margin)/2:0));
}

Entity::~Entity() {
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

void Entity::SetEdit() {
	edit=this; EditLabel(0);
	edit_pos=label.size();
}

void Entity::SetMouse() {
	mouse=this;
	SetNolink(this,false);
}

void Entity::UnSetMouse() {
	mouse=NULL;
	SetNolink(NULL,false);
	if (!parent && !prev) {
		if (entity_to_del) delete entity_to_del;
		entity_to_del=this;
	}
}

void Entity::SetNolink(Entity *m,bool n) {
	nolink=m;
	for (int i=0;i<n_child;i++)
		if (child[i]) child[i]->SetNolink(m,true);
	if (n && next) next->SetNolink(m,true);
	
}

void Entity::EditSpecialLabel(unsigned char key) {
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

void Entity::EditLabel(unsigned char key) {
	static bool acento=false; // para emular el acento como dead key
	if (acento) {
		if (key=='a') key='á';
		else if (key=='e') key='é';
		else if (key=='i') key='í';
		else if (key=='o') key='ó';
		else if (key=='u') key='ú';
		else if (key=='A') key='Á';
		else if (key=='E') key='É';
		else if (key=='I') key='Í';
		else if (key=='O') key='Ó';
		else if (key=='U') key='Ú';
		acento=false;
	}
	if (key==0) return;
	if (key==180) { acento=true; return; }
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

void Entity::SetLabel(string _label, bool recalc) {
	modified=true;
	for (unsigned int i=0;i<label.size();i++) if (label[i]=='\'') label[i]='\"';
	label=_label; GetTextSize(label,t_w,t_h); w=t_w; h=t_h;
	if (!w) w=margin*6;
	h+=2*margin; if (type!=ET_PROCESO) w+=2*margin;
	if (type==ET_REPETIR||type==ET_MIENTRAS||type==ET_SI) {
		w*=2; h*=2;
	} else if (type==ET_ESCRIBIR||type==ET_LEER) {
		w+=2*margin;
	} else if (type==ET_PARA) {
		h=2*h+3*margin; w=1.3*w+2*margin;
	} else if (type==ET_SEGUN) {
		h*=2;
	}
	if (recalc) Calculate();
	if (recalc && parent) parent->Calculate(true);
}

int Entity::CheckLinkChild(int x, int y) {
	if (nolink||!n_child||type==ET_SEGUN) return -1;
	if (type==ET_PARA||type==ET_REPETIR) { // h no tiene sentido porque la forma esta al final
		if (child[0]!=mouse && d_x+child_dx[0]+selection_tolerance_x>x && d_x+child_dx[0]-selection_tolerance_x<x && y>d_y-flecha_in-2*flecha_h/3-selection_tolerance_y && y<d_y-flecha_in-2*flecha_h/3+selection_tolerance_y)
			return 0; else return -1;
	}
	for (int i=0;i<n_child;i++) {
		if (child[i]!=mouse && d_x+child_dx[i]+selection_tolerance_x>x && d_x+child_dx[i]-selection_tolerance_x<x && y>d_y-d_h-2*flecha_h/3-selection_tolerance_y && y<d_y-d_h-2*flecha_h/3+selection_tolerance_y)
			return i;
	}
	return -1;
}

int Entity::CheckLinkOpcion(int x, int y) {
	if (y<d_dy+child[0]->y-child[0]->h || y>d_dy+child[0]->y+2*child[0]->h) return -1;
	if (x<d_dx+child[0]->x-child[0]->w || x>d_dx+child[n_child-1]->x+child[n_child-1]->w) return -1;
	for (int i=0;i<n_child-1;i++) {
		if (x<d_dx+child[i]->x) {
			if (i==mouse->child_id) return -1; else return i;
		}
	}
	return -1;
	
}

bool Entity::CheckLinkNext(int x, int y) {
	if (type==ET_OPCION || type==ET_AUX_PARA || (type==ET_PROCESO && !next) || mouse==next || nolink) return false;
	return (x>d_x-d_w/2 && x<d_x+d_w/2 && y>d_y-d_bh-2*flecha_h/3-selection_tolerance_y && y<d_y-d_bh-2*flecha_h/3+selection_tolerance_y);
}

void Entity::UnLink() {
	modified=true;
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

void Entity::LinkNext(Entity *e) {
	modified=true;
	e->prev=this; e->next=next;
	if (next) next->prev=e;
	next=e;
	e->parent=parent;
	e->child_id=child_id;
}

void Entity::RemoveChild(int j) { // elimina un hijo de la lista, reduciendo n_child
	modified=true;
	for (int i=j;i<n_child-1;i++) {
		child[i]=child[i+1];
		child_dx[i]=child_dx[i+1];
		child_bh[i]=child_bh[i+1];
		if (child[i]) child[i]->child_id=i;
	}
	n_child--;
}

void Entity::InsertChild(int i, Entity *e) { // similar a LinkChild, pero agrega un hijo, no reemplaza a uno que ya estaba
	modified=true;
	LinkChild(n_child,e); // agrega al final
	MoveChild(n_child-1,i);
}
void Entity::MoveChild(int i0, int i1) {
	if (i0==i1) return;
	modified=true;
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
void Entity::LinkChild(int i, Entity *e) { // i esta en base 0 y no puede ser negativo ni mayor a n_child
	modified=true;
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

void Entity::Tick() {
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

void Entity::DrawShapeSolid(const float *color,int x, int y, int w, int h) {
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

void Entity::DrawShapeBorder(const float *color,int x, int y, int w, int h) {
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

inline void DrawTrue(int x, int y) {
	glVertex2d(x,y); glVertex2d(x-margin/2,y+2*vf_size);
	glVertex2d(x,y); glVertex2d(x+margin/2,y+2*vf_size);
}

inline void DrawFalse(int x, int y) {
	glVertex2d(x-vf_size/2,y); glVertex2d(x-vf_size/2,y+2*vf_size);
	glVertex2d(x-vf_size/2,y+2*vf_size); glVertex2d(x+vf_size/2,y+2*vf_size);
	glVertex2d(x-vf_size/2,y+vf_size); glVertex2d(x+vf_size/2,y+vf_size);
}

inline void DrawFlechaDown(int x, int y1, int y2) {
	glVertex2d(x,y1); glVertex2d(x,y2);
	glVertex2d(x-flecha_d,y2+flecha_d); glVertex2d(x,y2);
	glVertex2d(x+flecha_d,y2+flecha_d); glVertex2d(x,y2);
}

inline void DrawFlechaDownHead(int x, int y2) {
	glVertex2d(x-flecha_d,y2+flecha_d); glVertex2d(x,y2);
	glVertex2d(x+flecha_d,y2+flecha_d); glVertex2d(x,y2);
}

inline void DrawFlechaUp(int x, int y1, int y2) {
	glVertex2d(x,y1); glVertex2d(x,y2);
	glVertex2d(x-flecha_d,y2-flecha_d); glVertex2d(x,y2);
	glVertex2d(x+flecha_d,y2-flecha_d); glVertex2d(x,y2);
}

inline void DrawFlechaR(int x1, int x2, int y) {
	glVertex2d(x1,y); glVertex2d(x2,y);
	glVertex2d(x2-flecha_d,y-flecha_d); glVertex2d(x2,y);
	glVertex2d(x2-flecha_d,y+flecha_d); glVertex2d(x2,y);
}

inline void DrawFlechaL(int x1, int x2, int y) {
	glVertex2d(x1,y); glVertex2d(x2,y);
	glVertex2d(x2+flecha_d,y-flecha_d); glVertex2d(x2,y);
	glVertex2d(x2+flecha_d,y+flecha_d); glVertex2d(x2,y);
}

void Entity::DrawText() {
	glColor3fv(edit==this?color_selection:(type==ET_PROCESO?color_arrow:color_label));
	glPushMatrix();
	glTranslated(d_fx-t_w/2+(edit_on&&type==ET_OPCION?flecha_w/2:0),d_fy-(d_h/2+margin)+t_dy,0);
	glScaled((.105*d_w)/w,(.15*d_h)/h,.1);
	for (unsigned int i=0;i<label.size();i++)
		dibujar_caracter(label[i]);
	glPopMatrix();
	if (edit==this && mouse!=this) {
		blink++; if (blink==20) blink=0;
		if (blink<10) {
			glBegin(GL_LINES);
			int lz=label.size(); if (!lz) lz=1;
			lz= d_fx-t_w/2+ t_w*edit_pos*d_w/lz/w+(type==ET_OPCION?flecha_w/2:0);
			glVertex2i(lz,d_fy-h/2-t_h/2-margin/2+t_dy);
			glVertex2i(lz,d_fy-h/2+t_h/2+margin/2+t_dy);
			glEnd();
		}
	}
}

void Entity::Draw(bool force) {
	if (!force && (type==ET_OPCION || type==ET_AUX_PARA)) return;
	if (this==mouse && (prev||parent)) // si se esta moviendo con el mouse, dibujar un ghost donde lo agregariamos al soltar
		DrawShapeBorder(color_ghost,d_dx+x,d_dy+y,bwr+bwl,h);
	else /*if (type!=ET_OPCION && type!=ET_SEGUN)*/ // sombra
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
			glVertex2d(d_x+child_dx[0]+(child[0]?child[0]->child_dx[0]:0),d_y-d_bh+flecha_h); glVertex2d(d_x,d_y-d_bh+flecha_h);
			glVertex2d(d_x,d_y-d_bh+flecha_h); glVertex2d(d_x+child_dx[n_child-1]+(child[n_child-1]?child[n_child-1]->child_dx[0]:0),d_y-d_bh+flecha_h);
		} else if (type==ET_MIENTRAS) {
			DrawTrue(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
			DrawFalse(d_fx+d_w/2+2*vf_size,d_fy-d_h/2+vf_size);
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
			if (variante) {
				DrawFalse(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
				DrawTrue(d_fx-d_w/2-2*vf_size,d_fy-d_h/2+vf_size);
			} else {
				DrawTrue(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
				DrawFalse(d_fx-d_w/2-2*vf_size,d_fy-d_h/2+vf_size);
			}
			glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra
			DrawFlechaL(d_x,d_x-d_bwl,d_fy-d_h/2); // sigue a la izquierda
			DrawFlechaUp(d_x-d_bwl,d_fy-d_h/2,d_y); // sube
			DrawFlechaR(d_x-d_bwl,d_x,d_y); // entra arriba de la condicion
			glVertex2i(d_fx,d_fy+flecha_h); glVertex2i(d_fx,d_fy); // flecha a la siguiente instruccion
		} else if (type==ET_SI) {
			DrawTrue(d_fx+d_w/2+2*vf_size,d_fy-d_h/2+vf_size);
			DrawFalse(d_fx-d_w/2-2*vf_size,d_fy-d_h/2+vf_size);
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
		if (type!=ET_OPCION && (prev||parent)) DrawFlechaDownHead(d_x,d_y-flecha_in); // no en inicio
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
	DrawText();
	if (!nolink) {
		if (type==ET_SEGUN) {
			for(int i=0;i<n_child;i++) { 
				child[i]->Draw(true);
			}
		} else  if (type==ET_PARA) {
			glColor3fv(color_border);
			glBegin(GL_LINES);
			glVertex2i(d_fx-w/2,d_fy-d_h/2); glVertex2i(d_fx+w/2,d_fy-d_h/2); // separadores de las cuatro partes del circulo
			if (!variante) {
				glVertex2i(d_x+child_dx[1],d_fy-d_h/2); glVertex2i(d_x+child_dx[1],d_fy-d_h+margin);
				glVertex2i(d_x+child_dx[2],d_fy-d_h/2); glVertex2i(d_x+child_dx[2],d_fy-d_h+margin);
				glEnd();
				child[1]->DrawText();
				child[2]->DrawText();
				child[3]->DrawText();
			} else {
				glEnd();
	//			child[1]->DrawText();
				child[2]->DrawText();
	//			child[3]->DrawText();
			}
		}
	}
}

void Entity::Calculate(bool also_parent) { // devuelve el tamaño total del bloque
	int awl,awr,ah;
	Calculate(awl,awr,ah);
	if (also_parent && parent) parent->Calculate(true);
}

void Entity::MoveX(int dx) { // mueve al item y todos sus hijos en x
	x+=dx; fx+=dx;
	for (int i=0;i<n_child;i++)
		if (child[i]) child[i]->MoveX(dx);
	if (next) next->MoveX(dx);
}

void Entity::Calculate(int &gwl, int &gwr, int &gh) { // calcula lo propio y manda a calcular al siguiente y a sus hijos, y acumula en gw,gh el tamaño de este item (para armar el tamaño del bloque)
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
			
			// averiguar cuanto miden las tres etiquetas de abajo en el circulo
			child[1]->x=child[2]->x=child[3]->x=x;
			int vl=0,vr=0,vh=0;
			child[1]->Calculate(vl,vr,vh); 
			int v1=vl+vr; vl=vr=0;
			child[2]->Calculate(vl,vr,vh); 
			int v2=vl+vr; vl=vr=0;
			child[3]->Calculate(vl,vr,vh); 
			int v3=vl+vr;
			if (variante) { v1=v3=0; child[1]->w=child[3]->w=0; }
			
			// calcular el acnho del circulo, puede estar dominado por las tres etiquetas de abajo o por la propia 
			int v=v1+v2+v3-2*margin;
			w=(v>t_w?v:t_w)*1.3+2*margin;
			v+=2*margin;
			
			// acomodar el circulo
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
			
			// acomodar las tres etiquetas
			int cy=fy-h/2-margin/2;
			child[1]->fy=cy; child[1]->MoveX(fx-x+(-v+v1)/2); child_dx[1]=child[1]->fx+v1/2;
			child[2]->fy=cy; child[2]->MoveX(fx-x+(-v+v2)/2+v1); child_dx[2]=child[2]->fx+v2/2;
			child[3]->fy=cy; child[3]->MoveX(fx-x+(v-v3)/2); child_dx[3]=child[3]->fx+v3/2;
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

void Entity::CopyPos(Entity *o) {
	x=o->x; y=o->y;
	fx=o->fx; fy=o->fy;
	d_x=o->d_x; d_y=o->d_y;
	d_fx=o->d_fx; d_fy=o->d_fy;
}

bool Entity::CheckMouse(int x, int y) {
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

void Entity::Print(ostream &out, string tab) {
	bool add_tab=false;
	if (type==ET_PROCESO) {
		add_tab=true;
		if (next) {
			out<<tab<<"Proceso "<<pname<<endl;
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
		out<<tab<<(variante?"Mientras Que ":"Hasta Que ")<<label<<endl;
	} else if (type==ET_PARA) {
		if (variante)
			out<<tab<<"Para Cada"<<label<<" de "<<child[2]->label<<" Hacer"<<endl;
		else
			out<<tab<<"Para "<<label<<"<-"<<child[1]->label<<" Hasta "<<child[3]->label<<" Con Paso "<<child[2]->label<<" Hacer"<<endl;
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
		if (child[1]) child[1]->Print(out,tab+"   ");
		if (child[0]) out<<tab<<"Sino"<<endl;
		if (child[0]) child[0]->Print(out,tab+"   ");
		out<<tab<<"FinSi"<<endl;
	} else if (type==ET_ASIGNAR) {
		out<<tab<<label<<";"<<endl;
	}
	if (next) next->Print(out,add_tab?tab+"   ":tab);
}

Entity *Entity::all_any=NULL;
