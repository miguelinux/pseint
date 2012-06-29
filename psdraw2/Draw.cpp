#include <GL/gl.h>
#include "Draw.h"
#include "Global.h"
#include "Entity.h"
#include "Events.h"
#include <GL/glut.h>

extern const int margin; // para los botones de confirm

void GetTextSize(const string &label, int &w, int &h) {
	w=label.size()*10;
	h=15;
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

static void DrawMenus() {
	glLineWidth(menu_line_width);
	glColor3fv(color_menu_back);
	glBegin(GL_QUADS);
	// shapebar
	glVertex2i(win_w-shapebar_size,0);
	glVertex2i(win_w-shapebar_size,win_h);
	glVertex2i(win_w,win_h);
	glVertex2i(win_w,0);
	// trash
	glVertex2i(0,trash_size);
	glVertex2i(trash_size,trash_size);
	glVertex2i(trash_size,0);
	glVertex2i(0,0);
	// menu
	glVertex2i(0,win_h);
	glVertex2i(menu_size_w,win_h);
	glVertex2i(menu_size_w,win_h-menu_size_h);
	glVertex2i(0,win_h-menu_size_h);
	glEnd();
	glColor3fv(color_menu);
	glBegin(GL_LINES);
	// trash
	glVertex2i(0,trash_size); glVertex2i(trash_size,trash_size);
	glVertex2i(trash_size,0); glVertex2i(trash_size,trash_size);
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
	// menu
	glVertex2i(0,win_h-menu_size_h); glVertex2i(menu_size_w,win_h-menu_size_h);
	glVertex2i(menu_size_w,win_h); glVertex2i(menu_size_w,win_h-menu_size_h);
	// shapebar
	glVertex2i(win_w-shapebar_size,0);
	glVertex2i(win_w-shapebar_size,win_h);
	if (shapebar) {
		int y=win_h, x=win_w-shapebar_size, sh=win_h/8;
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
	} else { 
		// en la shapebar cerrada se dibuja una flecha para abrirla
		glVertex2i(win_w-2*shapebar_size/3,win_h/2); glVertex2i(win_w-shapebar_size/3,win_h/2-2*shapebar_size/3);
		glVertex2i(win_w-2*shapebar_size/3,win_h/2); glVertex2i(win_w-shapebar_size/3,win_h/2+2*shapebar_size/3);
	}
	glEnd();
	// menu
	int top=menu_option_height*(MO_HELP+1), left=menu_size_w-220;
	DrawText(menu_sel==MO_ZOOM_EXTEND?color_selection:color_menu,left,win_h-menu_size_h+top,"Auto-ajustar Zoom"); top-=menu_option_height;
	DrawText(menu_sel==MO_SAVE       ?color_selection:color_menu,left,win_h-menu_size_h+top,"Guardar Cambios"); top-=menu_option_height;
	DrawText(menu_sel==MO_RUN        ?color_selection:color_menu,left,win_h-menu_size_h+top,"Guardar y Ejecutar"); top-=menu_option_height;
	DrawText(menu_sel==MO_SAVE_CLOSE ?color_selection:color_menu,left,win_h-menu_size_h+top,"Guardar y Cerrar"); top-=menu_option_height;
	DrawText(menu_sel==MO_CLOSE      ?color_selection:color_menu,left,win_h-menu_size_h+top,"Cerrar Sin Guardar"); top-=menu_option_height;
	DrawText(menu_sel==MO_HELP       ?color_selection:color_menu,left,win_h-menu_size_h+top,"Ayuda...");
	DrawText(color_menu,menu_size_w-55,win_h-menu_size_h+10,"Menu");
	// shapebar
	if (shapebar) {
		if (shapebar_sel==1) DrawText(color_selection,10,10,"Asiganacion/Dimension/Definicion");
		if (shapebar_sel==2) DrawText(color_selection,10,10,"Escribir");
		if (shapebar_sel==3) DrawText(color_selection,10,10,"Leer");
		if (shapebar_sel==4) DrawText(color_selection,10,10,"Si-Entonces");
		if (shapebar_sel==5) DrawText(color_selection,10,10,"Segun");
		if (shapebar_sel==6) DrawText(color_selection,10,10,"Mientras");
		if (shapebar_sel==7) DrawText(color_selection,10,10,"Repetir-Hasta que");
		if (shapebar_sel==8) DrawText(color_selection,10,10,"Para");
	} else if (trash) DrawText(color_selection,10+trash_size_max,10,"Eliminar");
}

static void DrawConfirm() {
	
	glClear(GL_COLOR_BUFFER_BIT);
	glLineWidth(menu_line_width);
	int h,w;
	const char *text1="Hay cambios sin guardar.";
	const char *text2="¿Cerrar el editor y perder los cambios?";
	GetTextSize(text1,w,h); DrawText(color_menu,win_w/2-w/2,win_h/2+h*3,text1);
	GetTextSize(text2,w,h); DrawText(color_menu,win_w/2-w/2,win_h/2+h*1,text2);
	
	const char *text3="No";
	GetTextSize(text3,w,h); 
	if (confirm_sel==1) {
		glColor3fv(color_shape);
		glBegin(GL_QUADS);
		glVertex2i(win_w/2-100,win_h/2-2*h-2*margin);
		glVertex2i(win_w/2-40,win_h/2-2*h-2*margin);
		glVertex2i(win_w/2-40,win_h/2-h+margin);
		glVertex2i(win_w/2-100,win_h/2-h+margin);
		glEnd();
	}
	DrawText(confirm_sel==1?color_selection:color_menu,win_w/2-w/2-70,win_h/2-h*2,text3);
	glBegin(GL_LINE_LOOP);
	glVertex2i(win_w/2-100,win_h/2-2*h-2*margin);
	glVertex2i(win_w/2-40,win_h/2-2*h-2*margin);
	glVertex2i(win_w/2-40,win_h/2-h+margin);
	glVertex2i(win_w/2-100,win_h/2-h+margin);
	glEnd();
	
	const char *text4="Si";
	GetTextSize(text4,w,h); 
	if (confirm_sel==2) {
		glColor3fv(color_shape);
		glBegin(GL_QUADS);
		glVertex2i(win_w/2+100,win_h/2-2*h-2*margin);
		glVertex2i(win_w/2+40,win_h/2-2*h-2*margin);
		glVertex2i(win_w/2+40,win_h/2-h+margin);
		glVertex2i(win_w/2+100,win_h/2-h+margin);
		glEnd();
	}
	DrawText(confirm_sel==2?color_selection:color_menu,win_w/2-w/2+70,win_h/2-h*2,text4);
	glBegin(GL_LINE_LOOP);
	glVertex2i(win_w/2+100,win_h/2-2*h-2*margin);
	glVertex2i(win_w/2+40,win_h/2-2*h-2*margin);
	glVertex2i(win_w/2+40,win_h/2-h+margin);
	glVertex2i(win_w/2+100,win_h/2-h+margin);
	glEnd();
	glutSwapBuffers();
	
}

void display_cb() {
	if (confirm) { DrawConfirm(); return; }
	if (entity_to_del) delete entity_to_del;
	glClear(GL_COLOR_BUFFER_BIT);
	// dibujar el diagrama
	Entity *aux=start;
	bool found=false;
	glLineWidth(zoom*2);
	glPushMatrix();
	glScalef(d_zoom,d_zoom,1);
	do {
		if (!found && mouse && mouse->type!=ET_OPCION) {
			if (aux!=mouse && aux->CheckLinkNext(cur_x,cur_y)) {
				mouse->UnLink();
				aux->LinkNext(mouse);
				start->Calculate();
				found=true;
			} else if (aux->child) {
				int i=aux->CheckLinkChild(cur_x,cur_y);
				if (i!=-1) {
					mouse->UnLink();
					aux->LinkChild(i,mouse);
					start->Calculate();
					found=true;
				}
			}
		}
		aux->Draw();
		aux=aux->all_next;
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
	if (edit_on) DrawMenus();
	// dibujar la seleccion para que quede delante de todo
	if (mouse) {
		glLineWidth(zoom*2);
		glPushMatrix();
		glScalef(d_zoom,d_zoom,1);
		mouse->Draw();
		glPopMatrix();
	}
	glutSwapBuffers();
}

