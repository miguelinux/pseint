#include <cstdlib>
#include <iostream>
#include "GLstuff.h"
#include "Events.h"
#include "Entity.h"
#include "Global.h"
#include "Comm.h"
#include "Draw.h"
#include "Load.h"
#include "Textures.h"
#include "MainWindow.h"
#include "Canvas.h"
using namespace std;

#define mouse_setted_delta 1000
static int mouse_setted_x,mouse_setted_y; // posicion del click que va a setear el mouse en una entidad cuando se mueva, con correccion de y y zoom aplicados
static Entity *to_set_mouse=NULL; // lo que se va a setear en mouse cuando el cursor se mueva un poco si sigue apretado el botón

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
	if (!force && modified) {
		main_window->AskForExit();
		return;
	}
	CloseComm();
	wxExit();
}

void reshape_cb (int w, int h) {
	if (w==0||h==0) return;
	if (win_w!=0&&win_h!=0) {
		float zw=float(w)/win_w;
		float zh=float(h)/win_h;
		d_dx*=zw/zh;
//		if (zh>zw) zoom*=zh; else zoom*=zw;
		zoom*=zh;
	}	
	win_h=h; win_w=w;
	glViewport(0,0,w,h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(0,w,0,h);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

void Raise() {
	main_window->Hide();
	wxYield();
	main_window->Show();
}

void idle_func() {
	ReadComm();
	static int last=glutGet( GLUT_ELAPSED_TIME );
	int now=glutGet( GLUT_ELAPSED_TIME );
//	static int frames=0;
//	static double times=0;
//	static bool skipped=false;
#define _delta_t 25000
//	times+=now-last; frames++;
//	cerr<<"DT: "<<times/frames<<"     \r";
	if (now-last<_delta_t) {
		wxMicroSleep(_delta_t-(now-last));
		last=now;
	}
//		skipped=false;
//	} else if (now-last>2*_delta_t && !skipped) {
//		skipped=true;
//	} else skipped=false;
	d_zoom=1/((2*1/d_zoom+1/zoom)/3);
	Entity *aux=start;
	do {
		aux->Tick();
		aux=aux->all_next;
	} while (aux!=start);
	if (mouse || choose_process_state==3) { 
		interpolate(shapebar_size,0);
		interpolate(menu_size_h,0);
		interpolate(menu_size_w,0);
		if (trash) interpolate(trash_size,trash_size_max);
		else interpolate(trash_size,trash_size_min);
		menu=shapebar=false;
	} else {
//		if (mouse_setted && --mouse_setted==0 && to_set_mouse) { to_set_mouse->SetMouse(); start->Calculate(); }
		if (shapebar) interpolate(shapebar_size,shapebar_size_max);
		else interpolate(shapebar_size,shapebar_size_min);
		if (menu) { interpolate(menu_size_h,menu_option_height*(MO_HELP+2)); interpolate(menu_size_w,menu_w_max); }
		else { interpolate(menu_size_h,menu_h_min); interpolate(menu_size_w,menu_w_min); }
		interpolate(trash_size,0); trash=false;
	}
	/*if (!skipped) */canvas->Refresh();
}

void passive_motion_cb(int x, int y) {
	if (choose_process_state) {
		if (choose_process_state<3 && choose_process_d_delta) {
			choose_process_sel=(y-choose_process_d_base)/choose_process_d_delta;
			if (choose_process_sel<0||choose_process_sel>int(procesos.size()-(edit_on?0:1))) choose_process_sel=-1;
		}
		return;
	}
	if (!win_h || !edit_on) return;
	if (mouse) {
		shapebar=false; menu=false;
		return;
	}
	if (edit_on) {
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
	cur_x=x; cur_y=win_h-y; canvas->Refresh();
}
void motion_cb(int x, int y) {
	y=win_h-y; 
	trash=x<trash_size && y<trash_size;
	y/=zoom; x/=zoom;
	if (to_set_mouse && (x-mouse_setted_x)*(x-mouse_setted_x)+(y-mouse_setted_y)*(y-mouse_setted_y)>mouse_setted_delta) { 
		if (to_set_mouse->type==ET_PROCESO) return; // no permitir mover "proceso" ni "finproceso"
		to_set_mouse->SetMouse(); start->Calculate();
	}
	if (selecting_zoom || choose_process_state) {
		cur_x=x; cur_y=y;
		return;
	}
	if (panning) { 
		d_dx+=x-m_x0; m_x0=x;
		d_dy+=y-m_y0; m_y0=y;
	} 
	if (mouse) { 
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
	menu=false; edit=NULL;
	if (op==MO_ZOOM_EXTEND) {
		int h=0,wl=0,wr=0;
		start->Calculate(wl,wr,h); // calcular tamaño total
		ZoomExtend(start->x-wl,start->y,start->x+wr,start->y-h,1.5);
	} else if (op==MO_FUNCTIONS) {
		choose_process_d_base=choose_process_d_delta=0;
		choose_process_state=1; edit=NULL;
		if (mouse) mouse->UnSetMouse();
	} else if (op==MO_SAVE||op==MO_RUN||op==MO_EXPORT||op==MO_DEBUG) {
		SendUpdate(op);
//	} else if (op==MO_SAVE_CLOSE) {
//		SendUpdate(); Salir();
	} else if (op==MO_CLOSE) {
		Salir();
	} else if (op==MO_HELP) {
		SendHelp();
	}
}

void mouse_cb(int button, int state, int x, int y) {
	to_set_mouse=NULL;
	if (choose_process_state) {
		if (choose_process_state==1) { choose_process_state=2; return; }
		if (choose_process_sel==int(procesos.size())) {
			CreateEmptyProc("SubProceso");
		}
		if (choose_process_sel!=-1) {
			if (state==ZMB_DOWN) {
				choose_process_state=3;
				cur_x=m_x0=x; cur_y=m_y0=win_h-y;
			} else if (trash) {
				if (edit_on && procesos[choose_process_sel]->lpre!="Proceso ")
					procesos.erase(procesos.begin()+choose_process_sel); // no lo quita de la memoria, solo del arreglo, con eso alcanza, algun día corregiré el memory leak
				choose_process_state=2;
			} else {
				SetProc(procesos[choose_process_sel]);
				choose_process_state=0;
				if (button==ZMB_RIGHT) start->SetEdit();
			}
		}
	}
	y=win_h-y; y/=zoom; x/=zoom;
	if (button==ZMB_WHEEL_DOWN||button==ZMB_WHEEL_UP) {
		double f=button==ZMB_WHEEL_UP?1.0/1.12:1.12;
		zoom*=f;
		float dx=x/f-x, dy=y/f-y;
		d_dx+=dx; d_dy+=dy;
	} else if (state==ZMB_DOWN) {
		if (button==ZMB_MIDDLE) { // click en el menu
			cur_x=m_x0=x; cur_y=m_y0=y; selecting_zoom=true;
			return;
		}
		if (menu && button==ZMB_LEFT) { // click en el menu
			if (menu_sel) ProcessMenu(menu_sel);
			return;
		}
		if (shapebar && button==ZMB_LEFT) { // click en la barra de entidades
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
			case 7: 
				aux = new Entity(ET_REPETIR,""); 
				if (canvas->GetModifiers()&MODIFIER_SHIFT) aux->variante=true;
				break;
			case 8: 
				aux = new Entity(ET_PARA,""); 
				if (canvas->GetModifiers()&MODIFIER_SHIFT) aux->variante=true;
				break;
			}
			if (!aux) return;
			aux->m_x=0; aux->m_y=0;
			to_set_mouse=aux;
			aux->SetMouse();
			aux->SetEdit();
			aux->SetPosition(x,y);
			return;
		}
		// click en una entidad? izquierdo=mover, derecho=editar label
		Entity *aux=start;
		if (mouse) mouse->UnSetMouse();
		do {
			if (aux->CheckMouse(x,y)) { 
				if (aux->type==ET_PROCESO && aux!=start) break; // para no editar el "FinProceso"
				if (button==ZMB_RIGHT) {
					aux->SetEdit(); return;
				} else {
					if (canvas->GetModifiers()==MODIFIER_SHIFT) {
						aux=DuplicateEntity(aux);
						aux->SetEdit();
					} 
					if (edit!=aux) {
						edit=NULL;
					}
					to_set_mouse=aux; mouse_setted_x=x; mouse_setted_y=y; // aux->SetMouse(); retrasado
					if (aux->type==ET_AUX_PARA) to_set_mouse=aux->parent; // para que no haga drag del hijo del para, sino de todo el para completo
					// doble click
					static int last_click_time=0;
					static Entity *last_click_mouse=NULL;
					int click_time=glutGet(GLUT_ELAPSED_TIME);
					if (click_time-last_click_time<500 && (last_click_mouse==aux ||  (aux->type==ET_PARA && aux->parent==last_click_mouse)) ) aux->SetEdit();
					last_click_mouse=aux; last_click_time=click_time;
					return;
				}
				break;
			}
			aux=aux->all_next;
		} while (aux!=start);
		m_x0=x; m_y0=y; panning=true;
	} else {
		if (button==ZMB_MIDDLE) {
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

void keyboard_cb(unsigned char key/*, int x, int y*/) {
	if (!edit) {
		if (key==27) Salir();
		return;
	} else {
//		if (canvas->GetModifiers()&MODIFIER_CTRL) return;
		edit->EditLabel(key);
	}
}

void ToggleEditable() {
	static bool old_edit_on;
	if (edit_on) {
		old_edit_on=true;
		edit_on=false; edit=NULL;
	} else {
		edit_on=old_edit_on;
	}
	start->Calculate();
}

void keyboard_esp_cb(int key/*, int x, int y*/) {
	if (key==WXK_F2) ProcessMenu(MO_SAVE);
	else if (key==WXK_F3) ProcessMenu(MO_FUNCTIONS);
	else if (key==WXK_F5) ProcessMenu(MO_DEBUG);
	else if (key==WXK_F9) ProcessMenu(MO_RUN);
	else if (key==WXK_F1) ProcessMenu(MO_HELP);
	else if (key==WXK_F7) { if (!debugging) ToggleEditable(); }
	else if (key==WXK_F11) main_window->ToggleFullScreen();
	else if (key==WXK_F12) ProcessMenu(MO_ZOOM_EXTEND);
	else if (edit) edit->EditSpecialLabel(key);
}

void FocusEntity(LineInfo *li) {
	if (li) { 
		if (li->proceso) {
			if (start!=li->proceso) SetProc(li->proceso);
			debug_current=li->entidad;
			if (li->entidad) {
				float fx=(win_w/2)/zoom;
				float fy=(win_h/2)/zoom;
				// que se vea el centro en x de la entidad
				int dx=-li->entidad->x+fx;
				if (dx<d_dx-fx||dx>d_dx+fy) d_dx=dx;
				if (li->entidad->bwl+li->entidad->bwr+fx/20<fx*2) { // si se puede ver todo el ancho... 
					// ..asegurar que se ven los bordes laterales
					int dx0=-(li->entidad->x-li->entidad->bwl-fx/40);
					int dx1=-(li->entidad->x+li->entidad->bwr+fx/40-2*fx);
					if (dx0>d_dx) d_dx=dx0;
					else if (dx1<d_dx) d_dx=dx1;
				}
				// que se vea el centro en y de la entidad
				int dy=-li->entidad->y+li->entidad->bh/2+fy;
				if (dy<d_dy-fy||dy>d_dy+fy) d_dy=dy;
			}
		}
		else if (!li->proceso) debug_current=NULL;
	} else debugging=NULL;
}

void SetModified( ) {
	if (modified) return;
	modified=true;
	if (!loading) NotifyModification();
}

