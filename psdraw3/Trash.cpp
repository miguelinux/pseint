#include "../wxPSeInt/string_conversions.h"
#include "Trash.h"
#include "Global.h"
#include "Text.h"
#include "Draw.h"

Trash *trash = NULL;

#define trash_size_max 100
#define trash_size_min 30

Trash::Trash() 
	: m_texture(imgs_path+"trash.png"),
	  m_visible(false), m_extended(false), m_size(0) 
{
	
}

void Trash::Draw ( ) {
	if (!m_size) return;
	
	if (m_extended) {
		DrawTextRaster(color_selection,10+trash_size_max,10,_Z("Eliminar"));
		mouse_cursor = Z_CURSOR_DESTROY;
	}
	
	glLineWidth(menu_line_width);
	glColor3fv(color_menu_back);
	glBegin(GL_QUADS);
	// trash
	glVertex2i(0,m_size);
	glVertex2i(m_size,m_size);
	glVertex2i(m_size,0);
	glVertex2i(0,0);
	glEnd();
	glColor3fv(color_menu);
	glBegin(GL_LINES);
	glVertex2i(0,m_size); glVertex2i(m_size,m_size);
	glVertex2i(m_size,0); glVertex2i(m_size,m_size);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	m_texture.Select();
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	if (trash) {
		glTexCoord2f(0*m_texture.max_s,.77*m_texture.max_t); glVertex2i(0,m_size);
		glTexCoord2f(1*m_texture.max_s,.77*m_texture.max_t); glVertex2i(m_size,m_size);
		glTexCoord2f(1*m_texture.max_s,0*m_texture.max_t); glVertex2i(m_size,0);
		glTexCoord2f(0*m_texture.max_s,0*m_texture.max_t); glVertex2i(0,0);
	} else {
		glTexCoord2f(0*m_texture.max_s,1*m_texture.max_t); glVertex2i(0,m_size);
		glTexCoord2f(.32*m_texture.max_s,1*m_texture.max_t); glVertex2i(m_size,m_size);
		glTexCoord2f(.32*m_texture.max_s,.77*m_texture.max_t); glVertex2i(m_size,0);
		glTexCoord2f(0*m_texture.max_s,.77*m_texture.max_t); glVertex2i(0,0);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void Trash::Initialize ( ) {
	trash = new Trash();
}

void Trash::ProcessIdle ( ) {
	int dest = m_visible ? ( m_extended ? trash_size_max : trash_size_min ) : 0;
	interpolate(m_size,dest);
}

void Trash::ProcessMotion (int x, int y) {
	m_extended = m_visible && x<m_size && (win_h-y)<m_size;	
}

