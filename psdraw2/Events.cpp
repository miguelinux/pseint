#include "Events.h"
#include "Entity.h"
#include "Global.h"
#include "Comm.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "Draw.h"
#include <cstdlib>
#include <iostream>
using namespace std;

extern const int margin; // para los botones de confirm

#ifdef __WIN32__
#include <windows.h>
#define usleep(x) Sleep((x)/1000)
#endif

static Entity *DuplicateEntity(Entity *orig) {
	Entity *nueva=new Entity(orig->type,orig->label);
	nueva->m_x=orig->m_x; nueva->m_y=orig->m_y;
	nueva->x=orig->x; nueva->y=orig->y;
	nueva->fx=orig->fx; nueva->fy=orig->fy;
	nueva->d_x=orig->d_x; nueva->d_y=orig->d_y;
	nueva->d_fx=orig->d_fx; nueva->d_fy=orig->d_fy;
	for(int i=0;i<orig->n_child;i++) { 
		if (orig->child[i]) nueva->LinkChild(i,DuplicateEntity(orig->child[i]));
		else nueva->LinkChild(i,NULL);
	}
	return nueva;
}

void Salir(bool force) {
	if (!force && modified && !confirm) {
		confirm=true; confirm_sel=0;
		return;
	}
	CloseComm();
	exit(0);
}

static void reshape_cb (int w, int h) {
	if (w==0||h==0) return;
	win_h=h; win_w=w;
	glViewport(0,0,w,h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(0,w,0,h);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

void Raise() {
	glutHideWindow();
	glutShowWindow();
}

static void idle_func() {
	ReadComm();
	static int last=glutGet( GLUT_ELAPSED_TIME );
	int now=glutGet( GLUT_ELAPSED_TIME );
	if (now-last<25000) {
		usleep(25000-(now-last));
		last=now;
	}
	Entity *aux=start;
	do {
		aux->Tick();
		aux=aux->all_next;
	} while (aux!=start);
	if (mouse) { 
		interpolate(shapebar_size,0);
		interpolate(menu_size_h,0);
		interpolate(menu_size_w,0);
		if (trash) interpolate(trash_size,trash_size_max);
		else interpolate(trash_size,trash_size_min);
		menu=shapebar=false;
	} else {
		if (shapebar) interpolate(shapebar_size,shapebar_size_max);
		else interpolate(shapebar_size,shapebar_size_min);
		if (menu) { interpolate(menu_size_h,menu_option_height*(MO_HELP+2)); interpolate(menu_size_w,menu_w_max); }
		else { interpolate(menu_size_h,menu_h_min); interpolate(menu_size_w,menu_w_min); }
		interpolate(trash_size,0); trash=false;
	}
	interpolate(d_zoom,zoom);
	glutPostRedisplay();
}

static void passive_motion_cb(int x, int y) {
	if (confirm) {
		y=win_h-y;
		int w,h;
		GetTextSize("a",w,h);
		if (y<win_h/2-2*h-2*margin || y>win_h/2-h+margin) confirm_sel=0;
		else if (x>win_w/2-100 && x<win_w/2-40) confirm_sel=1;
		else if (x>win_w/2+40 && x<win_w/2+100) confirm_sel=2;
		else confirm_sel=0;
		return;
	}
	if (!win_h || !edit_on) return;
	if (mouse) {
		shapebar=false; menu=false;
		return;
	}
	menu=x<menu_size_w&&y<menu_size_h;
	shapebar=x>win_w-shapebar_size;
	if (menu) {
//		y=win_h-y;
		y=y-10;
		menu_sel=y/menu_option_height+1;
		if (menu_sel<1 || menu_sel>MO_HELP) menu_sel=0;
//		if (y>win_h-menu_size_h+245) menu_sel=1;
//		else if (y>win_h-menu_size_h+205) menu_sel=2;
//		else if (y>win_h-menu_size_h+165) menu_sel=3;
//		else if (y>win_h-menu_size_h+125) menu_sel=4;
//		else if (y>win_h-menu_size_h+85) menu_sel=5;
//		else if (y>win_h-menu_size_h+45) menu_sel=6;
//		else menu_sel=0;
	} else if (shapebar) {
		shapebar_sel=y/(win_h/8)+1;
		if (y==9) y=0;
	}
}
static void motion_cb(int x, int y) {
	if (confirm) return;
	y=win_h-y; 
	trash=x<trash_size && y<trash_size;
	y/=zoom; x/=zoom;
	if (selecting_zoom) {
		cur_x=x; cur_y=y;
		return;
	}
	if (panning) { 
		d_dx+=x-m_x0; m_x0=x;
		d_dy+=y-m_y0; m_y0=y;
	} if (mouse) { 
		cur_y=y; cur_x=mouse->d_x;
		mouse->d_x=x-mouse->m_x;
		mouse->d_y=y-mouse->m_y;
		mouse->d_fx=x-mouse->m_x;
		mouse->d_fy=y-mouse->m_y;
	}
	if (trash && mouse) {
		if (mouse->type!=ET_OPCION && (mouse->parent||mouse->prev)) {
			mouse->UnLink();
			start->Calculate();
		}
	}
}

void ZoomExtend(int x0, int y0, int x1, int y1, float max) {
	if (x1<x0) { int aux=x1; x1=x0; x0=aux; }
	if (y0<y1) { int aux=y1; y1=y0; y0=aux; }
	if (x1-x0<10||y0-y1<10) return;
	int h=y0-y1, w=x1-x0;
	double zh=float(win_h-40)/h; // zoom para ajustar alto
	double zw=float(win_w-shapebar_size_min-40)/w; // zoom para ajustar ancho
	if (zw>zh) zoom=zh; else zoom=zw; // ver cual tamaño manda
	if (zoom>max) zoom=max;
	d_dx=win_w/zoom/2-(x1+x0)/2;
	d_dy=win_h/zoom/2-(y1+y0)/2/*+h/2/zoom*/;
}


void ProcessMenu(int op) {
	menu=false;
	if (op==MO_ZOOM_EXTEND) {
		int h=0,wl=0,wr=0;
		start->Calculate(wl,wr,h); // calcular tamaño total
		ZoomExtend(start->x-wl,start->y,start->x+wr,start->y-h,1.5);
	} else if (op==MO_SAVE) {
		SendUpdate();
	} else if (op==MO_RUN) {
		SendUpdate(true);
	} else if (op==MO_SAVE_CLOSE) {
		SendUpdate(); Salir();
	} else if (op==MO_CLOSE) {
		Salir();
	} else if (op==MO_HELP) {
		SendHelp();
	}
}

static void mouse_cb(int button, int state, int x, int y) {
	if (confirm) {
		if (confirm_sel==1) confirm=false;
		else if (confirm_sel==2) Salir();
	}
	y=win_h-y; y/=zoom; x/=zoom;
	if (button==4||button==3) {
		double f=button==4?1.05:1.0/1.05;
		zoom*=f;
		d_dx-=x*f-x;
		d_dy-=y*f-y;
	} else if (state==GLUT_DOWN) {
		if (button==GLUT_MIDDLE_BUTTON) { // click en el menu
			cur_x=m_x0=x; cur_y=m_y0=y; selecting_zoom=true;
			return;
		}
		if (menu && button==GLUT_LEFT_BUTTON) { // click en el menu
			if (menu_sel) ProcessMenu(menu_sel);
			return;
		}
		if (shapebar && button==GLUT_LEFT_BUTTON) { // click en la barra de entidades
			if (!shapebar_sel) return;
			shapebar=false;
			Entity*aux=NULL;
			switch (shapebar_sel) {
			case 1: aux = new Entity(ET_ASIGNAR,""); break;
			case 2: aux = new Entity(ET_ESCRIBIR,""); break;
			case 3: aux = new Entity(ET_LEER,""); break;
			case 4: aux = new Entity(ET_SI,""); break;
			case 5: aux = new Entity(ET_SEGUN,""); break;
			case 6: aux = new Entity(ET_MIENTRAS,""); break;
			case 7: aux = new Entity(ET_REPETIR,""); break;
			case 8: aux = new Entity(ET_PARA,""); break;
			}
			if (!aux) return;
			aux->m_x=0; aux->m_y=0;
			aux->SetMouse();
			aux->SetEdit();
			return;
		}
		// click en una entidad? izquierdo=mover, derecho=editar label
		Entity *aux=start;
		if (mouse) mouse->UnSetMouse();
		do {
			if (aux->CheckMouse(x,y)) { 
				if (aux->type==ET_PROCESO) break;
				if (button==GLUT_RIGHT_BUTTON) {
					aux->SetEdit(); return;
				} else {
					if (aux->type==ET_AUX_PARA) aux=aux->parent;
					if (glutGetModifiers()==GLUT_ACTIVE_SHIFT) {
						aux=DuplicateEntity(aux);
						aux->SetEdit();
					} 
					if (edit!=aux) {
						edit=NULL;
					}
					aux->SetMouse();
					start->Calculate();
					// doble click
					static int last_click_time=0;
					static Entity *last_click_mouse=NULL;
					int click_time=glutGet(GLUT_ELAPSED_TIME);
					if (click_time-last_click_time<500 && last_click_mouse==mouse) mouse->SetEdit();
					last_click_mouse=mouse; last_click_time=click_time;
					return;
				}
				break;
			}
			aux=aux->all_next;
		} while (aux!=start);
//		if (button==GLUT_RIGHT_BUTTON) { // click en el aire, derecho=pan
		m_x0=x; m_y0=y; panning=true;
//		}
	} else {
		if (button==GLUT_MIDDLE_BUTTON) {
			ZoomExtend(m_x0-d_dx,m_y0-d_dy,x-d_dx,y-d_dy);
			selecting_zoom=false;
			return;
		}
		panning=false;
		if (mouse) {
			if (trash && mouse->type==ET_OPCION) {
				Entity *p=mouse->parent;
				mouse->UnLink(); 
				p->Calculate();
			} 
			mouse->UnSetMouse();
		}
		start->Calculate();
	}
}

static void keyboard_cb(unsigned char key, int x, int y) {
	if (!edit) {
		if (key==27) Salir();
		return;
	} else {
		edit->EditLabel(key);
	}
}

void ToggleFullScreen() {
	static bool fullscreen=false;
	static int w_win_h,w_win_w, win_x,win_y; // estado antes de pasar a pantalla completa
	if ((fullscreen=!fullscreen)) {
		win_x = glutGet((GLenum)GLUT_WINDOW_X);
		win_y = glutGet((GLenum)GLUT_WINDOW_Y); 
		w_win_h=win_h; w_win_w=win_w;
		glutFullScreen();
	} else {
		glutReshapeWindow(w_win_w,w_win_h);
		glutPositionWindow(win_x,win_y);
		glutPositionWindow(100,100);
	}
}

void ToggleEditable() {
	static bool old_edit_on;
	if (edit_on) {
		old_edit_on=true;
		edit_on=false; edit=false;
	} else {
		edit_on=old_edit_on;
	}
	start->Calculate();
}

static void keyboard_esp_cb(int key, int x, int y) {
	if (key==GLUT_KEY_F5) ProcessMenu(MO_SAVE);
	else if (key==GLUT_KEY_F2) ProcessMenu(MO_SAVE);
	else if (key==GLUT_KEY_F9) ProcessMenu(MO_RUN);
	else if (key==GLUT_KEY_F1) ProcessMenu(MO_HELP);
	else if (key==GLUT_KEY_F7) ToggleEditable();
	else if (key==GLUT_KEY_F11) ToggleFullScreen();
	else if (key==GLUT_KEY_F12) ProcessMenu(MO_ZOOM_EXTEND);
	else if (edit) edit->EditSpecialLabel(key);
}

void initialize() {
	glutInitDisplayMode (GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize (win_w,win_h);
	glutInitWindowPosition (100,100);
	glutCreateWindow ("PSDraw v2");
	glutIdleFunc (idle_func);
	glutDisplayFunc (display_cb);
	glutReshapeFunc (reshape_cb);
	glutMouseFunc(mouse_cb);
	glutMotionFunc(motion_cb);
	glutKeyboardFunc(keyboard_cb);
	glutSpecialFunc(keyboard_esp_cb);
	glutPassiveMotionFunc(passive_motion_cb);
	glClearColor(color_back[0],color_back[1],color_back[2],1.f);
}
