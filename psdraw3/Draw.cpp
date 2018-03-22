#include <cstring>
#include <wx/cursor.h>
#include "MainWindow.h"
#include "GLstuff.h"
#include "ProcessSelector.h"
#include "Draw.h"
#include "Global.h"
#include "Entity.h"
#include "Events.h"
#include "Textures.h"
#include "Canvas.h"
#include "Text.h"
#include "ShapesBar.h"
#include "ToolBar.h"
#include "Trash.h"
#include <iostream>
using namespace std;

class RaiiColorChanger {
	float *p[5], v[5]; int n;
public:
	RaiiColorChanger() : n(0) {}
	void Change(float &c, float x) { p[n]=&c; v[n]=c; c=x; n++; }
	~RaiiColorChanger() { for(int i=0;i<n;i++) *(p[i])=v[i]; }
};


CURSORES mouse_cursor;

#define mouse_link_delta 250
static int mouse_link_x=0,mouse_link_y=0; 

static string status_text; // texto para la barra de estado
static const float *status_color; // color del texto para la barra de estado

void SetStatus(const float *color, const string &text) {
	status_color=color;
	status_text=text;
}


// dibuja una marca al lado de la entidad que indica que tiene un error de sintaxis
void draw_error_mark(Entity *e, float delta) {
	float x=e->d_fx-e->w/2+delta;//-2*delta;
	float y=e->d_fy-e->h/2+2*delta;
	glColor3f(1,.85,.85);
	glBegin(GL_QUADS);
		glVertex2f(x+delta,y);
		glVertex2f(x,y-2*delta);
		glVertex2f(x-2*delta,y-2*delta);
		glVertex2f(x,y+2*delta);
		glVertex2f(x-2*delta,y-2*delta);
		glVertex2f(x-3*delta,y);
		glVertex2f(x-2*delta,y+2*delta);
		glVertex2f(x,y+2*delta);
	glEnd();
	glColor3fv(color_error);
	glBegin(GL_LINE_LOOP);
		glVertex2f(x+delta,y);
		glVertex2f(x,y-2*delta);
		glVertex2f(x-2*delta,y-2*delta);
		glVertex2f(x-3*delta,y);
		glVertex2f(x-2*delta,y+2*delta);
		glVertex2f(x,y+2*delta);
	glEnd();
	glBegin(GL_LINES);
		glVertex2f(x,y-delta);
		glVertex2f(x-2*delta,y+delta);
		glVertex2f(x,y+delta);
		glVertex2f(x-2*delta,y-delta);
	glEnd();
}

void draw_error_mark_simple(Entity *e, float delta) {
	float x=e->d_fx-e->w/2-delta;
	float y=e->d_fy-e->h/2+2*delta;
	glColor3fv(color_error);
	glBegin(GL_LINES);
		glVertex2f(x,y-delta);
		glVertex2f(x-2*delta,y+delta);
		glVertex2f(x,y+delta);
		glVertex2f(x-2*delta,y-delta);
	glEnd();
}

// dibuja una flecha al lado de la entidad que indica que es el punto actual de la ejecucion paso a paso
void draw_debug_arrow(Entity *e, float delta) {
	float x=e->d_fx-e->w/2-2*delta;
	float y=e->d_fy-e->h/2;
	glColor3f(0,1,0);
	glBegin(GL_TRIANGLES);
		glVertex2f(x,y-2*delta);
		glVertex2f(x,y+2*delta);
		glVertex2f(x+delta,y);
	glEnd();
	glBegin(GL_QUADS);
		glVertex2f(x-delta,y-delta);
		glVertex2f(x,y-delta);
		glVertex2f(x,y+delta);
		glVertex2f(x-delta,y+delta);
	glEnd();
	glColor3f(0,.5,0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(x-delta,y-delta);
		glVertex2f(x,y-delta);
		glVertex2f(x,y-2*delta);
		glVertex2f(x+delta,y);
		glVertex2f(x,y+2*delta);
		glVertex2f(x,y+delta);
		glVertex2f(x-delta,y+delta);
	glEnd();
}

void MoveToNext(Entity *mouse, Entity *aux, bool calculate = true){
	Entity *mouse_next = mouse->GetNext();
	mouse->UnLink();
	aux->LinkNext(mouse);
	if (mouse_next && mouse_next->nolink) {
		MoveToNext(mouse_next,mouse,false);
	}
	if (calculate) Entity::CalculateAll();
}

void MoveToChild(Entity *mouse, Entity *aux, int i){
	Entity *mouse_next = mouse->GetNext();
	mouse->UnLink();
	aux->LinkChild(i,mouse);
	if (mouse_next && mouse_next->nolink==mouse) {
		MoveToNext(mouse,mouse_next,false);
	}
	Entity::CalculateAll();
}

void display_cb() {
	mouse_cursor=Z_CURSOR_CROSSHAIR;
	status_color=NULL;
	if (entity_to_del) delete entity_to_del;
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (process_selector->IsActive()) { process_selector->Draw(); return; }
	
	// dibujar el diagrama
	float mx=cur_x/zoom, my=cur_y/zoom;
	Entity *aux = start->GetTopEntity();
	Entity *my_start=aux;
	bool found=false;
	line_width_flechas=2*d_zoom<1?1:int(d_zoom*2);
	line_width_bordes=1*d_zoom<1?1:int(d_zoom*1);
	glLineWidth(line_width_flechas);
	glPushMatrix();
	glScalef(d_zoom,d_zoom,1);
	do {
		if (!found && mouse && mouse->type!=ET_OPCION && (cur_x-mouse_link_x)*(cur_x-mouse_link_x)+(cur_y-mouse_link_y)*(cur_y-mouse_link_y)>mouse_link_delta) {
			if (aux->CheckLinkNext(cur_x,cur_y) && !mouse->Contains(aux)) {
				mouse_link_x = cur_x; mouse_link_y = cur_y;
				MoveToNext(mouse,aux);
				found=true;
			} else if (aux->GetChildCount()) {
				int i=aux->CheckLinkChild(cur_x,cur_y);
				if (i!=-1) {
					mouse_link_x = cur_x; mouse_link_y = cur_y;
					MoveToChild(mouse,aux,i);
					found=true;
				}
			}
		}
		if (edit_on && (mouse?(aux==mouse):aux->CheckMouse(mx,my,false))) {
			RaiiColorChanger rcc;
			rcc.Change(color_shape[Entity::shape_colors?aux->type:ET_COUNT][2],.75); 
			rcc.Change(color_arrow[1],.5); rcc.Change(color_arrow[2],.5); // rcc.Change(color_arrow[0],1);
			line_width_bordes*=2;
			aux->Draw(aux->type==ET_OPCION);
			line_width_bordes/=2;
			if (aux->error.size()) SetStatus(color_error,aux->error);
			else if (!mouse && aux->IsLabelCropped()) SetStatus(color_label_high[0],aux->label);
		} else if (debugging && debug_current==aux) {
			RaiiColorChanger rcc;
			line_width_bordes*=2;
			if (!Entity::nassi_shneiderman) {
				rcc.Change(color_shape[Entity::shape_colors?aux->type:ET_COUNT][2],.65); rcc.Change(color_arrow[1],.4);
				rcc.Change(color_arrow[2],.4); // rcc.Change(color_arrow[0],1);
			}
			aux->Draw();
			line_width_bordes/=2;
			draw_debug_arrow(debug_current,5);
		} else {
			aux->Draw();
		}
		if (!aux->error.empty()) draw_error_mark/*_simple*/(aux,4);
		if (!mouse && edit==aux && aux->CheckMouse(mx,my,false)) mouse_cursor=Z_CURSOR_TEXT;
		aux = Entity::NextEntity(aux);
	} while (aux);
	if (mouse && !mouse->GetPrev() && !mouse->GetParent()) 
		mouse->Draw(); // cuando recien salen de la shapebar no esta linkeadas al algoritmo, no los toma la recorrida anterior
	if (mouse && mouse->type==ET_OPCION) {
		Entity *segun = mouse->GetParent();
		int new_id = segun->CheckLinkOpcion(cur_x,cur_y);
		if (new_id!=-1) {
			int old_id = mouse->GetChildId();
			segun->RemoveChild(old_id,false);
			if (old_id<new_id) --new_id;
			segun->InsertChild(new_id,mouse);
			segun->Calculate();
		}
	}
	
	if (selecting_zoom||selecting_entities) {
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
	shapes_bar->Draw(); 
	toolbar->Draw(); 
	trash->Draw();
	if (edit && !mouse && !status_color) {
		switch (edit->type) {
		case ET_LEER: SetStatus(color_selection,"? Lista de variables a leer, separadas por coma."); break;
		case ET_PROCESO: SetStatus(color_selection,(edit->lpre=="Proceso "||edit->lpre=="Algoritmo ")?"? Nombre del proceso.":"? Prototipo del subproceso."); break;
		case ET_COMENTARIO: SetStatus(color_selection,"? Texto libre, será ignorado por el interprete."); break;
		case ET_ESCRIBIR: SetStatus(color_selection,"? Lista de expresiones a mostrar, separadas por comas."); break;
		case ET_SI: SetStatus(color_selection,"? Expresión lógica."); break;
		case ET_SEGUN: SetStatus(color_selection,"? Expresión de control para la estructura."); break;
		case ET_OPCION: SetStatus(color_selection,"? Posible valor para la expresión de control."); break;
		case ET_PARA: 
			if (edit->variante)	SetStatus(color_selection,"? Identificador temporal para el elemento del vector/matriz.");
			else SetStatus(color_selection,"? Identificador de la variable de control (contador)."); 
			break;
		case ET_MIENTRAS: SetStatus(color_selection,"? Expresión de control (lógica)."); break;
		case ET_REPETIR: SetStatus(color_selection,"? Expresión de control (lógica)."); break;
		case ET_ASIGNAR: SetStatus(color_selection,"? Asignación o instruccion secuencial."); break;
		case ET_AUX_PARA: 
			if (edit->GetParent()->variante)	SetStatus(color_selection,"? Identificador del vector/matriz a recorrer.");
			else SetStatus(color_selection,
						   edit->GetParent()->GetChild(1)==edit
								? "? Valor inicial para el contador."
								: ( edit->GetParent()->GetChild(2)==edit
									? "? Paso, incremento del contador por cada iteración."
								    : "? Valor final para el contador." ) ); 
			break;
		default:;
		}
	}
	// dibujar la seleccion para que quede delante de todo
	if (mouse && !trash) {
		glLineWidth(zoom*2);
		glPushMatrix();
		glScalef(d_zoom,d_zoom,1);
		mouse->Draw();
		glPopMatrix();
	}
	// barra de estado
	if (status_color) {
		glColor3fv(color_back);
		int w=status_text.size()*9,bh=10,bw=10,h=15;
		bw-=3; w+=6; h+=6; bh-=6;
		glBegin(GL_QUADS);
			glVertex2i(bw,bh);
			glVertex2i(bw,bh+h);
			glVertex2i(bw+w,bh+h);
			glVertex2i(bw+w,bh);
		glEnd();
		DrawTextRaster(status_color,10,10,status_text.c_str());
	}
	if (mouse) mouse_cursor=Z_CURSOR_NONE;
}

