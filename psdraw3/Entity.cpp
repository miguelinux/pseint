#include <iostream>
#include <sstream>
#include <cstdlib>
#include <set>
#include <wx/wx.h>
#include "GLstuff.h"
#include "Entity.h"
#include "Global.h"
#include "Draw.h"
#include "Events.h"
#include "Text.h"
#ifndef _FOR_EXPORT
#	include "ShapesBar.h"
#endif
using namespace std;

static int edit_pos; // posición del cursor cuando se edita un texto
static unsigned char flechita = 27;

bool Entity::nassi_shneiderman=false;
bool Entity::alternative_io=false; 
bool Entity::shape_colors=false; 
bool Entity::enable_partial_text=true;
bool Entity::show_comments=true;
int Entity::max_label_len[ET_COUNT];

// tamaño de las letras
#define char_w 12
#define char_h 18

#ifdef _FOR_EXPORT
#	define SetModified()
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

Entity::Entity(ETYPE _type, string _label, bool _variante) 
	: type(_type), variante(_variante), label(_label), nolink(NULL)
{
	t_dx=t_dy=0; fx=x=0; fy=y=0; flecha_in=0;
	d_fx=d_fy=d_y=d_x=100;
	d_w=d_bh=d_h=d_bwl=d_bwr=bwl=bwr=bh=0;
	SetLabel(label);
	if (type==ET_SI) { // dos hijos
		SetChildCount(2);
	} else if (type==ET_PARA) { // cuatro hijos
		SetChildCount(4);
		flecha_in=flecha_h;
		LinkChild(1,new Entity(ET_AUX_PARA,"")); GetChild(1)->SetLabels();
		LinkChild(2,new Entity(ET_AUX_PARA,"")); GetChild(2)->SetLabels();
		LinkChild(3,new Entity(ET_AUX_PARA,"")); GetChild(3)->SetLabels();
	} else if (type==ET_MIENTRAS||type==ET_REPETIR||type==ET_OPCION||type==ET_SEGUN||type==ET_SELECTION) { // un hijo
		SetChildCount(1);
		if (type!=ET_OPCION && type!=ET_SEGUN) flecha_in=flecha_h; 
		else if (type==ET_SEGUN) LinkChild(0,new Entity(ET_OPCION,"De Otro Modo",true));
	}
	SetLabels();
	Calculate(false); // para que tengan un tamaño inicial no nulo al arrastrarlas desde la shapebar
}

Entity::~Entity() {
	if (this==edit) UnsetEdit();
	if (entity_to_del==this) entity_to_del=NULL;
	if (this==mouse) UnSetMouse();
}

void Entity::SetEdit(bool ensure_caret_visibility) {
	if (edit==this) return;
	if (edit) edit->UnsetEdit();
	edit=this; EditLabel(0);
	if (enable_partial_text)
		this->SetLabel(label,true);
	SetEditPos(label.size(),ensure_caret_visibility);
	error.clear();
}

void Entity::SetMouse() {
	mouse=this;
	SetNolink(this,false);
}

void Entity::UnSetMouse() {
	mouse=NULL;
	SetNolink(NULL,true);
	if (!GetParent() && !GetPrev()) {
		if (entity_to_del) delete entity_to_del;
		entity_to_del=this;
	}
}

void Entity::SetNolink(Entity *m, bool n) {
	nolink = m;
	for (int i=0;i<GetChildCount();i++)
		if (GetChild(i)) GetChild(i)->SetNolink(m,true);
	if (n && GetNext()) GetNext()->SetNolink(m,true);
	
}

void Entity::EditSpecialLabel(int key) {
	if (key==WXK_LEFT) {
		if (edit_pos>0) SetEditPos(edit_pos-1);
	}
	else if (key==WXK_RIGHT) {
		if (edit_pos<int(label.size())) SetEditPos(edit_pos+1);
	}
	else if (key==WXK_HOME) {
		SetEditPos(0);
	}
	else if (key==WXK_END) {
		SetEditPos(label.size());
	}
}



static bool is_sep(char c) {
	if (c==' '||c==',') return true;
	if (c>='a'&&c<='z') return false;
	if (c>='A'&&c<='Z') return false;
	if (c>='0'&&c<='9') return false;
	if (c=='_'||c=='.') return false;
	if (c=='á'||c=='Á') return false;
	if (c=='é'||c=='É') return false;
	if (c=='í'||c=='Í') return false;
	if (c=='ó'||c=='Ó') return false;
	if (c=='ú'||c=='Ú') return false;
	if (c=='ü'||c=='Ü') return false;
	if (c=='ñ'||c=='Ñ') return false;
	return true;
}


// estilos para colourized (ver def de color_label_high en Global.cpp)
// 'a' identificadors
// 'b' numeros
// 'c' cadenas
// 'd' keywords
// 'e' comentarios
// 'f' operadores y otros

static set<string> keywords;

void load_keywords() {
	string ks = lang.GetFunctions()+" "+lang.GetKeywords()+" ";
	for(int i=0,lp=0,l=ks.size();i<l;i++) {
		if (ks[i]==' ') {
			if (i!=lp) keywords.insert(ks.substr(lp,i-lp));
			lp=i+1;
		}
	}
}

void to_lower(string &s){
	for(size_t i=0;i<s.size();i++) { 
		if (s[i]>='A'&&s[i]<='Z') s[i] = tolower(s[i]); 
		else if (s[i]=='Á') s[i] = 'á';
		else if (s[i]=='É') s[i] = 'é';
		else if (s[i]=='Í') s[i] = 'í';
		else if (s[i]=='Ó') s[i] = 'ó';
		else if (s[i]=='Ú') s[i] = 'ú';
		else if (s[i]=='Ü') s[i] = 'ü';
		else if (s[i]=='Ñ') s[i] = 'ñ';
	}
}

static char get_color(string s) {
	if (s[0]=='.'||(s[0]>='0'&&s[0]<='9')) return 'b';
	to_lower(s);
	if (keywords.count(s)) return 'd';
	return 'a';
}

void Entity::Colourize ( ) {
	static bool keywords_loaded = false;
	if (!keywords_loaded) { load_keywords(); keywords_loaded = true; }
	colourized.replace(0,colourized.size(),label.size(),'e');
	if (type==ET_COMENTARIO) return;
	bool first_word = type==ET_ASIGNAR||type==ET_PROCESO;
	int par_level = 0;
	for (int i = 0, l = label.size(), lp=0; i<=l; i++) {
		// cadenas
		if (i<l && (label[i]=='\''||label[i]=='\"')) {
			do {
				colourized[i++] = 'c';
			} while (i<l && label[i]!='\'' && label[i]!='\"');
			colourized[i] = 'c';
			lp=i+1; first_word = false;
		} else 
			// numeros y palabras claves	
			if (i==l || is_sep(label[i])) {
			if (i!=lp) {
				char c = get_color(label.substr(lp,i-lp));
				for(int j=lp;j<i;j++) 
					colourized[j] = c;
			}
			colourized[i] = 'f';
			// caso especial del op de asignacion
			static char flechita_s[] = {' ',flechita,' ','\0'};
			if (first_word && i<l) {
				if (label[i]=='['||label[i]=='(') {
					par_level++;
				} else if (label[i]==']'||label[i]==')') {
					par_level--;
				} else if (par_level==0) {
					if (label[i]==flechita||label[i]=='=') {
						colourized[i] = 'd';
					} else if (i+1<l && label[i]=='<' && label[i+1]=='-') {
						colourized[i] = colourized[i+1] = 'd'; ++i;
					} else if (i+1<l && label[i]==':' && label[i+1]=='=') {
						colourized[i] = colourized[i+1] = 'd'; ++i;
					}
					if (i==l||label[i]!=' ') first_word = false;
				}
			} else if (i<l && label[i]==';') {
				first_word = type==ET_ASIGNAR||type==ET_PROCESO;
			}
			lp = i+1;
		}
	}
}

static void beautify_label(ETYPE type, string &label, int &edit_pos) {
//	if (type==ET_COMENTARIO) return;
	if (type!=ET_ASIGNAR&&type!=ET_PROCESO) return;
	bool first_word = true;
	int par_level = 0;
	for (int i = 0, l = label.size(); i<l; i++) {
		// cadenas
		if (i<l && (label[i]=='\''||label[i]=='\"')) {
			do { i++; } while (i<l && label[i]!='\'' && label[i]!='\"');
		} else if (label[i]=='('||label[i]=='[') {
			par_level++;
		} else if (label[i]==']'||label[i]==')') {
			par_level--;
		} else if (label[i]==';') {
			if (i+1<l && label[i+1]!=' ') {
				label.insert(i+1," "); ++l;
				if (edit_pos>=i+1) ++edit_pos;
			}
			first_word = true;
		} else if (first_word && is_sep(label[i])) {
			if (label[i]==flechita||label[i]=='=') {
				if (i+1<l&&label[i+1]!=' ') {
					label.insert(i+1," "); ++l;
					if (edit_pos>=i+1) ++edit_pos;
				}
				if (i&&label[i-1]!=' ') {
					label.insert(i," "); ++l;
					if (edit_pos>=i-1) ++edit_pos;
				}
			} else if (i+1<l && label[i]=='<' && label[i+1]=='-') {
				if (i+2<l&&label[i+2]!=' ') {
					label.insert(i+2," "); ++l;
					if (edit_pos>=i+2) ++edit_pos;
				}
				label.replace(i,2,1,flechita); --l;
				if (edit_pos>i) --edit_pos;
				if (i&&label[i-1]!=' ') {
					label.insert(i," "); ++l;
					if (edit_pos>=i-1) ++edit_pos;
				}
			} else if (i+1<l && label[i]==':' && label[i+1]=='=') {
				if (i+2<l&&label[i+2]!=' ') {
					label.insert(i+2," "); ++l;
					if (edit_pos>=i+2) ++edit_pos;
				}
				if (i&&label[i-1]!=' ') {
					label.insert(i," "); ++l;
					if (edit_pos>=i-1) ++edit_pos;
				}
			}
			if (label[i]!=' ') first_word = false;
		}
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
	if (key==0) { int aux=0; beautify_label(type,label,aux); return; }
	if (key==180) { acento=true; return; }
	if (key=='\b') {
		if (edit_pos>0) {
			label.erase(edit_pos-1,1);
			int new_edit_pos = edit_pos-1;
			beautify_label(type,label,new_edit_pos);
			SetEditPos(new_edit_pos);
			SetLabel(label,true);
		}
		SetModified();
	} else if (key==127) {
		if (edit_pos<int(label.size())) {
			label.erase(edit_pos,1);
			int new_edit_pos = edit_pos;
			beautify_label(type,label,new_edit_pos);
			SetEditPos(new_edit_pos);
			SetLabel(label,true);
		}
	} else if (key==13 || key==27) {
		edit=NULL;
		if (enable_partial_text) SetLabel(label,true);
	} else {
		label.insert(edit_pos,string(1,key));
		int new_edit_pos = edit_pos+1;
		beautify_label(type,label,new_edit_pos);
		SetEditPos(new_edit_pos);
		SetLabel(label,true);
		SetModified();
	}
}


int Entity::IsLabelCropped ( ) {
	if (!enable_partial_text || this==edit) return 0;
	int max_len = max_label_len[type];
	if (int(label.size())>max_len) return max_len; else return 0;
}


void Entity::SetLabel(string _label, bool recalc) {
	if (_label!=label) SetModified();
	for (unsigned int i=0;i<label.size();i++) if (label[i]=='\'') label[i]='\"';
	label=_label; 
	Colourize();
	GetTextSize(t_w,t_h); w=t_w; h=t_h;
	int aux; GetTextSize(lpre,t_prew,aux); t_w+=t_prew;
	if (recalc) {
		if (nassi_shneiderman) { 
			if (start) Entity::CalculateAll(); // todo: recalcula demas, parche hasta que analice bien donde esta el problema con el otro metodo
		} else {
			Calculate();
			if (GetParent()) GetParent()->Calculate(true);
		}
	}
}

int Entity::CheckLinkChild(int x, int y) {
	if (nolink||!GetChildCount()||type==ET_SEGUN) return -1;
	if (type==ET_PARA||type==ET_REPETIR) { // h no tiene sentido porque la forma esta al final
		if (GetChild(0)!=mouse && d_x+child_dx[0]+selection_tolerance_x>x && d_x+child_dx[0]-selection_tolerance_x<x && y>d_y-flecha_in-2*flecha_h/3-selection_tolerance_y && y<d_y-flecha_in-2*flecha_h/3+selection_tolerance_y)
			return 0; else return -1;
	}
	for (int i=0;i<GetChildCount();i++) {
		if (GetChild(i)!=mouse && d_x+child_dx[i]+selection_tolerance_x>x && d_x+child_dx[i]-selection_tolerance_x<x && y>d_y-d_h-flecha_h-selection_tolerance_y && y<d_y-d_h-flecha_h+selection_tolerance_y)
			return i;
	}
	return -1;
}

int Entity::CheckLinkOpcion(int x, int y) {
	if (y<d_dy+GetChild(0)->y-GetChild(0)->h || y>d_dy+GetChild(0)->y+2*GetChild(0)->h) return -1;
	if (x<d_dx+GetChild(0)->x-GetChild(0)->w || x>d_dx+GetChild(GetChildCount()-1)->x+GetChild(GetChildCount()-1)->w) return -1;
	for (int i=0;i<GetChildCount();i++) {
		if (x<d_dx+GetChild(i)->x) {
			if (i==mouse->GetChildId()) return -1; 
			else return i;
		}
	}
	return -1;
	
}

bool Entity::CheckLinkNext(int x, int y) {
	if (type==ET_COMENTARIO && mouse->type!=ET_COMENTARIO && IsOutOfProcess()) return false;
	if (type==ET_OPCION || type==ET_AUX_PARA || (type==ET_PROCESO&&variante) || mouse==GetNext() || nolink) return false;
	return (x>d_x-d_w/2 && x<d_x+d_w/2 && y>d_y-d_bh-selection_tolerance_y && y<d_y-d_bh+selection_tolerance_y);
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
	if (nolink && mouse==nolink) { // si se esta moviendo como parte de un bloque, se esconde detras de ese bloque
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
	if (this==edit) { glScaled(.105,.15,1); } // el escalado molesta visualmente al editar el texto
	else glScaled((.105*d_w)/w,(.15*d_h)/h,1);
	begin_texto();
	glColor3fv(color_label_high[3]);
	for (unsigned int i=0;i<lpre.size();i++) {
		dibujar_caracter(lpre[i]);
	}
//	glColor3fv(edit==this?color_selection:(type==ET_PROCESO?color_arrow:(type==ET_COMENTARIO?color_comment:color_label)));
	int llen = label.size(), crop_len = IsLabelCropped();
	if (crop_len) llen=crop_len-3;
	int last_color = 'a'; bool syntax = type!=ET_COMENTARIO; 
	if (syntax) glColor3fv(color_label_high[last_color-'a']);
	for (int i=0;i<llen;i++) {
		if (syntax && colourized[i]!=last_color) 
			glColor3fv(color_label_high[(last_color=colourized[i])-'a']);
		dibujar_caracter(label[i]);
	}
	if (llen!=int(label.size()))
		for (unsigned int i=0;i<3;i++)
			dibujar_caracter('.');
	end_texto();
	glPopMatrix();
	// dibuja el cursor de edición si estamos editando justo este texto
	if (edit==this && mouse!=this && w>0) {
		blink++; if (blink==20) blink=0;
		if (blink<10) {
			glBegin(GL_LINES);
			int lz=label.size(); if (!lz) lz=1; // ojo estas dos lineas deben coincidir con las dos de EnsureCaretVisible
			lz= d_fx+t_dx-t_w/2+t_prew+(t_w-t_prew)*edit_pos*d_w/lz/w+(type==ET_OPCION?flecha_w/2:0);
			glVertex2i(lz,d_fy-h/2-t_h/2-margin/2+t_dy);
			glVertex2i(lz,d_fy-h/2+t_h/2+margin/2+t_dy);
			glEnd();
		}
	}
}

void Entity::EnsureCaretVisible() {
#ifndef _FOR_EXPORT
	if (!w) return;
	int lz = label.size(); if (!lz) lz=1; // ojo estas dos lineas deben coincidir con las dos de DrawText
	lz = d_fx+t_dx-t_w/2+t_prew+(t_w-t_prew)*edit_pos*d_w/lz/w+(type==ET_OPCION?flecha_w/2:0);
	// asegurarse de que el cursor se vea
	const int margin = 50;
	int max_x = (win_w-shapes_bar->GetWidth()-margin)/zoom, min_x = (shapes_bar->GetWidth()+margin)/zoom;
	if (lz>max_x) d_dx-=(lz-max_x); else if (lz<min_x) d_dx+=(min_x-lz);
#endif
}

void Entity::Draw(bool force) {
	if (!show_comments && type==ET_COMENTARIO) return;
	if (nassi_shneiderman) DrawNassiShne(force);
	else DrawClasico(force);
}

void Entity::Calculate(bool also_parent) { // devuelve el tamaño total del bloque
	int awl,awr,ah;
	Calculate(awl,awr,ah);
	if (also_parent && GetParent()) GetParent()->Calculate(true);
}

void Entity::MoveX(int dx) { // mueve al item y todos sus hijos en x
	x+=dx; fx+=dx;
	for (int i=0;i<GetChildCount();i++)
		if (GetChild(i)) GetChild(i)->MoveX(dx);
	if (GetNext()) GetNext()->MoveX(dx);
}

void Entity:: ResizeW(int aw, bool up) {
	if (!nolink || this==nolink) {
		int old=bwl+bwr;
		bwl+=(aw-old)/2;
		bwr+=(aw-old)/2;
		w+=(aw-old);
		int nc=GetChildCount(); if (type==ET_PARA) nc=1;
		for (int i=0;i<nc;i++)
			if (GetChild(i)) 
				GetChild(i)->ResizeW(GetChild(i)->bwl+GetChild(i)->bwr+(aw-old)/nc,false);
		if (type==ET_SI) {
			int dx=(aw-old)/4;
			if (GetChild(0)) GetChild(0)->MoveX(dx);
			if (GetChild(1)) GetChild(1)->MoveX(-dx);
			child_dx[0]+=dx; child_dx[1]-=dx;
		} else if (type==ET_SEGUN) {
			int dx=(aw-old)/2;
			int dx0=dx-dx/nc;
			for(int i=0;i<GetChildCount();i++) { 
				int dd=-dx0+2*dx*(i)/nc;
				GetChild(i)->MoveX(dd);
				child_dx[i]+=dd;
			}
		}
	}
	if (up && GetPrev()) GetPrev()->ResizeW(aw,true);
	if (!up && GetNext()) GetNext()->ResizeW(aw,false);
}

void Entity::Calculate(int &gwl, int &gwr, int &gh) { // calcula lo propio y manda a calcular al siguiente y a sus hijos, y acumula en gw,gh el tamaño de este item (para armar el tamaño del bloque)
	if (nassi_shneiderman) CalculateNassiShne();
	else CalculateClasico();
	// pasar a la siguiente entidad
	if (GetNext()) {
		GetNext()->x=x;
		GetNext()->y=y-bh;
		GetNext()->Calculate(gwl,gwr,gh);
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
	if (click) {
		if (type==ET_OPCION) {
			if (x>=d_fx-d_bwl && x<=d_fx-d_bwl+flecha_w && y<=d_fy && y>=d_fy-d_h) { // agregar una opción más
				GetParent()->InsertChild(GetChildId(),new Entity(ET_OPCION,""));
				GetParent()->GetChild(GetChildId()-1)->SetEdit();
				GetParent()->GetChild(GetChildId()-1)->CopyPos(this);
				return false;
			}
			if (GetChildId()==GetParent()->GetChildCount()-1) return false;
		} else if (type==ET_SELECTION) {
			int w = margin, xc = d_fx+d_w/2-(nassi_shneiderman?2*margin:0), yc = d_fy-(nassi_shneiderman?2*margin:0);
			if (x>xc-w&&x<xc+w&&y>yc-w&&y<yc+w) {
				Entity *aux = this;
				while(GetChild(0)) {
					Entity *child = this->GetChild(0);
					child->UnLink();
					aux->LinkNext(child);
					aux = child;
				}
				this->UnLink();
				entity_to_del = this;
				return false;
			}
		}
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

bool Entity::IsInside(int x0, int y0, int x1, int y1) {
	if (!edit_on || type==ET_PROCESO || type==ET_PROCESO) return false;
	return x0<d_fx+t_dx-d_w/2 && x1>d_fx+t_dx+d_w/2 && y0>d_fy && y1<d_fy-d_h;
}

#define _tabs "\t"

#define _endl_this inline_comments<<endl; inline_comments=""; {stringstream ss; ss<<line_num<<":1"; code2draw[ss.str()]=LineInfo(process,this);} line_num++;
#define _endl_prev inline_comments<<endl; inline_comments=""; {stringstream ss; ss<<line_num<<":1"; code2draw[ss.str()]=LineInfo(NULL,this);} line_num++;
#define _endl_none inline_comments<<endl; inline_comments=""; {stringstream ss; ss<<line_num<<":1"; code2draw[ss.str()]=LineInfo(NULL,NULL);} line_num++;

#define _fix(label,def) (label.size()?label:def)

void Entity::Print(ostream &out, string tab, Entity *process, int &line_num) {
	static string inline_comments; 
	bool add_tab=false;
	string old_label = label;
	if (type==ET_PROCESO) {
		add_tab=true;
		if (GetNext()) {
			if (label.find(flechita)!=string::npos) label.replace(label.find(flechita),1,"<-");
			out<<tab<<lpre<<_fix(label,"{sin_titulo}")<<_endl_this;
			if (GetNext()) GetNext()->Print(out,add_tab?tab+_tabs:tab,process,line_num);
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
		if (GetChild(0)) GetChild(0)->Print(out,tab+_tabs,process,line_num);
		out<<tab<<"FinMientras"<<_endl_prev;
	} else if (type==ET_REPETIR) {
		out<<tab<<"Repetir"<<_endl_prev;
		if (GetChild(0)) GetChild(0)->Print(out,tab+_tabs,process,line_num);
		out<<tab<<(variante?"Mientras Que ":"Hasta Que ")<<_fix(label,"{condicion}")<<_endl_this;
	} else if (type==ET_PARA) {
		if (variante) {
			out<<tab<<"Para Cada"<<_fix(label,"{variable}")<<" de "<<_fix(GetChild(2)->label,"{arreglo}")<<" Hacer"<<_endl_this;
		} else {
			bool has_paso=GetChild(2)->label!="1"&&GetChild(2)->label!="+1"&&GetChild(2)->label!="";
			out<<tab<<"Para "<<_fix(label,"{variable}")<<"<-"<<_fix(GetChild(1)->label,"{valor_inicial}")<<" Hasta "<<_fix(GetChild(3)->label,"{valor_final}")
				<<(has_paso?" Con Paso ":"")<<(has_paso?_fix(GetChild(2)->label,"{paso}"):"") <<" Hacer"<<_endl_this;
		}
		if (GetChild(0)) GetChild(0)->Print(out,tab+_tabs,process,line_num);
		out<<tab<<"FinPara"<<_endl_prev;
	} else if (type==ET_SELECTION) {
		GetChild(0)->Print(out,_tabs,process,line_num);
	} else if (type==ET_SEGUN) {
		out<<tab<<"Segun "<<_fix(label,"{expresion}")<<" Hacer"<<_endl_this;
		for(int i=0;i<GetChildCount()-1;i++) { 
			GetChild(i)->Print(out,tab+_tabs,process,line_num);
		}
		if (GetChild(GetChildCount()-1)->GetChild(0)) // de otro modo
			GetChild(GetChildCount()-1)->Print(out,tab+_tabs,process,line_num);
		out<<tab<<"FinSegun"<<_endl_prev;
	} else if (type==ET_OPCION) {
		add_tab=true;
		out<<tab<<_fix(label,"{expresion}")<<":"<<_endl_this;
		if (GetChild(0)) GetChild(0)->Print(out,tab+_tabs,process,line_num);
	} else if (type==ET_SI) {
		out<<tab<<"Si "<<_fix(label,"{condicion}")<<" Entonces"<<_endl_this;
		if (GetChild(1)) { GetChild(1)->Print(out,tab+_tabs,process,line_num); }
		if (GetChild(0)) { out<<tab<<"SiNo"<<_endl_prev; }
		if (GetChild(0)) { GetChild(0)->Print(out,tab+_tabs,process,line_num); }
		out<<tab<<"FinSi"<<_endl_prev;
	} else if (type==ET_ASIGNAR) {
		while (label.find(flechita)!=string::npos) label.replace(label.find(flechita),1,"<-");
		if (lang[LS_FORCE_SEMICOLON] && label[label.size()-1]==';') label=label.erase(label.size()-1);
		if (label.size()) { out<<tab<<label<<(lang[LS_FORCE_SEMICOLON]?";":"")<<_endl_this; }
	} else if (type==ET_COMENTARIO) {
		string pre="// "; if (!label.empty() && label[0]=='/') pre="//";
		if (variante) {
			if (inline_comments=="") inline_comments=string(" ")+pre; else inline_comments+=" - ";
			inline_comments+=label;
		} else {
			string prev_ilc = inline_comments;
			inline_comments = "";
			out<<tab<<pre<<label<<_endl_this;
			inline_comments = prev_ilc;
		}
	}
	label=old_label;
	if (GetNext()) GetNext()->Print(out,add_tab?tab+_tabs:tab,process,line_num);
}

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
	while(top->GetPrev()) top = top->GetPrev();
	return top;
}

void Entity::CalculateAll (bool also_text_size) {
	if (also_text_size) {
		Entity::AllIterator it = Entity::AllBegin();
		while (it!=Entity::AllEnd()) {
			it->SetLabels();
			++it;
		}
	}
	start->GetTopEntity()->Calculate();
}

Entity *Entity::GetNextNoComment() {
	Entity *e_aux = GetNext();
	while (e_aux && e_aux->type==ET_COMENTARIO) e_aux = e_aux->GetNext();
	return e_aux;
}

bool Entity::IsOutOfProcess(Entity *next_no_commnet) {
//	return !next_no_commnet || // fin proceso
//		(next_no_commnet->type==ET_PROCESO&&!next_no_commnet->variante); // proceso
	return next_no_commnet && (next_no_commnet->type==ET_PROCESO&&!next_no_commnet->variante); // proceso
}

bool Entity::IsOutOfProcess() {
	return IsOutOfProcess(GetNextNoComment());
}

// habria que sacar esto y reemplazarlo por recursión en el cliente
Entity *Entity::NextEntity(Entity *aux) {
	static vector<int> pila_nc;
	static vector<Entity*> pila_e;
	// si tiene hijos, se empieza por los hijos
	if (aux->GetChildCount()) {
		for(int i=0;i<aux->GetChildCount();i++) { 
			if (aux->GetChild(i)) {
				pila_nc.push_back(i);
				pila_e.push_back(aux);
				return aux->GetChild(i);
			}
		}
	}
	while(true) {
		// si no tiene hijos se pasa a la siguiente
		if (aux->GetNext()) return aux->GetNext();
		// si no hay siguiente, se sube
		if (pila_e.empty()) return NULL; // si no se puede subir estamos en "Fin Proceso"
		int last=++pila_nc.back();
		aux=pila_e.back();
		if (aux->GetChildCount()!=last) {
			if (aux->GetChild(last)) return aux->GetChild(last);
			else pila_nc.back()=last;
		} else {
			pila_nc.pop_back();
			pila_e.pop_back();
		}
	}
}

void Entity::SetLabels() {
	switch(type) {
		case ET_COMENTARIO:
			lpre=nassi_shneiderman?"// ":"";
			break;
		case ET_LEER:
			lpre=nassi_shneiderman?"Leer ":"";
		case ET_PROCESO: // se fija desde afuera, nunca cambia
//			lpre=variante?"Fin Proceso":"Proceso ";
			break;
		case ET_ESCRIBIR:
			lpre=nassi_shneiderman?"Escribir ":"";
		case ET_ASIGNAR:
			break;
		case ET_SI:
			break;
		case ET_SEGUN:
			break;
		case ET_OPCION:
			break;
		case ET_PARA:
			lpre=nassi_shneiderman?(variante?"Para Cada":"Para "):"";
			break;
		case ET_MIENTRAS:
			lpre=nassi_shneiderman?"Mientras ":"";
			break;
		case ET_REPETIR:
			lpre=nassi_shneiderman?(variante?"Mientras Que ":"Hasta Que "):"";
			break;
		case ET_AUX_PARA:
			if (!GetParent()) return; // en el ctor todavía no está linkeado al padre, y el lpre depende de la posicion en el padre
			if (nassi_shneiderman) {
				if (GetParent()->GetChild(1)==this) lpre=" Desde ";
				else if (GetParent()->GetChild(2)==this) lpre=" Con Paso ";
				else if (GetParent()->GetChild(3)==this) lpre=" Hasta ";
			} else {
				lpre="";
			}
			break;
	case ET_COUNT: return;
	}
	SetLabel(label);
}

void Entity::SetEditPos (int pos, bool ensure_caret_visibility) {
	edit_pos=pos;
	if (ensure_caret_visibility) EnsureCaretVisible();
}

void Entity::OnLinkingEvent (LnkEvtType t, int i) {
	switch(t) {
	case EntityLinkingBase::EVT_UNLINK:
		if (type==ET_SEGUN) RemoveChild(i);
		break;
	case EntityLinkingBase::EVT_SETCHILDCOUNT:
		child_bh.Resize(i,0); child_dx.Resize(i,0);
		break;
	case EntityLinkingBase::EVT_INSERTCHILD:
		child_bh.Insert(i,0); child_dx.Insert(i,0);
		break;
	case EntityLinkingBase::EVT_REMOVECHILD:
		child_bh.Remove(i); child_dx.Remove(i);
		break;
	}
}


