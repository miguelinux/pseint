#include <iostream>
#include <sstream>
#include <cstdlib>
#include "GLstuff.h"
#include "Entity.h"
#include "Global.h"
#include "Draw.h"
#include "Events.h"
#include <wx/wx.h>
#include "Text.h"
using namespace std;

static int edit_pos; // posición del cursor cuando se edita un texto

bool Entity::nassi_shneiderman=false;
bool Entity::alternative_io=false; 
bool Entity::shape_colors=false; 
bool Entity::enable_partial_text=true;
bool Entity::show_comments=true;
static const int max_label_len_sec=25; // maxima longitud de un label antes de que se muestre cortado y con ..., para instrucciones secuenciales
static const int max_label_len_cont=15; // maxima longitud de un label antes de que se muestre cortado y con ..., para estructuras de control

// tamaño de las letras
#define char_w 12
#define char_h 18

#ifdef _FOR_EXPORT
#define SetModified()
#endif

void Entity::GetTextSize(const string &label, int &w, int &h) {
	w=label.size()*char_w;
	h=char_h;
}

void Entity::GetTextSize(int &w, int &h) {
	int pt = IsLabelCropped();
	w=(pt==0?label.size():pt)*char_w;
	h=char_h;
}

Entity::Entity(ETYPE _type, string _label) :type(_type),label(_label) {
	if (nassi_shneiderman) {
		if (_type==ET_ESCRIBIR) lpre="ESCRIBIR ";
		else if (_type==ET_COMENTARIO) lpre="// ";
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
		n_child=0;
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
		if (nassi_shneiderman) {
			child[1]->lpre=" DESDE ";
			child[1]->SetLabel("");
			child[2]->lpre=" CON PASO ";
			child[2]->SetLabel("");
			child[3]->lpre=" HASTA ";
			child[3]->SetLabel("");
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
	Calculate(false); // para que tengan un tamaño inicial no nulo al arrastrarlas desde la shapebar
}

Entity::~Entity() {
	if (this==edit) UnsetEdit();
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
	if (edit==this) return;
	if (edit) edit->UnsetEdit();
	edit=this; EditLabel(0);
	if (enable_partial_text)
		this->SetLabel(label,true);
	edit_pos=label.size();
	error.clear();
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

void Entity::EditSpecialLabel(int key) {
	if (key==WXK_LEFT) {
		if (edit_pos>0) edit_pos--;
	}
	else if (key==WXK_RIGHT) {
		if (edit_pos<int(label.size())) edit_pos++;
	}
	else if (key==WXK_HOME) {
		edit_pos=0;
	}
	else if (key==WXK_END) {
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
		if (enable_partial_text) SetLabel(label,true);
	} else {
		label.insert(edit_pos++,string(1,key));
		SetLabel(label,true);
	}
}

int Entity::IsLabelCropped ( ) {
	if (!enable_partial_text || this==edit) return 0;
	int max_len = type<=ET_ASIGNAR?max_label_len_sec:max_label_len_cont;
	if (int(label.size())>max_len) return max_len; else return 0;
}


void Entity::SetLabel(string _label, bool recalc) {
	if (_label!=label) SetModified();
	for (unsigned int i=0;i<label.size();i++) if (label[i]=='\'') label[i]='\"';
	label=_label; 
	GetTextSize(t_w,t_h); w=t_w; h=t_h;
	int aux; GetTextSize(lpre,t_prew,aux); t_w+=t_prew;
	if (recalc) {
		if (nassi_shneiderman) { 
			if (start) Entity::CalculateAll(); // todo: recalcula demas, parche hasta que analice bien donde esta el problema con el otro metodo
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
		if (child[i]!=mouse && d_x+child_dx[i]+selection_tolerance_x>x && d_x+child_dx[i]-selection_tolerance_x<x && y>d_y-d_h-flecha_h-selection_tolerance_y && y<d_y-d_h-flecha_h+selection_tolerance_y)
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
	if (type==ET_COMENTARIO && mouse->type!=ET_COMENTARIO && IsOutOfProcess()) return false;
	if (type==ET_OPCION || type==ET_AUX_PARA || (type==ET_PROCESO&&variante) || mouse==next || nolink) return false;
	return (x>d_x-d_w/2 && x<d_x+d_w/2 && y>d_y-d_bh-selection_tolerance_y && y<d_y-d_bh+selection_tolerance_y);
}

void Entity::UnLink() {
	SetModified();
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
	SetModified();
	e->prev=this; e->next=next;
	if (next) next->prev=e;
	next=e;
	e->parent=parent;
	e->child_id=child_id;
}

void Entity::RemoveChild(int j) { // elimina un hijo de la lista, reduciendo n_child
	SetModified();
	for (int i=j;i<n_child-1;i++) {
		child[i]=child[i+1];
		child_dx[i]=child_dx[i+1];
		child_bh[i]=child_bh[i+1];
		if (child[i]) child[i]->child_id=i;
	}
	n_child--;
}

void Entity::InsertChild(int i, Entity *e) { // similar a LinkChild, pero agrega un hijo, no reemplaza a uno que ya estaba
	SetModified();
	LinkChild(n_child,e); // agrega al final
	MoveChild(n_child-1,i);
}
void Entity::MoveChild(int i0, int i1) {
	if (i0==i1) return;
	SetModified();
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
	SetModified();
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
	begin_texto();
	glColor3fv(color_label_fix);
	for (unsigned int i=0;i<lpre.size();i++) {
		dibujar_caracter(lpre[i]);
	}
	glColor3fv(edit==this?color_selection:(type==ET_PROCESO?color_arrow:(type==ET_COMENTARIO?color_comment:color_label)));
	int llen=label.size(), crop_len = IsLabelCropped();
	if (crop_len) llen=crop_len-3;
	for (int i=0;i<llen;i++) {
		dibujar_caracter(label[i]);
	}
	if (llen!=int(label.size()))
		for (unsigned int i=0;i<3;i++)
			dibujar_caracter('.');
	end_texto();
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
	if (!show_comments && type==ET_COMENTARIO) return;
	if (nassi_shneiderman) DrawNassiShne(force);
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
	if (!nolink || this==nolink) {
		int old=bwl+bwr;
		bwl+=(aw-old)/2;
		bwr+=(aw-old)/2;
		int nc=n_child; if (type==ET_PARA) nc=1;
		for (int i=0;i<nc;i++)
			if (child[i]) 
				child[i]->ResizeW(child[i]->bwl+child[i]->bwr+(aw-old)/nc,false);
		if (type==ET_SI) {
			int dx=(aw-old)/4;
			if (child[0]) child[0]->MoveX(dx);
			if (child[1]) child[1]->MoveX(-dx);
			child_dx[0]+=dx; child_dx[1]-=dx;
		} else if (type==ET_SEGUN) {
			int dx=(aw-old)/2;
			int dx0=dx-dx/nc;
			for(int i=0;i<n_child;i++) { 
				int dd=-dx0+2*dx*(i)/nc;
				child[i]->MoveX(dd);
				child_dx[i]+=dd;
			}
		}
	}
	if (up && prev) prev->ResizeW(aw,true);
	if (!up && next) next->ResizeW(aw,false);
}

void Entity::Calculate(int &gwl, int &gwr, int &gh) { // calcula lo propio y manda a calcular al siguiente y a sus hijos, y acumula en gw,gh el tamaño de este item (para armar el tamaño del bloque)
	if (!show_comments && type==ET_COMENTARIO) {
		gwl=gwr=gh=fx=fy=bwl=bwr=bh=0;
	} else {
		if (nassi_shneiderman) CalculateNassiShne();
		else CalculateClasico();
	}
	// pasar a la siguiente entidad
	if (next) {
		next->x=x;
		next->y=y-bh;
		next->Calculate(gwl,gwr,gh);
	} else {
		gwl=gwr=gh=0;
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

bool Entity::CheckMouse(int x, int y, bool click) {
	if (!edit_on || (!showbase&&type==ET_COMENTARIO)) return false;
	if (click && type==ET_OPCION) {
		if (x>=d_fx-d_bwl && x<=d_fx-d_bwl+flecha_w && y<=d_fy && y>=d_fy-d_h) { // agregar una opción más
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
		if (click && this==edit) {
			int bt=d_fx+t_dx+t_prew-t_w/2+(edit_on&&type==ET_OPCION?flecha_w/2:0);
			edit_pos=(x-bt+char_w/2)/char_w;
			if (edit_pos<0) edit_pos=0; else if (edit_pos>int(label.size())) edit_pos=label.size();
		}
		return true;
	}
	return false;
}

#define _tabs "\t"

#define _endl_this inline_comments<<endl; inline_comments=""; {stringstream ss; ss<<line_num<<":1"; code2draw[ss.str()]=LineInfo(process,this);} line_num++;
#define _endl_prev inline_comments<<endl; inline_comments=""; {stringstream ss; ss<<line_num<<":1"; code2draw[ss.str()]=LineInfo(NULL,this);} line_num++;
#define _endl_none inline_comments<<endl; inline_comments=""; {stringstream ss; ss<<line_num<<":1"; code2draw[ss.str()]=LineInfo(NULL,NULL);} line_num++;

#define _fix(label,def) (label.size()?label:def)

void Entity::Print(ostream &out, string tab, Entity *process, int &line_num) {
	static string inline_comments; 
	bool add_tab=false;
	if (type==ET_PROCESO) {
		add_tab=true;
		if (next) {
			out<<tab<<lpre<<_fix(label,"{sin_titulo}")<<_endl_this;
			if (next) next->Print(out,add_tab?tab+_tabs:tab,process,line_num);
			out<<tab<<"Fin"<<lpre.substr(0,lpre.size()-1)<<_endl_none;
			return;
		}
	} else if (type==ET_ESCRIBIR) {
		if (lang[LS_FORCE_SEMICOLON] && label[label.size()-1]==';') label=label.erase(label.size()-1);
		out<<tab<<"Escribir "<<_fix(label,"{lista_de_expresiones}")<<(variante?" Sin Saltar":"")<<(lang[LS_FORCE_SEMICOLON]?";":"")<<_endl_this;
	} else if (type==ET_LEER) {
		if (lang[LS_FORCE_SEMICOLON] && label[label.size()-1]==';') label=label.erase(label.size()-1);
		out<<tab<<"Leer "<<_fix(label,"{lista_de_variables}")<<(lang[LS_FORCE_SEMICOLON]?";":"")<<_endl_this;
	} else if (type==ET_MIENTRAS) {
		out<<tab<<"Mientras "<<_fix(label,"{condicion}")<<" Hacer"<<_endl_this;
		if (child[0]) child[0]->Print(out,tab+_tabs,process,line_num);
		out<<tab<<"FinMientras"<<_endl_prev;
	} else if (type==ET_REPETIR) {
		out<<tab<<"Repetir"<<_endl_prev;
		if (child[0]) child[0]->Print(out,tab+_tabs,process,line_num);
		out<<tab<<(variante?"Mientras Que ":"Hasta Que ")<<_fix(label,"{condicion}")<<_endl_this;
	} else if (type==ET_PARA) {
		if (variante) {
			out<<tab<<"Para Cada"<<_fix(label,"{variable}")<<" de "<<_fix(child[2]->label,"{arreglo}")<<" Hacer"<<_endl_this;
		} else {
			bool has_paso=child[2]->label!="1"&&child[2]->label!="+1"&&child[2]->label!="";
			out<<tab<<"Para "<<_fix(label,"{variable}")<<"<-"<<_fix(child[1]->label,"{valor_inicial}")<<" Hasta "<<_fix(child[3]->label,"{valor_final}")
				<<(has_paso?" Con Paso ":"")<<(has_paso?_fix(child[2]->label,"{paso}"):"") <<" Hacer"<<_endl_this;
		}
		if (child[0]) child[0]->Print(out,tab+_tabs,process,line_num);
		out<<tab<<"FinPara"<<_endl_prev;
	} else if (type==ET_SEGUN) {
		out<<tab<<"Segun "<<_fix(label,"{expresion}")<<" Hacer"<<_endl_this;
		for(int i=0;i<n_child-1;i++) { 
			child[i]->Print(out,tab+_tabs,process,line_num);
		}
		if (child[n_child-1]->child[0]) // de otro modo
			child[n_child-1]->Print(out,tab+_tabs,process,line_num);
		out<<tab<<"FinSegun"<<_endl_prev;
	} else if (type==ET_OPCION) {
		add_tab=true;
		out<<tab<<_fix(label,"{expresion}")<<":"<<_endl_this;
		if (child[0]) child[0]->Print(out,tab+_tabs,process,line_num);
	} else if (type==ET_SI) {
		out<<tab<<"Si "<<_fix(label,"{condicion}")<<" Entonces"<<_endl_this;
		if (child[1]) { child[1]->Print(out,tab+_tabs,process,line_num); }
		if (child[0]) { out<<tab<<"Sino"<<_endl_prev; }
		if (child[0]) { child[0]->Print(out,tab+_tabs,process,line_num); }
		out<<tab<<"FinSi"<<_endl_prev;
	} else if (type==ET_ASIGNAR) {
		if (lang[LS_FORCE_SEMICOLON] && label[label.size()-1]==';') label=label.erase(label.size()-1);
		if (label.size()) { out<<tab<<label<<(lang[LS_FORCE_SEMICOLON]?";":"")<<_endl_this; }
	} else if (type==ET_COMENTARIO) {
		if (variante) { 
			if (inline_comments=="") inline_comments=" // "; else inline_comments+=" - ";
			inline_comments+=label;
		} else {
			string prev_ilc = inline_comments;
			inline_comments = "";
			out<<tab<<"// "<<label<<_endl_this;
			inline_comments = prev_ilc;
		}
	}
	if (next) next->Print(out,add_tab?tab+_tabs:tab,process,line_num);
}

Entity *Entity::all_any=NULL;

void Entity::SetPosition (int x0, int y0) {
	d_fx=d_x=x0; 
	d_fy=d_y=y0;
	fx=x=x0-d_dx;
	fy=y=y0-d_dy;
//	Calculate();
}


void Entity::UnsetEdit ( ) {
	if (enable_partial_text) {
		Entity *prev=this;
		prev->EditLabel(27);
	}
}

Entity * Entity::GetTopEntity ( ) {
	Entity *top=this; 
	while(top->prev) top=top->prev;
	return top;
}

void Entity::CalculateAll (bool also_text_size) {
	if (also_text_size) {
		Entity *aux=Entity::all_any;
		do {
			aux->SetLabel(aux->label);
			aux=aux->all_next;
		} while (aux && aux!=Entity::all_any);
	}
	start->GetTopEntity()->Calculate();
}

Entity *Entity::GetNextNoComment() {
	Entity *e_aux = next;
	while (e_aux && e_aux->type==ET_COMENTARIO) e_aux = e_aux->next;
	return e_aux;
}

bool Entity::IsOutOfProcess(Entity *next_no_commnet) {
	return !next_no_commnet || // fin proceso
		(next_no_commnet->type==ET_PROCESO&&!next_no_commnet->variante); // proceso
}

bool Entity::IsOutOfProcess() {
	return IsOutOfProcess(GetNextNoComment());
}

