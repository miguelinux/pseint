#include "Text.h"
#include "GLstuff.h"
#include "Textures.h"
#include <iostream>

bool use_textures_font = 
#ifdef _USE_FONT
	true
#else
	false
#endif
	;

void GetTextSize(const string &label, int &w, int &h) {
	w=label.size()*10;
	h=15;
}

#define c2i(ch) ((int)((unsigned char)(ch)))

void DrawTextRaster(const float *color, int x, int y, const char *t) {
	glColor3fv(color);
	glRasterPos2f(x,y);
	
	struct char_info { 
		char c; bool acento, mayusculas, enie; 
		void set_char(char c) { acento = mayusculas = enie = false; this->c = c; }
		void set_acento(char x) { c = x; acento = true; mayusculas = x>='A'&&x<='Z'; }
		void set_enie(char x) { c = x; enie = true; mayusculas = x>='A'&&x<='Z'; }
	};
	static char_info char_map[256];
	static bool map_inicializado = false;
	if (!map_inicializado) {
		for(int i=0;i<256;i++)
			char_map[i].set_char((char)i);
		char_map[c2i('á')].set_acento('a');
		char_map[c2i('é')].set_acento('e');
		char_map[c2i('í')].set_acento('i');
		char_map[c2i('ó')].set_acento('o');
		char_map[c2i('ú')].set_acento('u');
		char_map[c2i('Á')].set_acento('A');
		char_map[c2i('É')].set_acento('E');
		char_map[c2i('Í')].set_acento('I');
		char_map[c2i('Ó')].set_acento('O');
		char_map[c2i('Ú')].set_acento('U');
		char_map[c2i('ñ')].set_enie('n');
		char_map[c2i('Ñ')].set_enie('N');
		map_inicializado = true;
	}
	for(int i=0;t[i]!='\0';++i) {
		char_info &ci = char_map[c2i(t[i])];
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,ci.c);
		if (ci.acento) {
			glLineWidth(1);
			glBegin(GL_LINES);
			if (ci.mayusculas) {
				glVertex2i(x+i*9+4,y+11);
				glVertex2i(x+i*9+6,y+14);
			} else {
				glVertex2i(x+i*9+4,y+9);
				glVertex2i(x+i*9+6,y+12);
			}
			glEnd();
		} else if (ci.enie) {
			glLineWidth(1);
			glBegin(GL_LINES);
			if (ci.mayusculas) {
				glVertex2i(x+i*9+2,y+11);
				glVertex2i(x+i*9+7,y+11);
			} else {
				glVertex2i(x+i*9+3,y+8);
				glVertex2i(x+i*9+6,y+8);
			}
			glEnd();
		}
	}
//	glPushMatrix();
//	glTranslatef(x,y,0);
//	glScalef(.08,.12,1);
//	while (t[i]!='\0') dibujar_caracter(t[i++]);
//	glPopMatrix();
}

//void DrawText(const float *color, int x, int y, const char *t) {
//	glColor3fv(color);
//	glPushMatrix();
//	glTranslated(x,y,0);
//	glScaled(.08,.12,.1);
//	begin_texto();
//	while (*t) 
//		dibujar_caracter(*(t++));
//	end_texto();
//	glPopMatrix();
//}

void ys_dibujar_caracter(const char chr);

static float x_texto = 0;

void begin_texto( ) {
#ifdef _USE_FONT
	if (!use_textures_font) return;
	x_texto = 0;
	texture_font.Select();
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
#endif
}

void end_texto( ) {
#ifdef _USE_FONT
	if (!use_textures_font) return;
	glEnd();
	glDisable(GL_TEXTURE_2D);
#endif
}


void dibujar_caracter(const char chr) { 
#ifdef _USE_FONT
	if (use_textures_font) { 
		const float f=1.f/16.f;
		const int w=120, h=150,y0=-25;
		float x0=x_texto;
		unsigned char ch=chr;
		int r=15-ch/16, c=ch%16;
		glTexCoord2f(c*f,r*f);
		glVertex2i(x0,y0);
		glTexCoord2f(c*f+f,r*f);
		glVertex2i(x0+w,y0);
		glTexCoord2f(c*f+f,r*f+f);
		glVertex2i(x0+w,y0+h);
		glTexCoord2f(c*f,r*f+f);
		glVertex2i(x0,y0+h);
		x_texto += 800.0/7.0;
	} else 
#endif
	{
		ys_dibujar_caracter(chr); return;
		glTranslated(800.0/7.0,0.0,0.0);
	}

	
	
}

void dibujar_caracter(const char chr, bool extra) { 
#ifdef _USE_FONT
	if (use_textures_font) { 
		dibujar_caracter(chr);
	} else 
#endif
	{
		x_texto += 150.0/7.0;
		glTranslated(150.0/7.0,0.0,0.0);
	}
}

