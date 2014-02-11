#include "GLstuff.h"
#include "Draw.h"
#include "Global.h"
#include "Entity.h"
#include "Events.h"
#include <cstring>
#include "Textures.h"
#include <wx/cursor.h>
#include "MainWindow.h"
using namespace std;

CURSORES mouse_cursor;

#define mouse_link_delta 250
static int mouse_link_x=0,mouse_link_y=0; 

static string status_text; // texto para la barra de estado
static const float *status_color; // color del texto para la barra de estado
void SetStatus(const float *color, const string &text) {
	status_color=color;
	status_text=text;
}


void GetTextSize(const string &label, int &w, int &h) {
	w=label.size()*10;
	h=15;
}

void DrawTextRaster(const float *color, int x, int y, const char *t) {
	glColor3fv(color);
	glRasterPos2f(x,y);
	int i=0;
	while (t[i]!='\0') glutBitmapCharacter(GLUT_BITMAP_9_BY_15,t[i++]);
}

void DrawText(const float *color, int x, int y, const char *t) {
	glColor3fv(color);
	glPushMatrix();
	glTranslated(x,y,0);
	glScaled(.08,.12,.1);
	while (*t) 
		dibujar_caracter(*(t++));
	glPopMatrix();
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
#ifdef _USE_TEXTURES
if (use_textures) {
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
} else {
#endif
	if (trash) {
		glColor3fv(color_selection);
		int tm=trash_size/6;
		// borde
		glVertex2i(2*tm,tm); glVertex2i(trash_size-tm*2,tm);
		glVertex2i(2*tm,tm); glVertex2i(tm,trash_size-tm); 
		glVertex2i(trash_size-tm*2,tm);	glVertex2i(trash_size-tm,trash_size-tm);
		glVertex2i(tm,trash_size-tm); glVertex2i(trash_size-tm,trash_size-tm);
		// tramado
		glVertex2i(tm+2*tm  ,  tm); glVertex2i(4*tm+2*tm/4,trash_size-3*tm);
		glVertex2i(tm+  tm  ,  tm); glVertex2i(trash_size-tm,trash_size-tm);
		glVertex2i(tm+3*tm/4,2*tm); glVertex2i(trash_size-2*tm,trash_size-tm);
		glVertex2i(tm+2*tm/4,3*tm); glVertex2i(trash_size-3*tm,trash_size-tm);
		glVertex2i(tm+1*tm/4,4*tm); glVertex2i(trash_size-4*tm,trash_size-tm);
		glVertex2i(trash_size-tm-2*tm  ,  tm); glVertex2i(tm+2*tm/4,trash_size-3*tm);
		glVertex2i(trash_size-tm-  tm  ,  tm); glVertex2i(tm,trash_size-tm);
		glVertex2i(trash_size-tm-3*tm/4,2*tm); glVertex2i(2*tm,trash_size-tm);
		glVertex2i(trash_size-tm-2*tm/4,3*tm); glVertex2i(3*tm,trash_size-tm);
		glVertex2i(trash_size-tm-1*tm/4,4*tm); glVertex2i(4*tm,trash_size-tm);
		glColor3fv(color_menu);
	} else {
		int t6=trash_size/5;
		glVertex2i(t6,t6); glVertex2i(4*t6,4*t6);
		glVertex2i(t6,4*t6); glVertex2i(4*t6,t6);
	}
	glEnd();
#ifdef _USE_TEXTURES
}
#endif
}
static void DrawMenuAndShapeBar() {
	glLineWidth(menu_line_width);
	glColor3fv(color_menu_back);
	glBegin(GL_QUADS);
	// shapebar
	glVertex2i(win_w-shapebar_size,0);
	glVertex2i(win_w-shapebar_size,win_h);
	glVertex2i(win_w,win_h);
	glVertex2i(win_w,0);
	// menu
	glVertex2i(0,win_h);
	glVertex2i(menu_size_w,win_h);
	glVertex2i(menu_size_w,win_h-menu_size_h);
	glVertex2i(0,win_h-menu_size_h);
	glEnd();
	glColor3fv(color_menu);
	glBegin(GL_LINES);
	// menu
	glVertex2i(0,win_h-menu_size_h); glVertex2i(menu_size_w,win_h-menu_size_h);
	glVertex2i(menu_size_w,win_h); glVertex2i(menu_size_w,win_h-menu_size_h);
	// shapebar
	glVertex2i(win_w-shapebar_size,0);
	glVertex2i(win_w-shapebar_size,win_h);
	if (shapebar) {
		
		int y=win_h, x=win_w-shapebar_size, sh=win_h/8;
		
		if(shapebar_sel>0) {
			glEnd();
			glColor3fv(color_menu_sel);
			glBegin(GL_QUADS);
			glVertex2i(win_w,              win_h-sh*(shapebar_sel-1)); 
			glVertex2i(win_w-shapebar_size,win_h-sh*(shapebar_sel-1)); 
			glVertex2i(win_w-shapebar_size,win_h-sh*(shapebar_sel-0)); 
			glVertex2i(win_w,              win_h-sh*(shapebar_sel-0)); 
			glEnd();
			glColor3fv(color_menu);
			glBegin(GL_LINES);
		}
#ifdef _USE_TEXTURES
if (use_textures) {
		glEnd();
		glEnable(GL_TEXTURE_2D);
		texture_shapes.Select();
		glColor3f(1,1,1);
		glBegin(GL_QUADS);
		float dy=win_h/8, th0=0, th1=1.0/8, dth=1.0/8;
		float x0=win_w-shapebar_size, x1=win_w, y0=0, y1=win_h/8.0; 
		float ratio=float(shapebar_size)/win_h/8;
		if (ratio>texture_shapes.r) {
			float dx=(x1-x0)*(1-texture_shapes.r/ratio)/2;
			x0+=dx; x1-=dx;
		} else {
			float dy=(y1-y0)*(1-ratio/texture_shapes.r)/2;
			y0+=dy; y1-=dy;
		}
		for(int i=0;i<8;i++) {
			glTexCoord2f(0*texture_shapes.max_s,th0*texture_shapes.max_t); glVertex2f(x0,y0);
			glTexCoord2f(1*texture_shapes.max_s,th0*texture_shapes.max_t); glVertex2f(x1,y0);
			glTexCoord2f(1*texture_shapes.max_s,th1*texture_shapes.max_t); glVertex2f(x1,y1);
			glTexCoord2f(0*texture_shapes.max_s,th1*texture_shapes.max_t); glVertex2f(x0,y1);
			y0+=dy; y1+=dy; th0+=dth; th1+=dth;
		}
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
		glColor3fv(color_menu);
		for(int i=0,y0=win_h-dy;i<7;i++,y0-=dy) {
			glVertex2i(win_w-shapebar_size,y0);
			glVertex2i(win_w,y0);
		}
} else {
#endif
		int x0,x1,y0,y1,xm,ym;
		x0=x+shapebar_size/5; x1=x+shapebar_size-shapebar_size/5;
		y0=y-3*sh/10; y1=y-7*sh/10; xm=(x0+x1)/2; ym=(y0+y1)/2;
		int delta=win_h/8/8; int rombo_h=win_h/8/5;
		// asignar
		if (shapebar_sel==1) glColor3fv(color_selection);
		glVertex2i(x0,y0); glVertex2i(x0,y1);
		glVertex2i(x1,y1); glVertex2i(x0,y1);
		glVertex2i(x1,y1); glVertex2i(x1,y0);
		glVertex2i(x0,y0); glVertex2i(x1,y0);
		glColor3fv(color_menu); 
		// leer
		y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
		if (shapebar_sel==2) glColor3fv(color_selection);
		glVertex2i(x0+delta,y0); glVertex2i(x0-delta,y1);
		glVertex2i(x1-delta,y1); glVertex2i(x0-delta,y1);
		glVertex2i(x1-delta,y1); glVertex2i(x1+delta,y0);
		glVertex2i(x0+delta,y0); glVertex2i(x1+delta,y0);
		glVertex2i(x1-2*delta,y0-sh/10); glVertex2i(x1-2*delta,y0+sh/5);
		glVertex2i(x1-2*delta,y0+sh/5); glVertex2i(x1-1*delta,y0+sh/10);
		glVertex2i(x1-2*delta,y0+sh/5); glVertex2i(x1-3*delta,y0+sh/10);
		glColor3fv(color_menu); 
		// escribir
		y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
		if (shapebar_sel==3) glColor3fv(color_selection);
		y0=y-3*sh/10; y1=y-7*sh/10;
		glVertex2i(x0+delta,y0); glVertex2i(x0-delta,y1);
		glVertex2i(x1-delta,y1); glVertex2i(x0-delta,y1);
		glVertex2i(x1-delta,y1); glVertex2i(x1+delta,y0);
		glVertex2i(x0+delta,y0); glVertex2i(x1+delta,y0);
		glVertex2i(x1-2*delta,y0-sh/5); glVertex2i(x1-2*delta,y0+sh/10);
		glVertex2i(x1-2*delta,y0-sh/5); glVertex2i(x1-3*delta,y0-sh/10);
		glVertex2i(x1-2*delta,y0-sh/5); glVertex2i(x1-1*delta,y0-sh/10);
		glColor3fv(color_menu); 
		// si
		y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
		if (shapebar_sel==4) glColor3fv(color_selection);
		glVertex2i(xm,y0+delta); glVertex2i(xm,y0); // entra
		glVertex2i(xm,y0); glVertex2i(x0+delta,ym);
		glVertex2i(xm,y1); glVertex2i(x0+delta,ym);
		glVertex2i(xm,y1); glVertex2i(x1-delta,ym);
		glVertex2i(xm,y0); glVertex2i(x1-delta,ym);
		glVertex2i(x0+delta,ym); glVertex2i(x0-delta,ym); // izquierda
		glVertex2i(x0-delta,ym); glVertex2i(x0-delta,y1-delta);
		glVertex2i(x1-delta,ym); glVertex2i(x1+delta,ym); // derecha
		glVertex2i(x1+delta,ym); glVertex2i(x1+delta,y1-delta);
		glColor3fv(color_menu); 
		// segun
		if (shapebar_sel==5) glColor3fv(color_selection);
		y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
		glVertex2i(xm,y0+delta); glVertex2i(xm,y0); // entra
		glVertex2i(xm,y0); glVertex2i(x0,y1); // rombo
		glVertex2i(xm,y0); glVertex2i(x1,y1);
		glVertex2i(x0,y1); glVertex2i(x1,y1);
		glVertex2i(x0+delta,y1); glVertex2i(x0+delta,y1-delta); // salidas
		glVertex2i(x1-delta,y1); glVertex2i(x1-delta,y1-delta);
		glVertex2i(x0+4*delta,y1); glVertex2i(x0+4*delta,y1-delta);
		glVertex2i(x1-4*delta,y1); glVertex2i(x1-4*delta,y1-delta);
		glColor3fv(color_menu); 
		// mientras
		y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
		if (shapebar_sel==6) glColor3fv(color_selection);
		glVertex2i(xm,y0+2*delta); glVertex2i(xm,y0); // entra
		glVertex2i(xm,y0); glVertex2i(x0+delta,y0-rombo_h/2); // rombo
		glVertex2i(xm,y0-rombo_h); glVertex2i(x0+delta,y0-rombo_h/2);
		glVertex2i(xm,y0-rombo_h); glVertex2i(x1-delta,y0-rombo_h/2);
		glVertex2i(xm,y0); glVertex2i(x1-delta,y0-rombo_h/2);
		glVertex2i(xm,y0-rombo_h); glVertex2i(xm,y1); // verdadero
		glVertex2i(xm,y1); glVertex2i(x0,y1); 
		glVertex2i(x0,y1); glVertex2i(x0,y0+delta); 
		glVertex2i(xm,y0+delta); glVertex2i(x0,y0+delta); 
		glVertex2i(x1-delta,y0-rombo_h/2); glVertex2i(x1,y0-rombo_h/2); // falso
		glVertex2i(x1,y0-rombo_h/2); glVertex2i(x1,y1-delta);
		glVertex2i(xm,y1-delta); glVertex2i(x1,y1-delta);
		glVertex2i(xm,y1-delta); glVertex2i(xm,y1-2*delta); 
		glColor3fv(color_menu); 
		// repetir
		y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
		if (shapebar_sel==7) glColor3fv(color_selection);
		glVertex2i(xm,y0+2*delta); glVertex2i(xm,y1+rombo_h); // entra
		glVertex2i(xm,y1+rombo_h); glVertex2i(x0+delta,y1+rombo_h/2); // rombo
		glVertex2i(xm,y1); glVertex2i(x0+delta,y1+rombo_h/2);
		glVertex2i(xm,y1); glVertex2i(x1-delta,y1+rombo_h/2);
		glVertex2i(xm,y1+rombo_h); glVertex2i(x1-delta,y1+rombo_h/2);
		glVertex2i(xm,y1); glVertex2i(xm,y1-delta); // verdadero
		glVertex2i(x0+delta,y1+rombo_h/2); glVertex2i(x0-delta,y1+rombo_h/2); // falso
		glVertex2i(x0-delta,y1+rombo_h/2); glVertex2i(x0-delta,y0);
		glVertex2i(xm,y0); glVertex2i(x0-delta,y0); 
		glColor3fv(color_menu); 
		// para
		y-=sh; glVertex2i(x,y); glVertex2i(win_w,y); y0-=sh; y1-=sh; ym-=sh;
		if (shapebar_sel==8) glColor3fv(color_selection);
		glVertex2i(xm-  delta,ym-  delta); glVertex2i(xm-  delta,ym+  delta); // forma
		glVertex2i(xm-  delta,ym+  delta); glVertex2i(xm-2*delta,ym+2*delta);
		glVertex2i(xm-4*delta,ym+2*delta); glVertex2i(xm-2*delta,ym+2*delta);
		glVertex2i(xm-5*delta,ym+  delta); glVertex2i(xm-4*delta,ym+2*delta);
		glVertex2i(xm-5*delta,ym+  delta); glVertex2i(xm-5*delta,ym-  delta);
		glVertex2i(xm-4*delta,ym-2*delta); glVertex2i(xm-5*delta,ym-  delta);
		glVertex2i(xm-4*delta,ym-2*delta); glVertex2i(xm-2*delta,ym-2*delta);
		glVertex2i(xm-  delta,ym-  delta); glVertex2i(xm-2*delta,ym-2*delta);
		glVertex2i(xm-3*delta,ym+2*delta); glVertex2i(xm-3*delta,y0+  delta); // flecha arriba
		glVertex2i(xm-3*delta,y0  +delta); glVertex2i(xm+2*delta,y0+  delta); 
		glVertex2i(xm-3*delta,ym-2*delta); glVertex2i(xm-3*delta,y1-  delta); // flecha abajo
		glVertex2i(xm-3*delta,y1-  delta); glVertex2i(xm+2*delta,y1-  delta); 
		glVertex2i(xm+2*delta,y1-2*delta); glVertex2i(xm+2*delta,y0+2*delta); // line vertical
		glColor3fv(color_menu); 
#ifdef _USE_TEXTURES
}
#endif
	} else { 
#ifdef _USE_TEXTURES
if (use_textures) {
		glEnd();
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
		glBegin(GL_LINES);
} else {
#endif
		// en la shapebar cerrada se dibuja una flecha para abrirla
		glVertex2i(win_w-2*shapebar_size/3,win_h/2); glVertex2i(win_w-shapebar_size/3,win_h/2-2*shapebar_size/3);
		glVertex2i(win_w-2*shapebar_size/3,win_h/2); glVertex2i(win_w-shapebar_size/3,win_h/2+2*shapebar_size/3);
#ifdef _USE_TEXTURES
}
#endif
	}
	glEnd();
	
	// menu
	int top=menu_option_height*(MO_HELP+1), left=menu_size_w-menu_w_max+15;
#ifdef _USE_TEXTURES 
	if (use_textures) left+=25;
#endif
	if(menu_sel!=MO_NONE) {
		glColor3fv(color_menu_sel);
		glBegin(GL_QUADS);
		glVertex2i(3,            win_h-menu_size_h+top-menu_option_height*(menu_sel-1.7)); 
		glVertex2i(menu_size_w-4,win_h-menu_size_h+top-menu_option_height*(menu_sel-1.7)); 
		glVertex2i(menu_size_w-4,win_h-menu_size_h+top-menu_option_height*(menu_sel-0.7)); 
		glVertex2i(3,            win_h-menu_size_h+top-menu_option_height*(menu_sel-0.7)); 
		glEnd();
	}
	DrawTextRaster(menu_sel==MO_ZOOM_EXTEND?color_selection:color_menu,left,win_h-menu_size_h+top,"Auto-ajustar Zoom"); top-=menu_option_height;
	DrawTextRaster(menu_sel==MO_FUNCTIONS?color_selection:color_menu,left,win_h-menu_size_h+top,"Procesos/SubProcesos..."); top-=menu_option_height;
	DrawTextRaster(menu_sel==MO_RUN       ?color_selection:color_menu,left,win_h-menu_size_h+top,"Ejecutar..."); top-=menu_option_height;
	DrawTextRaster(menu_sel==MO_DEBUG        ?color_selection:color_menu,left,win_h-menu_size_h+top,"Ejecutar Paso a Paso..."); top-=menu_option_height;
	DrawTextRaster(menu_sel==MO_EXPORT ?color_selection:color_menu,left,win_h-menu_size_h+top,"Guardar Imagen..."); top-=menu_option_height;
//	DrawTextRaster(menu_sel==MO_SAVE_CLOSE ?color_selection:color_menu,left,win_h-menu_size_h+top,"Aplicar y Cerrar"); top-=menu_option_height;
	DrawTextRaster(menu_sel==MO_CLOSE      ?color_selection:color_menu,left,win_h-menu_size_h+top,"Cerrar"); top-=menu_option_height;
	DrawTextRaster(menu_sel==MO_HELP       ?color_selection:color_menu,left,win_h-menu_size_h+top,"Ayuda...");
	if (menu) switch(menu_sel) {
	case MO_ZOOM_EXTEND: SetStatus(color_selection,"Ajusta el zoom para visualizar todo el diagrama."); break;
	case MO_FUNCTIONS: SetStatus(color_selection,"Permite elegir cual subproceso editar, crear uno nuevo o eliminar uno existente."); break;
	case MO_RUN: SetStatus(color_selection,"Ejecuta el algoritmo en la terminal de PSeInt."); break;
	case MO_DEBUG: SetStatus(color_selection,"Ejecuta el algoritmo paso a paso marcando los pasos sobre el diagrama."); break;
	case MO_EXPORT: SetStatus(color_selection,"Permite guardar el diagrama como imagen."); break;
	case MO_CLOSE: SetStatus(color_selection,"Cierra el editor, preguntando antes si se deben aplicar los cambios en el pseudocodigo"); break;
	case MO_HELP: SetStatus(color_selection,"Muestra una ventana de ayuda que explica como utilizar este editor y cuales son sus atajos de teclado."); break;
	default:;
	}
	
#ifdef _USE_TEXTURES
if (use_textures) {
	glEnable(GL_TEXTURE_2D);
	texture_trash.Select();
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(.32*texture_trash.max_s,.77*texture_trash.max_t); glVertex2d(menu_size_w-75,win_h-menu_size_h);
	glTexCoord2f(1*texture_trash.max_s,.77*texture_trash.max_t); glVertex2d(menu_size_w-75+texture_trash.w*.68,win_h-menu_size_h);
	glTexCoord2f(1*texture_trash.max_s,1*texture_trash.max_t); glVertex2d(menu_size_w-75+texture_trash.w*.68,win_h-menu_size_h+texture_trash.h*.23);
	glTexCoord2f(.32*texture_trash.max_s,1*texture_trash.max_t); glVertex2d(menu_size_w-75,win_h-menu_size_h+texture_trash.h*.23);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	
	left-=30; top-=8;
	glEnable(GL_TEXTURE_2D);
	texture_menu.Select();
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0*texture_menu.max_s,0*texture_menu.max_t); glVertex2d(left,win_h-menu_size_h+top);
	glTexCoord2f(1*texture_menu.max_s,0*texture_menu.max_t); glVertex2d(left+texture_menu.w,win_h-menu_size_h+top);
	glTexCoord2f(1*texture_menu.max_s,1*texture_menu.max_t); glVertex2d(left+texture_menu.w,win_h-menu_size_h+top+texture_menu.h);
	glTexCoord2f(0*texture_menu.max_s,1*texture_menu.max_t); glVertex2d(left,win_h-menu_size_h+top+texture_menu.h);
	glEnd();
	glDisable(GL_TEXTURE_2D);
} else {
#endif
	DrawTextRaster(color_menu,menu_size_w-55,win_h-menu_size_h+10,"Menu");
#ifdef _USE_TEXTURES
}
#endif
	
	// shapebar
	if (shapebar) {
		switch(shapebar_sel) {
		case 1: SetStatus(color_selection,"Asignacion/Dimension/Definicion"); break;
		case 2: SetStatus(color_selection,"Escribir (instruccion para generar salidas)"); break;
		case 3: SetStatus(color_selection,"Leer (instruccion para obtener entradas)"); break;
		case 4: SetStatus(color_selection,"Si-Entonces (estructura condicional simple)"); break;
		case 5: SetStatus(color_selection,"Segun (estructura de seleccion multiple)"); break;
		case 6: SetStatus(color_selection,"Mientras (estructura repetitiva)"); break;
		case 7: SetStatus(color_selection,"Repetir-Hasta que (estructura repetitiva)"); break;
		case 8: SetStatus(color_selection,"Para (estructura repetitiva)"); break;
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
	DrawTextRaster(color_menu,10,win_h-25,"Seleccione un proceso/subproceso para visualizar:");
	
	int base=win_h-choose_process_d_base, delta=choose_process_d_delta;
	glColor3fv(color_menu); glBegin(GL_LINES); 
	glVertex2i(0,base); glVertex2i(win_w,base);
	glEnd();
	for(int i=0;i<=int(procesos.size()-(edit_on?0:1));i++) {
		
		if (choose_process_sel==i) {
			glColor3fv(color_shape);
			glBegin(GL_QUADS);
			glVertex2i(0,base);
			glVertex2i(win_w,base);
			glVertex2i(win_w,base-delta);
			glVertex2i(0,base-delta);
			glEnd();
		}
		base-=delta;
		
		if (i==int(procesos.size())) {
			DrawTextRaster(choose_process_sel==i?color_selection:color_menu,20,base+10,"Agregar Nuevo SubProceso");
		} else {
			
			string &s=procesos[i]->label;
			int l=s.size(),p=0; int f=s.find('<');
			if (f==string::npos) f=s.find('='); else f++;
			if (f==string::npos) f=0; else f++;
			int t=f; while (t<l && s[t]!=' ' && s[t]!='(') t++;
			
			glColor3fv(choose_process_sel==i?color_selection:color_menu);
			glPushMatrix();
			int px=20, py=base+10;
			if (choose_process_state==3 && choose_process_sel==i) glTranslated(cur_x+(px-m_x0),cur_y+(py-m_y0),0);
			else glTranslated(px,py,0);
			glScaled(.08,.12,.1);
			
			string &sp=procesos[i]->lpre;
			p=0; l=sp.size();
			while (p<l)
				dibujar_caracter(sp[p++]);
			
			p=0; l=s.size();
			while (p<l) {
				if (p==f) glLineWidth(2);
				if (p==t) glLineWidth(1);
				dibujar_caracter(s[p++],true);
			}
			glPopMatrix();		
		}
		
		glLineWidth(1);
		glColor3fv(color_menu); glBegin(GL_LINES); 
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

Entity *next_entity(Entity *aux) {
	static vector<int> pila_nc;
	static vector<Entity*> pila_e;
	// si tiene hijos, se empieza por los hijos
	if (aux->n_child) {
		for(int i=0;i<aux->n_child;i++) { 
			if (aux->child[i]) {
				pila_nc.push_back(i);
				pila_e.push_back(aux);
				return aux->child[i];
			}
		}
	}
	while(true) {
		// si no tiene hijos se pasa a la siguiente
		if (aux->next) return aux->next;
		// si no hay siguiente, se sube
		if (pila_e.empty()) return start; // si no se puede subir estamos en "Fin Proceso"
		int last=++pila_nc.back();
		aux=pila_e.back();
		if (aux->n_child!=last) {
			if (aux->child[last]) return aux->child[last];
			else pila_nc.back()=last;
		} else {
			pila_nc.pop_back();
			pila_e.pop_back();
		}
	}
}

void display_cb() {
	mouse_cursor=Z_CURSOR_CROSSHAIR;
	if (choose_process_state) { DrawChooseProcess(); return; }
	status_color=NULL;
	if (entity_to_del) delete entity_to_del;
	glClear(GL_COLOR_BUFFER_BIT);
	// dibujar el diagrama
	float mx=cur_x/zoom, my=cur_y/zoom;
	Entity *aux=start;
	bool found=false;
	int line_width=2*d_zoom<1?1:int(d_zoom*2);
	glLineWidth(line_width);
	glPushMatrix();
	glScalef(d_zoom,d_zoom,1);
	do {
		if (!found && mouse && mouse->type!=ET_OPCION && (cur_x-mouse_link_x)*(cur_x-mouse_link_x)+(cur_y-mouse_link_y)*(cur_y-mouse_link_y)>mouse_link_delta) {
			if (aux!=mouse && aux->CheckLinkNext(cur_x,cur_y)) {
				mouse_link_x=cur_x; mouse_link_y=cur_y;
				mouse->UnLink();
				aux->LinkNext(mouse);
				start->Calculate();
				found=true;
			} else if (aux->child) {
				int i=aux->CheckLinkChild(cur_x,cur_y);
				if (i!=-1) {
					mouse_link_x=cur_x; mouse_link_y=cur_y;
					mouse->UnLink();
					aux->LinkChild(i,mouse);
					start->Calculate();
					found=true;
				}
			}
		}
		if (!Entity::nassi_schneiderman && edit_on && (mouse?(aux==mouse):aux->CheckMouse(mx,my,false))) {
//			float color_arrow[3]={.9,0,0}; // flechas que guian el flujo y unen entidades
			color_shape[2]=.75; color_arrow[1]=.5; color_arrow[2]=.5; //color_arrow[0]=1;
//			glLineWidth(3*d_zoom<1.5?1.5:int(d_zoom*3));
			aux->Draw();
			color_shape[2]=.9; color_arrow[1]=0; color_arrow[2]=0; //color_arrow[0]=.9;
//			glLineWidth(2*d_zoom<1?1:int(d_zoom*2));
			if (aux->error.size()) SetStatus(color_error,aux->error);
		} else if (debugging && debug_current==aux) {
			glLineWidth(line_width+1);
			if (!Entity::nassi_schneiderman) {
				color_shape[2]=.65; color_arrow[1]=.4; color_arrow[2]=.4; //color_arrow[0]=1;
			}
			aux->Draw();
			if (!Entity::nassi_schneiderman) {
				color_shape[2]=.9; color_arrow[1]=0; color_arrow[2]=0; //color_arrow[0]=.9;
			}
			glLineWidth(line_width);
			draw_debug_arrow(debug_current,5);
		} else {
			aux->Draw();
		}
		if (!aux->error.empty()) draw_error_mark/*_simple*/(aux,4);
		if (!mouse && edit==aux && aux->CheckMouse(mx,my,false)) mouse_cursor=Z_CURSOR_TEXT;
//		aux=aux->all_next;
		aux=next_entity(aux);
	} while (aux!=start);
	if (mouse && mouse->type==ET_OPCION) {
		int i=mouse->parent->CheckLinkOpcion(cur_x,cur_y);
		if (i!=-1) {
			mouse->parent->MoveChild(mouse->child_id,i);
			mouse->parent->Calculate();
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
		case ET_LEER: SetStatus(color_selection,"Lista de variables a leer, separadas por coma."); break;
		case ET_PROCESO: SetStatus(color_selection,edit->lpre=="Proceso "?"Nombre del proceso.":"Prototipo del subproceso."); break;
		case ET_ESCRIBIR: SetStatus(color_selection,"Lista de expresiones a mostrar, separadas por comas."); break;
		case ET_SI: SetStatus(color_selection,"Expresion logica."); break;
		case ET_SEGUN: SetStatus(color_selection,"Expresion de control para la estructura."); break;
		case ET_OPCION: SetStatus(color_selection,"Posible valor para la expresion de control."); break;
		case ET_PARA: SetStatus(color_selection,"Identificador de la variable de control (contador)."); break;
		case ET_MIENTRAS: SetStatus(color_selection,"Expresion de control (logica)."); break;
		case ET_REPETIR: SetStatus(color_selection,"Expresion de control (logica)."); break;
		case ET_ASIGNAR: SetStatus(color_selection,"Asignacion o instruccion secuencial."); break;
		case ET_AUX_PARA: SetStatus(color_selection,edit->parent->child[1]==edit?"Valor inicial para el contador.":(edit->parent->child[2]==edit?"Paso, incremento del contador por cada iteracion.":"Valor final para el contador.")); break;
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
	else if (menu||shapebar||choose_process_sel) mouse_cursor=Z_CURSOR_INHERIT;
}

