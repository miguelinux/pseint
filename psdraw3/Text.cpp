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

void DrawTextRaster(const float *color, int x, int y, const char *t) {
	glColor3fv(color);
	glRasterPos2f(x,y);
	int i=0;
	while (t[i]!='\0') glutBitmapCharacter(GLUT_BITMAP_9_BY_15,t[i++]);
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
	glEnable(GL_TEXTURE_2D);
	texture_font.Select();
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

