#include "GLstuff.h"
#include "Draw.h"
#include "Global.h"
#include "Entity.h"
#include "Events.h"
#include <cstring>
#include "Textures.h"
#include <wx/cursor.h>
#include "MainWindow.h"
#include "Canvas.h"
#include "Text.h"
using namespace std;

const int cant_shapes_in_bar = 9;

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




static void DrawTrash() {
	glLineWidth(menu_line_width);
	glColor3fv(color_menu_back);
	glBegin(GL_QUADS);
		// trash
		glVertex2i(0,trash_size);
		glVertex2i(trash_size,trash_size);
		glVertex2i(trash_size,0);
		glVertex2i(0,0);
	glEnd();
	glColor3fv(color_menu);
	glBegin(GL_LINES);
		glVertex2i(0,trash_size); glVertex2i(trash_size,trash_size);
		glVertex2i(trash_size,0); glVertex2i(trash_size,trash_size);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	texture_trash.Select();
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
		if (trash) {
			glTexCoord2f(0*texture_trash.max_s,.77*texture_trash.max_t); glVertex2i(0,trash_size);
			glTexCoord2f(1*texture_trash.max_s,.77*texture_trash.max_t); glVertex2i(trash_size,trash_size);
			glTexCoord2f(1*texture_trash.max_s,0*texture_trash.max_t); glVertex2i(trash_size,0);
			glTexCoord2f(0*texture_trash.max_s,0*texture_trash.max_t); glVertex2i(0,0);
		} else {
			glTexCoord2f(0*texture_trash.max_s,1*texture_trash.max_t); glVertex2i(0,trash_size);
			glTexCoord2f(.32*texture_trash.max_s,1*texture_trash.max_t); glVertex2i(trash_size,trash_size);
			glTexCoord2f(.32*texture_trash.max_s,.77*texture_trash.max_t); glVertex2i(trash_size,0);
			glTexCoord2f(0*texture_trash.max_s,.77*texture_trash.max_t); glVertex2i(0,0);
		}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

static void DrawMenuAndShapeBar() {
	glLineWidth(menu_line_width);
	glColor3fv(color_menu_back);
	// shapebar
	glBegin(GL_QUADS);
		glVertex2i(win_w-shapebar_size,0);
		glVertex2i(win_w-shapebar_size,win_h);
		glVertex2i(win_w,win_h);
		glVertex2i(win_w,0);
	glEnd();
	// shapebar
	glColor3fv(color_menu);
	glBegin(GL_LINES);
		glVertex2i(win_w-shapebar_size,0);
		glVertex2i(win_w-shapebar_size,win_h);
	glEnd();
	if (shapebar) {
		
		float sh=float(win_h)/cant_shapes_in_bar;
		// resaltado(fondo) de la entidad seleccionada
		if(shapebar_sel>0) {
			glColor3fv(color_menu_sel);
			glBegin(GL_QUADS);
				glVertex2f(win_w,              win_h-sh*(shapebar_sel-1)); 
				glVertex2f(win_w-shapebar_size,win_h-sh*(shapebar_sel-1)); 
				glVertex2f(win_w-shapebar_size,win_h-sh*(shapebar_sel-0)); 
				glVertex2f(win_w,              win_h-sh*(shapebar_sel-0)); 
			glEnd();
		}
		
		// imagenes de cada entidad
		glEnable(GL_TEXTURE_2D);
		texture_shapes.Select();
		glColor3f(1,1,1);
		glBegin(GL_QUADS);
			float dy=sh, th0=0, th1=1.0/float(cant_shapes_in_bar), dth=1.0/float(cant_shapes_in_bar);
			float x0=win_w-shapebar_size, x1=win_w, y0=0, y1=float(win_h)/float(cant_shapes_in_bar); 
			float ratio=float(shapebar_size)/float(win_h)/float(cant_shapes_in_bar);
			if (ratio>texture_shapes.r) {
				float dx=(x1-x0)*(1-texture_shapes.r/ratio)/2;
				x0+=dx; x1-=dx;
			} else {
				float dy=(y1-y0)*(1-ratio/texture_shapes.r)/2;
				y0+=dy; y1-=dy;
			}
			for(int i=0;i<cant_shapes_in_bar;i++) {
				glTexCoord2f(0*texture_shapes.max_s,th0*texture_shapes.max_t); glVertex2f(x0,y0);
				glTexCoord2f(1*texture_shapes.max_s,th0*texture_shapes.max_t); glVertex2f(x1,y0);
				glTexCoord2f(1*texture_shapes.max_s,th1*texture_shapes.max_t); glVertex2f(x1,y1);
				glTexCoord2f(0*texture_shapes.max_s,th1*texture_shapes.max_t); glVertex2f(x0,y1);
				y0+=dy; y1+=dy; th0+=dth; th1+=dth;
			}
		glEnd();
		glDisable(GL_TEXTURE_2D);
		// lineas que separan las entidades
		glBegin(GL_LINES);
			glColor3fv(color_menu);
			y0=win_h-dy;
			for(int i=0;i<cant_shapes_in_bar-1;i++,y0-=dy) {
				glVertex2f(win_w-shapebar_size,y0);
				glVertex2f(win_w,y0);
			}
		glEnd();
	} else { 
		glEnable(GL_TEXTURE_2D);
		texture_commands.Select();
		glColor3f(1,1,1);
		glBegin(GL_QUADS);
			glTexCoord2f(0*texture_commands.max_s,0*texture_commands.max_t); glVertex2i(win_w-shapebar_size,(win_h-texture_commands.h)/2);
			glTexCoord2f(1*texture_commands.max_s,0*texture_commands.max_t); glVertex2i(win_w-shapebar_size+texture_commands.w,(win_h-texture_commands.h)/2);
			glTexCoord2f(1*texture_commands.max_s,1*texture_commands.max_t); glVertex2i(win_w-shapebar_size+texture_commands.w,(win_h+texture_commands.h)/2);
			glTexCoord2f(0*texture_commands.max_s,1*texture_commands.max_t); glVertex2i(win_w-shapebar_size,(win_h+texture_commands.h)/2);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	
	if (menu_sel!=MO_NONE) {
		switch(menu_sel) {
		case MO_ZOOM_EXTEND: SetStatus(color_selection,"Ajustar el zoom para visualizar todo el diagrama."); break;
		case MO_TOGGLE_FULLSCREEN: SetStatus(color_selection,"Alternar entre modo ventana y pantalla completa."); break;
		case MO_TOGGLE_COLORS: SetStatus(color_selection,"Alternar entre modo ventana y pantalla completa."); break;
		case MO_CROP_LABELS: SetStatus(color_selection,"Acortar textos muy largos en etiquetas."); break;
		case MO_TOGGLE_COMMENTS: SetStatus(color_selection,"Mostrar/Ocultar comentarios."); break;
		case MO_CHANGE_STYLE: SetStatus(color_selection,"Cambiar el tipo de diagrama (clásico, o Nassi-Shneiderman)."); break;
		case MO_FUNCTIONS: 
			SetStatus(color_selection,
					  lang[LS_PREFER_FUNCION] ? "Permite elegir cual función editar, crear una nueva o eliminar una existente."
						: (lang[LS_PREFER_ALGORITMO] ? "Permite elegir cual subalgoritmo editar, crear uno nuevo o eliminar uno existente."
							: "Permite elegir cual subproceso editar, crear uno nuevo o eliminar uno existente.") ); 
			break;
		case MO_RUN: SetStatus(color_selection,"Ejecuta el algoritmo en la terminal de PSeInt."); break;
		case MO_DEBUG: SetStatus(color_selection,"Ejecuta el algoritmo paso a paso marcando los pasos sobre el diagrama."); break;
		case MO_EXPORT: SetStatus(color_selection,"Permite guardar el diagrama como imagen."); break;
		case MO_CLOSE: SetStatus(color_selection,"Cierra el editor, preguntando antes si se deben aplicar los cambios en el pseudocodigo"); break;
		case MO_HELP: SetStatus(color_selection,"Muestra una ventana de ayuda que explica como utilizar este editor y cuales son sus atajos de teclado."); break;
		default:;
		}
	}
	
	// shapebar
	if (shapebar) {
		switch(shapebar_sel) {
		case 1: 
			SetStatus(color_selection,"Comentario (texto libre que el interprete ignora)"); break;
		case 2: 
			if (canvas->GetModifiers()&MODIFIER_SHIFT) 
				SetStatus(color_selection,"Invocacion de un subproceso");
			else
				SetStatus(color_selection,"Asignacion/Dimension/Definicion"); 
			break;
		case 3: SetStatus(color_selection,"Escribir (instruccion para generar salidas)"); break;
		case 4: SetStatus(color_selection,"Leer (instruccion para obtener entradas)"); break;
		case 5: SetStatus(color_selection,"Si-Entonces (estructura condicional simple)"); break;
		case 6: SetStatus(color_selection,"Segun (estructura de seleccion multiple)"); break;
		case 7: SetStatus(color_selection,"Mientras (estructura repetitiva)"); break;
		case 8: 
			if (canvas->GetModifiers()&MODIFIER_SHIFT)
				SetStatus(color_selection,"Repetir-Mientras que (estructura repetitiva)"); 
			else
				SetStatus(color_selection,"Repetir-Hasta que (estructura repetitiva)");
			break;
		case 9: 
			if (canvas->GetModifiers()&MODIFIER_SHIFT)
				SetStatus(color_selection,"Para Cada (estructura repetitiva)"); 
			else
				SetStatus(color_selection,"Para (estructura repetitiva)"); 
			break;
		default:;
		}
	} else if (trash) DrawTextRaster(color_selection,10+trash_size_max,10,"Eliminar");
}

static void DrawChooseProcess() {
	zoom=1;
	interpolate_good(choose_process_d_base,choose_process_base);
	interpolate_good(choose_process_d_delta,choose_process_delta);
	
	glLineWidth(1);
	glClear(GL_COLOR_BUFFER_BIT); glLineWidth(menu_line_width);
	DrawTextRaster(color_menu,10,win_h-25,
				   (lang[LS_PREFER_FUNCION]
						? (lang[LS_PREFER_ALGORITMO]
							? "Seleccione un algoritmo/funcion para visualizar:"
							: "Seleccione un proceso/funcion para visualizar:")
						: (lang[LS_PREFER_ALGORITMO]
						   ? "Seleccione un proceso/subproceso para visualizar:"
						   : "Seleccione un algoritmo/subalgoritmo para visualizar:") ) );
	
	int base=win_h-choose_process_d_base, delta=choose_process_d_delta;
	glColor3fv(color_menu); 
	glBegin(GL_LINES); 
		glVertex2i(0,base); glVertex2i(win_w,base);
	glEnd();
	for(int i=0;i<=int(procesos.size()-(edit_on?0:1));i++) {
		
		if (choose_process_sel==i) {
			glColor3fv(color_shape[ET_PROCESO]);
			glBegin(GL_QUADS);
				glVertex2i(0,base);
				glVertex2i(win_w,base);
				glVertex2i(win_w,base-delta);
				glVertex2i(0,base-delta);
			glEnd();
		}
		base-=delta;
		
		if (i==int(procesos.size())) {
			DrawTextRaster(choose_process_sel==i?color_selection:color_menu,20,base+10,
						   lang[LS_PREFER_FUNCION]?"Agregar Nueva Funcion"
								:(lang[LS_PREFER_ALGORITMO]?"Agregar Nuevo SubAlgoritmo"
									:"Agregar Nuevo SubProceso") );
		} else {
			
			string &s=procesos[i]->label;
			int l=s.size(),p=0; size_t f=s.find('<');
			if (f==string::npos) f=s.find('='); else f++;
			if (f==string::npos) f=0; else f++;
			int t=f; while (t<l && s[t]!=' ' && s[t]!='(') t++;
			
			glColor3fv(choose_process_sel==i?color_selection:color_menu);
			glPushMatrix();
			int px=20, py=base+10;
			if (choose_process_state==3 && choose_process_sel==i) glTranslated(cur_x+(px-m_x0),cur_y+(py-m_y0),0);
			else glTranslated(px,py,0);
			glScaled(.08,.12,.1);
			
			begin_texto();
				string &sp=procesos[i]->lpre;
				p=0; l=sp.size();
				while (p<l)
					dibujar_caracter(sp[p++]);
				
				p=0; l=s.size();
				while (p<l) {
					if (p==int(f)) glLineWidth(2);
					if (p==t) glLineWidth(1);
					dibujar_caracter(s[p++],true);
				}
			end_texto();
			glPopMatrix();		
		}
		
		glLineWidth(1);
		glColor3fv(color_menu); 
		glBegin(GL_LINES); 
			glVertex2i(0,base); glVertex2i(win_w,base);
		glEnd();
	}
	glEnd();
	
	DrawTrash();
	
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

void display_cb() {
	mouse_cursor=Z_CURSOR_CROSSHAIR;
	if (choose_process_state) { DrawChooseProcess(); return; }
	status_color=NULL;
	if (entity_to_del) delete entity_to_del;
	glClear(GL_COLOR_BUFFER_BIT);
	// dibujar el diagrama
	float mx=cur_x/zoom, my=cur_y/zoom;
	Entity *aux=start->GetTopEntity();
	Entity *my_start=aux;
	bool found=false;
	line_width_flechas=2*d_zoom<1?1:int(d_zoom*2);
	line_width_bordes=1*d_zoom<1?1:int(d_zoom*1);
	glLineWidth(line_width_flechas);
	glPushMatrix();
	glScalef(d_zoom,d_zoom,1);
	do {
		if (!found && mouse && mouse->type!=ET_OPCION && (cur_x-mouse_link_x)*(cur_x-mouse_link_x)+(cur_y-mouse_link_y)*(cur_y-mouse_link_y)>mouse_link_delta) {
			if (aux!=mouse && aux->CheckLinkNext(cur_x,cur_y)) {
				mouse_link_x=cur_x; mouse_link_y=cur_y;
				mouse->UnLink();
				aux->LinkNext(mouse);
				Entity::CalculateAll();
				found=true;
			} else if (aux->GetChildCount()) {
				int i=aux->CheckLinkChild(cur_x,cur_y);
				if (i!=-1) {
					mouse_link_x=cur_x; mouse_link_y=cur_y;
					mouse->UnLink();
					aux->LinkChild(i,mouse);
					Entity::CalculateAll();
					found=true;
				}
			}
		}
		if (/*!Entity::nassi_shneiderman &&*/ edit_on && (mouse?(aux==mouse):aux->CheckMouse(mx,my,false))) {
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
	if (mouse && mouse->type==ET_OPCION) {
		int i=mouse->GetParent()->CheckLinkOpcion(cur_x,cur_y);
		if (i!=-1) {
			mouse->GetParent()->Calculate();
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
	if (edit_on) { DrawMenuAndShapeBar(); DrawTrash(); }
	if (edit && !mouse && !status_color) {
		switch (edit->type) {
		case ET_LEER: SetStatus(color_selection,"? Lista de variables a leer, separadas por coma."); break;
		case ET_PROCESO: SetStatus(color_selection,edit->lpre=="Proceso "||edit->lpre=="Algoritmo "?"? Nombre del proceso.":"? Prototipo del subproceso."); break;
		case ET_COMENTARIO: SetStatus(color_selection,"? Texto libre, sera ignorado por el interprete."); break;
		case ET_ESCRIBIR: SetStatus(color_selection,"? Lista de expresiones a mostrar, separadas por comas."); break;
		case ET_SI: SetStatus(color_selection,"? Expresion logica."); break;
		case ET_SEGUN: SetStatus(color_selection,"? Expresion de control para la estructura."); break;
		case ET_OPCION: SetStatus(color_selection,"? Posible valor para la expresion de control."); break;
		case ET_PARA: 
			if (edit->variante)	SetStatus(color_selection,"? Identificador temporal para el elemento del vector/matriz.");
			else SetStatus(color_selection,"? Identificador de la variable de control (contador)."); 
			break;
		case ET_MIENTRAS: SetStatus(color_selection,"? Expresion de control (logica)."); break;
		case ET_REPETIR: SetStatus(color_selection,"? Expresion de control (logica)."); break;
		case ET_ASIGNAR: SetStatus(color_selection,"? Asignacion o instruccion secuencial."); break;
		case ET_AUX_PARA: 
			if (edit->GetParent()->variante)	SetStatus(color_selection,"? Identificador del vector/matriz a recorrer.");
			else SetStatus(color_selection,
						   edit->GetParent()->GetChild(1)==edit
								? "? Valor inicial para el contador."
								: ( edit->GetParent()->GetChild(2)==edit
									? "? Paso, incremento del contador por cada iteracion."
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
	if (trash) mouse_cursor=Z_CURSOR_DESTROY;
	else if (mouse) mouse_cursor=Z_CURSOR_NONE;
	else if (shapebar||choose_process_sel) mouse_cursor=Z_CURSOR_INHERIT;
}

