#include <cstddef>
#include "ShapesBar.h"
#include "Canvas.h"
#include "Entity.h"
#include "Global.h"
#include "Events.h"
#include "Draw.h"
#include "Text.h"

//#define shapebar_size_min 25
//#define shapebar_size_max 150
#define no_selection 0
#define cant_shapes_in_bar 9

ShapesBar *shapes_bar = NULL;

ShapesBar::ShapesBar() 
	: m_texture_retracted(imgs_path+"commands.png"),
	  m_texture_extended( Entity::nassi_shneiderman 
							? imgs_path+"shapes_ns.png"
							: ( Entity::alternative_io 
								  ? imgs_path+"shapes_alt.png"
								  : imgs_path+"shapes.png" ) ),
	  m_visible(true), m_extended(false), m_fixed(false), m_width(0),
	  m_has_mouse(false), m_current_selection(no_selection)
{
	shapebar_size_min = big_icons ? 34 : 25;
	shapebar_size_max = big_icons ? 200 : 150;
}

void ShapesBar::ProcessMotion (int x, int y) {
	if (!m_visible) { m_current_selection = no_selection; return; }
	m_extended = x > win_w-m_width;
	if (m_extended) {
		m_current_selection = y/(win_h/cant_shapes_in_bar)+1;
		if (m_current_selection > cant_shapes_in_bar) m_current_selection = no_selection;
	} else {
		m_current_selection=no_selection; return;
	}
}

bool ShapesBar::ProcessMouse (int button, int state, int x, int y) {
	ProcessMotion(x,y); // ensures updated m_current_selection
	if (m_current_selection==no_selection) return false;
	if (button!=ZMB_LEFT||state!=ZMB_DOWN) return true;
	m_extended = false;
	
	Entity*new_entity = NULL;
	switch (m_current_selection) {
	case 1: 
		if (!Entity::show_comments) ProcessMenu(MO_TOGGLE_COMMENTS);
		new_entity = new Entity(ET_COMENTARIO,"");
		if (canvas->GetModifiers()&MODIFIER_SHIFT) new_entity->variante=true;
		break;
	case 2: 
		new_entity = new Entity(ET_ASIGNAR,""); 
		if (canvas->GetModifiers()&MODIFIER_SHIFT) new_entity->variante=true;
		break;
	case 3: new_entity = new Entity(ET_ESCRIBIR,""); break;
	case 4: new_entity = new Entity(ET_LEER,""); break;
	case 5: new_entity = new Entity(ET_SI,""); break;
	case 6: new_entity = new Entity(ET_SEGUN,""); break;
	case 7: new_entity = new Entity(ET_MIENTRAS,""); break;
	case 8: 
		new_entity = new Entity(ET_REPETIR,""); 
		if ( (canvas->GetModifiers()&MODIFIER_SHIFT)!=lang[LS_PREFER_REPEAT_WHILE] ) new_entity->variante=true;
		break;
	case 9:
		new_entity = new Entity(ET_PARA,""); 
		if (canvas->GetModifiers()&MODIFIER_SHIFT) new_entity->variante=true;
		break;
	}
	if (new_entity) {
		fix_mouse_coords(x,y);
		new_entity->m_x = 0; new_entity->m_y = 0;
		to_set_mouse = new_entity;
		new_entity->SetMouse();					
		new_entity->SetEdit(false);
		new_entity->SetPosition(x,y);
	}
	return true;
}

void ShapesBar::Draw() {
	if (m_width == 0) return;
	glLineWidth(menu_line_width);
	glColor3fv(color_menu_back);
	// shapebar
	glBegin(GL_QUADS);
	glVertex2i(win_w-m_width,0);
	glVertex2i(win_w-m_width,win_h);
	glVertex2i(win_w,win_h);
	glVertex2i(win_w,0);
	glEnd();
	// shapebar
	glColor3fv(color_menu);
	glBegin(GL_LINES);
	glVertex2i(win_w-m_width,0);
	glVertex2i(win_w-m_width,win_h);
	glEnd();
	if (m_extended||m_fixed) {
		mouse_cursor = Z_CURSOR_INHERIT;
		float sh=float(win_h)/cant_shapes_in_bar;
		// resaltado(fondo) de la entidad seleccionada
		if(m_current_selection!=no_selection) {
			glColor3fv(color_menu_sel);
			glBegin(GL_QUADS);
			glVertex2f(win_w,        win_h-sh*(m_current_selection-1)); 
			glVertex2f(win_w-m_width,win_h-sh*(m_current_selection-1)); 
			glVertex2f(win_w-m_width,win_h-sh*(m_current_selection-0)); 
			glVertex2f(win_w,        win_h-sh*(m_current_selection-0)); 
			glEnd();
		}
		
		// imagenes de cada entidad
		glEnable(GL_TEXTURE_2D);
		m_texture_extended.Select();
		if (m_visible) glColor3f(1,1,1); 
		else glColor4f(1,1,1,.5); // dimm when fixed but not enabled
		glBegin(GL_QUADS);
		float dy=sh, th0=0, th1=1.0/float(cant_shapes_in_bar), dth=1.0/float(cant_shapes_in_bar);
		float x0=win_w-m_width, x1=win_w, y0=0, y1=float(win_h)/float(cant_shapes_in_bar); 
		float ratio=float(m_width)/float(win_h)/float(cant_shapes_in_bar);
		if (ratio>(m_texture_extended.r/8)) {
			float dx=(x1-x0)*(1-(m_texture_extended.r/8)/ratio)/2;
			x0+=dx; x1-=dx;
		} else {
			float dy=(y1-y0)*(1-ratio/(m_texture_extended.r/8))/2;
			y0+=dy; y1-=dy;
		}
		for(int i=0;i<cant_shapes_in_bar;i++) {
			glTexCoord2f(0*m_texture_extended.max_s,th0*m_texture_extended.max_t); glVertex2f(x0,y0);
			glTexCoord2f(1*m_texture_extended.max_s,th0*m_texture_extended.max_t); glVertex2f(x1,y0);
			glTexCoord2f(1*m_texture_extended.max_s,th1*m_texture_extended.max_t); glVertex2f(x1,y1);
			glTexCoord2f(0*m_texture_extended.max_s,th1*m_texture_extended.max_t); glVertex2f(x0,y1);
			y0+=dy; y1+=dy; th0+=dth; th1+=dth;
		}
		glEnd();
		glDisable(GL_TEXTURE_2D);
		// lineas que separan las entidades
		glBegin(GL_LINES);
		glColor3fv(color_menu);
		y0=win_h-dy;
		for(int i=0;i<cant_shapes_in_bar-1;i++,y0-=dy) {
			glVertex2f(win_w-m_width,y0);
			glVertex2f(win_w,y0);
		}
		glEnd();
	} else { 
		glEnable(GL_TEXTURE_2D);
		m_texture_retracted.Select();
		glColor3f(1,1,1);
		glBegin(GL_QUADS);
		glTexCoord2f(0*m_texture_retracted.max_s,0*m_texture_retracted.max_t); glVertex2i(win_w-m_width,(win_h-m_texture_retracted.h)/2);
		glTexCoord2f(1*m_texture_retracted.max_s,0*m_texture_retracted.max_t); glVertex2i(win_w-m_width+m_texture_retracted.w,(win_h-m_texture_retracted.h)/2);
		glTexCoord2f(1*m_texture_retracted.max_s,1*m_texture_retracted.max_t); glVertex2i(win_w-m_width+m_texture_retracted.w,(win_h+m_texture_retracted.h)/2);
		glTexCoord2f(0*m_texture_retracted.max_s,1*m_texture_retracted.max_t); glVertex2i(win_w-m_width,(win_h+m_texture_retracted.h)/2);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	
	// shapebar
	if (m_visible) {
		switch(m_current_selection) {
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
			if ((canvas->GetModifiers()&MODIFIER_SHIFT)!=lang[LS_PREFER_REPEAT_WHILE])
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
	}
}

void ShapesBar::Initialize ( ) {
	shapes_bar = new ShapesBar();
}


void ShapesBar::ProcessIdle ( ) {
	int dest = (m_visible||m_fixed) ? ((m_fixed||m_extended) ? shapebar_size_max : shapebar_size_min ) : 0;
	interpolate(m_width, dest);
}

int ShapesBar::GetWidth() const {
	return (m_visible||m_fixed) ? ((m_fixed||m_extended) ? shapebar_size_max : shapebar_size_min ) : 0;
};
