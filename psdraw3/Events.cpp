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
#include "ShapesBar.h"
#include "Trash.h"
#include "ProcessSelector.h"
using namespace std;

#define mouse_setted_delta 1000
static int mouse_setted_x,mouse_setted_y; // posicion del click que va a setear el mouse en una entidad cuando se mueva, con correccion de y y zoom aplicados
Entity *to_set_mouse=NULL; // lo que se va a setear en mouse cuando el cursor se mueva un poco si sigue apretado el botón

static Entity *DuplicateEntity(Entity *orig, bool and_next_one_too = false) {
	Entity *nueva=new Entity(orig->type,orig->label);
	nueva->variante=orig->variante;
	nueva->m_x=orig->m_x; nueva->m_y=orig->m_y;
	nueva->x=orig->x; nueva->y=orig->y;
	nueva->fx=orig->fx; nueva->fy=orig->fy;
	nueva->d_x=orig->d_x; nueva->d_y=orig->d_y;
	nueva->d_fx=orig->d_fx; nueva->d_fy=orig->d_fy;
	for(int i=0;i<orig->GetChildCount();i++) {
		if (orig->GetChild(i)) nueva->LinkChild(i,DuplicateEntity(orig->GetChild(i),true));
		else nueva->LinkChild(i,NULL);
	}
	if (and_next_one_too && orig->GetNext()) {
		nueva->LinkNext(DuplicateEntity(orig->GetNext(),true));
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
	if (main_window->IsIconized()) main_window->Iconize(false);
	else { main_window->Hide(); wxYield(); main_window->Show(); }
}

void idle_func() {
	ReadComm();
	const int _delta_t=25000;
	static int last=glutGet( GLUT_ELAPSED_TIME );
	int now=glutGet( GLUT_ELAPSED_TIME );
	if (now-last<_delta_t) {
		wxMicroSleep(_delta_t-(now-last));
		last=now;
	}
	d_zoom=1/((2*1/d_zoom+1/zoom)/3);
	Entity::AllIterator it = Entity::AllBegin();
	while (it!=Entity::AllEnd()) {
		it->Tick();
		++it;
	}
	
	if (process_selector->IsActive()) {
		process_selector->ProcessIddle();
	} else {
		if (mouse) {
			// la corrección de m_x es para que cuando la estructura se colapse, el punto
			// de agarre no quede fuera de la misma (ej, un segun con muchas opciones)
			mouse->m_x -= std::max(mouse->m_x-mouse->d_bwr,0);
			trash->Show();
			shapes_bar->Hide();
		} else {
			trash->Hide();
			if (edit_on) shapes_bar->Show();
			else         shapes_bar->Hide();
		}
		shapes_bar->ProcessIdle();
	}
	trash->ProcessIdle();
	
	canvas->Refresh();
}

void passive_motion_cb(int x, int y) {
	if (process_selector->IsActive()) {	
		process_selector->ProcessMotion(x,y); return; 
	} else {
		if (!win_h || !edit_on) return;
		if (mouse) {
			shapes_bar->Hide();
			return;
		}
		if (edit_on) {
			shapes_bar->ProcessMotion(x,y);
		}
	}
	cur_x=x; cur_y=win_h-y; canvas->Refresh();
}
void motion_cb(int x, int y) {
	trash->ProcessMotion(x,y);
	if (process_selector->IsActive()) {
		process_selector->ProcessMotion(x,y); return; 
	} else {
		fix_mouse_coords(x,y);
		if (to_set_mouse && (x-mouse_setted_x)*(x-mouse_setted_x)+(y-mouse_setted_y)*(y-mouse_setted_y)>mouse_setted_delta) { 
			if (to_set_mouse->type==ET_PROCESO) return; // no permitir mover "proceso" ni "finproceso"
			to_set_mouse->SetMouse(); Entity::CalculateAll();
		}
		if (selecting_zoom||selecting_entities) {
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
		if (trash->IsSelected() && mouse) {
			if (mouse->type!=ET_OPCION && (mouse->GetParent()||mouse->GetPrev())) {
				mouse->UnLink();
				Entity::CalculateAll();
			}
		}
	}
}

void ZoomExtend(int x0, int y0, int x1, int y1, float max) {
	if (x1<x0) { int aux=x1; x1=x0; x0=aux; }
	if (y0<y1) { int aux=y1; y1=y0; y0=aux; }
	if (x1-x0<10||y0-y1<10) return;
	int h=y0-y1, w=x1-x0;
	const int margin = 40;
	double zh=float(win_h-margin)/h; // zoom para ajustar alto
	double zw=float(win_w-shapes_bar->GetWidth()-margin)/w; // zoom para ajustar ancho
	if (zw>zh) zoom=zh; else zoom=zw; // ver cual tamaño manda
	if (zoom>max) zoom=max;
	d_dx=(win_w-shapes_bar->GetWidth())/zoom/2-(x1+x0)/2;
	d_dy=win_h/zoom/2-(y1+y0)/2/*+h/2/zoom*/;
}

void ProcessMenu(int op) {
	if (edit) edit->UnsetEdit();
	if (op==MO_ZOOM_EXTEND) {
		int h=0,wl=0,wr=0;
		Entity *real_start = start->GetTopEntity();
		real_start->Calculate(wl,wr,h); // calcular tamaño total
		ZoomExtend(real_start->x-wl,real_start->y,real_start->x+wr,real_start->y-h,1.5);
	} else if (op==MO_TOGGLE_FULLSCREEN) {
		main_window->ToggleFullScreen();
	} else if (op==MO_FUNCTIONS) {
		if (edit) edit->UnsetEdit();
		if (mouse) mouse->UnSetMouse();
		process_selector->Show();
	} else if (op==MO_SAVE||op==MO_RUN||op==MO_EXPORT||op==MO_DEBUG) {
		SendUpdate(op);
//	} else if (op==MO_SAVE_CLOSE) {
//		SendUpdate(); Salir();
	} else if (op==MO_CLOSE) {
		Salir();
	} else if (op==MO_HELP) {
		SendHelp();
	} else if (op==MO_CROP_LABELS) {
		Entity::enable_partial_text=!Entity::enable_partial_text;
		Entity::CalculateAll(true);
	} else if (op==MO_TOGGLE_COMMENTS) {
		Entity::show_comments=!Entity::show_comments;
		Entity::CalculateAll(true);
	} else if (op==MO_TOGGLE_COLORS) {
		Entity::shape_colors=!Entity::shape_colors;
	} else if (op==MO_CHANGE_STYLE) {
		Entity::nassi_shneiderman = !Entity::nassi_shneiderman;
		Entity::CalculateAll(true);
		ProcessMenu(MO_ZOOM_EXTEND);
	}
}	

void fix_mouse_coords(int &x, int &y) {
	y=win_h-y; y/=zoom; x/=zoom;
}

void mouse_dcb(int x, int y) {
	fix_mouse_coords(x,y);
	Entity::AllIterator it = Entity::AllBegin();
	while (it!=Entity::AllEnd()) {
		if (it->CheckMouse(x,y)) {
			if (it->type==ET_PROCESO && it!=start) break; // para no editar el "FinProceso"
			it->SetEdit();
			return;
		}
		++it;
	}
}

void FinishMultipleSelection(int x0, int y0, int x1, int y1) {
	if (x0>x1) { int aux = x0; x0 = x1; x1 = aux; }
	if (y0<y1) { int aux = y0; y0 = y1; y1 = aux; }
	selecting_entities = false;
	// encontrar la primera que entra en la selección
	Entity *aux = start;
	do {
		if (aux->IsInside(x0,y0,x1,y1))
			break;
		aux = Entity::NextEntity(aux);
	} while (aux);
	if (!aux) return;
	// crear la entidad de seleccion y poner la primera como primer hija
	Entity *selection = new Entity(ET_SELECTION,""), 
		   *aux_prev = aux->GetPrev(), *aux_parent = aux->GetParent();
	
	selection->d_fx = (x0+x1)/2; selection->d_fy = y0;
	selection->d_h = (y0-y1); selection->d_w = x1-x0;
	
	int aux_chid = aux->GetChildId(); aux->UnLink(); 
	if (aux_prev) aux_prev->LinkNext(selection);
	else aux_parent->LinkChild(aux_chid,selection);
	selection->LinkChild(0,aux);
	// ver cuantas "next" también entran en la selección
	while (selection->GetNext() && selection->GetNext()->IsInside(x0,y0,x1,y1)) {
		Entity *sel_next = selection->GetNext();
		sel_next->UnLink();
		aux->LinkNext(sel_next);
		aux = sel_next;
	}
}

void mouse_cb(int button, int state, int x, int y) {
	if (process_selector->IsActive()) { process_selector->ProcessClick(button,state,x,y); return; }
	to_set_mouse=NULL;
	if (!panning && !selecting_zoom && !selecting_entities)
		if (shapes_bar->ProcessMouse(button,state,x,y)) return;
	fix_mouse_coords(x,y);
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
		// click en una entidad? izquierdo=mover, derecho=editar label
		if (mouse) mouse->UnSetMouse();
		Entity::AllIterator it = Entity::AllBegin();
		while (it!=Entity::AllEnd()) {
			if (it->CheckMouse(x,y)) {
				if (it->type==ET_PROCESO && it!=start) break; // para no editar el "FinProceso"
				if (button==ZMB_RIGHT) {
					it->SetEdit(); return;
				} else {
					if (it->type!=ET_PROCESO && canvas->GetModifiers()==MODIFIER_SHIFT) { // no duplicar "Proceso..." y "FinProceso"
						mouse = DuplicateEntity(it.GetPtr());
						it.SetPtr(mouse); it->SetEdit();
					} 
					if (it.GetPtr()!=edit) {
						if (edit) edit->UnsetEdit();
					}
					to_set_mouse = it.GetPtr(); mouse_setted_x=x; mouse_setted_y=y; // aux->SetMouse(); retrasado
					if (it->type==ET_AUX_PARA) to_set_mouse=it->GetParent(); // para que no haga drag del hijo del para, sino de todo el para completo
					return;
				}
				break;
			}
			++it;
		}
		if (button==ZMB_LEFT && canvas->GetModifiers()==MODIFIER_SHIFT) {
			cur_x=m_x0=x; cur_y=m_y0=y; selecting_entities=true;
		} else {
			m_x0=x; m_y0=y; panning=true;
		}
	} else {
		if (button==ZMB_LEFT) {
			if (mouse) {
				if (trash->IsSelected() && mouse->type==ET_OPCION) {
					Entity *p=mouse->GetParent();
					mouse->UnLink(); 
					p->Calculate();
				} 
			}
			if (mouse) mouse->UnSetMouse();
			if (selecting_entities) {
				FinishMultipleSelection(m_x0,m_y0,cur_x,cur_y);
			}
//			// doble click (por alguna extraña razon en mi wx un doble click genera un evento de down y dos de up)
//			Entity *aux=start;
//			do {
//				if (aux->CheckMouse(x,y)) {
//					static int last_click_time=0;
//					static Entity *last_click_mouse=NULL;
//					int click_time=glutGet(GLUT_ELAPSED_TIME);
//					if (click_time-last_click_time<500 && (last_click_mouse==aux ||  (aux->type==ET_PARA && aux->parent==last_click_mouse)) ) aux->SetEdit();
//					last_click_mouse=aux; last_click_time=click_time;
//				}
//				aux=aux->all_next;
//			} while (aux!=start);
		} else if (button==ZMB_MIDDLE) {
			ZoomExtend(m_x0-d_dx,m_y0-d_dy,x-d_dx,y-d_dy);
			selecting_zoom=false;
			return;
		}
		panning=false;
		Entity::CalculateAll();
	}
}

void keyboard_cb(unsigned char key/*, int x, int y*/) {
	if (key=='\t') shapes_bar->ToggleFixed();
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
		edit_on=false; if (edit) edit->UnsetEdit();
	} else {
		edit_on=old_edit_on;
	}
	Entity::CalculateAll();
}

void keyboard_esp_cb(int key/*, int x, int y*/) {
	if (key==WXK_F2) ProcessMenu(MO_SAVE);
	else if (key==WXK_F3) ProcessMenu(MO_FUNCTIONS);
	else if (key==WXK_F5) ProcessMenu(MO_DEBUG);
	else if (key==WXK_F9) ProcessMenu(MO_RUN);
	else if (key==WXK_F1) ProcessMenu(MO_HELP);
	else if (key==WXK_F7) { if (!debugging) ToggleEditable(); }
	else if (key==WXK_F11) ProcessMenu(MO_TOGGLE_FULLSCREEN);
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

