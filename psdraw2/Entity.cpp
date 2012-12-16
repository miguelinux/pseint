#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
#include "Entity.h"
#include "Global.h"
#include "Draw.h"
using namespace std;

static int edit_pos; // posición del cursor cuando se edita un texto

bool Entity::nassi_schneiderman=false; // alterna el formato entre diagrama de flujo estandar y diagrama de Nassi-Schneiderman

// tamaño de las letras
#define char_w 12
#define char_h 18

void Entity::GetTextSize(const string &label, int &w, int &h) {
	w=label.size()*char_w;
	h=char_h;
}

Entity::Entity(ETYPE _type, string _label) :type(_type),label(_label) {
	if (nassi_schneiderman) {
		if (_type==ET_ESCRIBIR) lpre="ESCRIBIR ";
		else if (_type==ET_LEER) lpre="LEER ";
		else if (_type==ET_PARA) lpre="PARA ";
		else if (_type==ET_REPETIR) lpre="HASTA QUE ";
		else if (_type==ET_MIENTRAS) lpre="MIENTRAS ";
	}		
	variante=false;
	if (!all_any) {
		all_any=this; 
		all_next=all_prev=this;
	} else { 
		all_next=all_any->all_next; 
		if (all_next) all_next->all_prev=this;
		all_any->all_next=this; 
		all_prev=all_any;
	}
	t_dx=t_dy=0; fx=x=0; fy=y=0; flecha_in=0;
	d_fx=d_fy=d_y=d_x=100;
	d_w=d_bh=d_h=d_bwl=d_bwr=bwl=bwr=bh=0;
	nolink=NULL;
	SetLabel(label);
	parent=prev=next=NULL; child=NULL;
	if (type==ET_PROCESO) { 
		lpre="Proceso ";
	} else if (type==ET_SI) { // dos hijos
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
		if (nassi_schneiderman) {
			child[1]->lpre=" DESDE ";
			child[2]->lpre=" CON PASO ";
			child[3]->lpre=" HASTA ";
		}
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
	if (this==all_any) all_any=all_next; // busca otra...
	if (this==all_any) all_any=NULL; // ...si no hay otra esta era la ultima
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
	int aux; GetTextSize(lpre,t_prew,aux); t_w+=t_prew;
	if (recalc) {
		if (nassi_schneiderman) { 
			if (start) start->Calculate(); // todo: recalcula demas, parche hasta que analice bien donde esta el problema con el otro metodo
		} else {
			Calculate();
			if (parent) parent->Calculate(true);
		}
	}
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
	return (x>d_x-d_w/2 && x<d_x+d_w/2 && y>d_y-d_bh/2-2*flecha_h/3-selection_tolerance_y && y<d_y-d_bh/2-2*flecha_h/3+selection_tolerance_y);
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

void Entity::DrawText() {
	glPushMatrix();
	glTranslated(d_fx+t_dx-t_w/2+(edit_on&&type==ET_OPCION?flecha_w/2:0),d_fy-(d_h/2+margin)+t_dy,0);
	glScaled((.105*d_w)/w,(.15*d_h)/h,.1);
	glColor3fv(color_label_fix);
	for (unsigned int i=0;i<lpre.size();i++) {
		dibujar_caracter(lpre[i]);
	}
	glColor3fv(edit==this?color_selection:(type==ET_PROCESO?color_arrow:color_label));
	for (unsigned int i=0;i<label.size();i++) {
		dibujar_caracter(label[i]);
	}
	glPopMatrix();
	if (edit==this && mouse!=this && w>0) {
		blink++; if (blink==20) blink=0;
		if (blink<10) {
			glBegin(GL_LINES);
			int lz=label.size(); if (!lz) lz=1;
			lz= d_fx+t_dx-t_w/2+t_prew+(t_w-t_prew)*edit_pos*d_w/lz/w+(type==ET_OPCION?flecha_w/2:0);
			glVertex2i(lz,d_fy-h/2-t_h/2-margin/2+t_dy);
			glVertex2i(lz,d_fy-h/2+t_h/2+margin/2+t_dy);
			glEnd();
		}
	}
}

void Entity::Draw(bool force) {
	if (nassi_schneiderman) DrawNassiSchne(force);
	else DrawClasico(force);
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
void Entity:: ResizeW(int aw, bool up) {
	int old=bwl+bwr;
	bwl+=(aw-old)/2;
	bwr+=(aw-old)/2;
	int nc=n_child; if (type==ET_PARA) nc=1;
	for (int i=0;i<nc;i++)
		if (child[i]) 
			child[i]->ResizeW(child[i]->bwl+child[i]->bwr+(aw-old)/nc,false);
	if (type==ET_SI) {
		int dx=(aw-old)/4;
		if (child[0]) child[0]->MoveX(-dx);
		if (child[1]) child[1]->MoveX(dx);
		child_dx[0]-=dx; child_dx[1]+=dx;
	} else if (type==ET_SEGUN) {
		int dx=(aw-old)/2;
		int dx0=dx-dx/nc;
		for(int i=0;i<n_child;i++) { 
			int dd=-dx0+2*dx*(i)/nc;
			child[i]->MoveX(dd);
			child_dx[i]+=dd;
		}
	}
	if (up && prev) prev->ResizeW(aw,true);
	if (!up && next) next->ResizeW(aw,false);
}

void Entity::Calculate(int &gwl, int &gwr, int &gh) { // calcula lo propio y manda a calcular al siguiente y a sus hijos, y acumula en gw,gh el tamaño de este item (para armar el tamaño del bloque)
	if (nassi_schneiderman) CalculateNassiSchne();
	else CalculateClasico();
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
	if (x>=d_fx+t_dx-d_w/2 && x<=d_fx+t_dx+d_w/2 && y<=d_fy && y>=d_fy-d_h) {
		m_x=x-d_fx;
		m_y=y-d_fy;
		if (this==edit) {
			int bt=d_fx+t_dx+t_prew-t_w/2+(edit_on&&type==ET_OPCION?flecha_w/2:0);
			edit_pos=(x-bt+char_w/2)/char_w;
			if (edit_pos<0) edit_pos=0; else if (edit_pos>int(label.size())) edit_pos=label.size();
		}
		return true;
	}
	return false;
}

#define _tabs "    "

void Entity::Print(ostream &out, string tab) {
	bool add_tab=false;
	if (type==ET_PROCESO) {
		add_tab=true;
		if (next) {
			out<<tab<<lpre<<label<<endl;
			if (next) next->Print(out,add_tab?tab+_tabs:tab);
			out<<tab<<"Fin"<<lpre.substr(0,lpre.size()-1)<<endl;
			return;
		}
	} else if (type==ET_ESCRIBIR) {
		out<<tab<<"Escribir "<<label<<";"<<endl;
	} else if (type==ET_LEER) {
		out<<tab<<"Leer "<<label<<";"<<endl;
	} else if (type==ET_MIENTRAS) {
		out<<tab<<"Mientras "<<label<<" Hacer"<<endl;
		if (child[0]) child[0]->Print(out,tab+_tabs);
		out<<tab<<"FinMientras"<<endl;
	} else if (type==ET_REPETIR) {
		out<<tab<<"Repetir"<<endl;
		if (child[0]) child[0]->Print(out,tab+_tabs);
		out<<tab<<(variante?"Mientras Que ":"Hasta Que ")<<label<<endl;
	} else if (type==ET_PARA) {
		if (variante)
			out<<tab<<"Para Cada"<<label<<" de "<<child[2]->label<<" Hacer"<<endl;
		else
			out<<tab<<"Para "<<label<<"<-"<<child[1]->label<<" Hasta "<<child[3]->label<<" Con Paso "<<child[2]->label<<" Hacer"<<endl;
		if (child[0]) child[0]->Print(out,tab+_tabs);
		out<<tab<<"FinPara"<<endl;
	} else if (type==ET_SEGUN) {
		out<<tab<<"Segun "<<label<<" Hacer"<<endl;
		for(int i=0;i<n_child-1;i++) { 
			child[i]->Print(out,tab+_tabs);
		}
		if (child[n_child-1]->child[0]) // de otro modo
			child[n_child-1]->Print(out,tab+_tabs);
		out<<tab<<"FinSegun"<<endl;
	} else if (type==ET_OPCION) {
		add_tab=true;
		out<<tab<<label<<":"<<endl;
		if (child[0]) child[0]->Print(out,tab+_tabs);
	} else if (type==ET_SI) {
		out<<tab<<"Si "<<label<<" Entonces"<<endl;
		if (child[1]) child[1]->Print(out,tab+_tabs);
		if (child[0]) out<<tab<<"Sino"<<endl;
		if (child[0]) child[0]->Print(out,tab+_tabs);
		out<<tab<<"FinSi"<<endl;
	} else if (type==ET_ASIGNAR) {
		out<<tab<<label<<";"<<endl;
	}
	if (next) next->Print(out,add_tab?tab+_tabs:tab);
}

Entity *Entity::all_any=NULL;

void Entity::SetPosition (int x0, int y0) {
	d_fx=d_x=x0; 
	d_fy=d_y=y0;
	fx=x=x0-d_dx;
	fy=y=y0-d_dy;
//	Calculate();
}
