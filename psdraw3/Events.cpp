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
#include <wx/timer.h>
using namespace std;

#define mouse_setted_delta 1000
static int mouse_setted_x,mouse_setted_y; // posicion del click que va a setear el mouse en una entidad cuando se mueva, con correccion de y y zoom aplicados
Entity *to_set_mouse = nullptr; // lo que se va a setear en mouse cuando el cursor se mueva un poco si sigue apretado el botón

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
		else nueva->LinkChild(i,nullptr);
	}
	if (and_next_one_too && orig->GetNext()) {
		nueva->LinkNext(DuplicateEntity(orig->GetNext(),true));
	}
	return nueva;
}

void Salir(bool force) {
	if ((not force) and g_state.modified) {
		g_main_window->AskForExit();
		return;
	}
	CloseComm();
	wxExit();
}

void reshape_cb (int w, int h) {
	if (w==0||h==0) return;
	if (g_view.win_w!=0 and g_view.win_h!=0) {
		double zw=double(w)/g_view.win_w;
		double zh=double(h)/g_view.win_h;
		g_view.d_dx*=zw/zh;
//		if (zh>zw) zoom*=zh; else zoom*=zw;
		g_view.zoom*=zh;
	}	
	g_view.win_h=h; g_view.win_w=w;
	glViewport(0,0,w,h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(0,w,0,h);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

void Raise() {
	if (g_main_window->IsIconized()) g_main_window->Iconize(false);
	else { g_main_window->Hide(); wxYield(); g_main_window->Show(); }
}

void idle_func() {
	ReadComm();
	const int _delta_t=25000;
	static long long last = wxGetLocalTimeMillis().GetValue();
	long long now = wxGetLocalTimeMillis().GetValue();
	if (now-last<_delta_t) {
		wxMicroSleep(_delta_t-(now-last));
		last=now;
	}
	g_view.d_zoom = 1/((2*1/g_view.d_zoom+1/g_view.zoom)/3);
	Entity::AllIterator it = Entity::AllBegin();
	while (it!=Entity::AllEnd()) {
		it->Tick();
		++it;
	}
	
	if (g_process_selector->IsActive()) {
		g_process_selector->ProcessIddle();
	} else {
		if (g_state.mouse) {
			// la corrección de m_x es para que cuando la estructura se colapse, el punto
			// de agarre no quede fuera de la misma (ej, un segun con muchas opciones)
			g_state.mouse->m_x -= std::max(g_state.mouse->m_x-g_state.mouse->d_bwr,0);
			g_trash->Show();
			g_shapes_bar->Hide();
		} else {
			g_trash->Hide();
			if (g_state.edit_on) g_shapes_bar->Show();
			else                 g_shapes_bar->Hide();
		}
		g_shapes_bar->ProcessIdle();
	}
	g_trash->ProcessIdle();
	
	g_canvas->Refresh();
}

void passive_motion_cb(int x, int y) {
	if (g_process_selector->IsActive()) {	
		g_process_selector->ProcessMotion(x,y); return; 
	} else {
		if ((not g_view.win_h) or (not g_state.edit_on)) return;
		if (g_state.mouse) {
			g_shapes_bar->Hide();
			return;
		}
		if (g_state.edit_on) {
			g_shapes_bar->ProcessMotion(x,y);
		}
	}
	g_state.cur_x = x; g_state.cur_y = g_view.win_h-y; g_canvas->Refresh();
}
void motion_cb(int x, int y) {
	g_trash->ProcessMotion(x,y);
	if (g_process_selector->IsActive()) {
		g_process_selector->ProcessMotion(x,y); return; 
	} else {
		fix_mouse_coords(x,y);
		if (to_set_mouse && (x-mouse_setted_x)*(x-mouse_setted_x)+(y-mouse_setted_y)*(y-mouse_setted_y)>mouse_setted_delta) { 
			if (to_set_mouse->type==ET_PROCESO) return; // no permitir mover "proceso" ni "finproceso"
			to_set_mouse->SetMouse(); Entity::CalculateAll();
		}
		if (g_state.selecting_zoom or g_state.selecting_entities) {
			g_state.cur_x = x; g_state.cur_y = y;
			return;
		}
		if (g_view.panning) { 
			g_view.d_dx += x-g_state.m_x0; g_state.m_x0 = x;
			g_view.d_dy += y-g_state.m_y0; g_state.m_y0 = y;
		} 
		if (g_state.mouse) { 
			g_state.cur_y = y; g_state.cur_x = g_state.mouse->d_x;
			g_state.mouse->d_x =x-g_state.mouse->m_x;
			g_state.mouse->d_y =y-g_state.mouse->m_y;
			g_state.mouse->d_fx =x-g_state.mouse->m_x;
			g_state.mouse->d_fy =y-g_state.mouse->m_y;
		}
		if (g_trash->IsSelected() and g_state.mouse) {
			if (g_state.mouse->type!=ET_OPCION and (g_state.mouse->GetParent() or g_state.mouse->GetPrev())) {
				g_state.mouse->UnLink();
				Entity::CalculateAll();
			}
		}
	}
}

void ZoomExtend(int x0, int y0, int x1, int y1, double max) {
	if (x1<x0) { int aux=x1; x1=x0; x0=aux; }
	if (y0<y1) { int aux=y1; y1=y0; y0=aux; }
	if (x1-x0<10||y0-y1<10) return;
	int h=y0-y1, w=x1-x0;
	const int margin = 40;
	double zh = double(g_view.win_h-margin)/h; // zoom para ajustar alto
	double zw = double(g_view.win_w-g_shapes_bar->GetWidth()-margin)/w; // zoom para ajustar ancho
	if (zw>zh) g_view.zoom = zh; else g_view.zoom = zw; // ver cual tamaño manda
	if (g_view.zoom>max) g_view.zoom = max;
	g_view.d_dx=(g_view.win_w-g_shapes_bar->GetWidth())/g_view.zoom/2-(x1+x0)/2;
	g_view.d_dy=g_view.win_h/g_view.zoom/2-(y1+y0)/2/*+h/2/zoom*/;
}

void ProcessMenu(int op) {
	if (g_state.edit) g_state.edit->UnsetEdit();
	if (op==MO_ZOOM_EXTEND) {
		int h=0,wl=0,wr=0;
		Entity *real_start = g_code.start->GetTopEntity();
		real_start->Calculate(wl,wr,h); // calcular tamaño total
		ZoomExtend(real_start->x-wl,real_start->y,real_start->x+wr,real_start->y-h,1.5);
	} else if (op==MO_TOGGLE_FULLSCREEN) {
		g_main_window->ToggleFullScreen();
	} else if (op==MO_FUNCTIONS) {
		if (g_state.edit) g_state.edit->UnsetEdit();
		if (g_state.mouse) g_state.mouse->UnSetMouse();
		g_process_selector->Show();
	} else if (op==MO_SAVE||op==MO_RUN||op==MO_EXPORT||op==MO_DEBUG) {
		SendUpdate(op);
//	} else if (op==MO_SAVE_CLOSE) {
//		SendUpdate(); Salir();
	} else if (op==MO_CLOSE) {
		Salir();
	} else if (op==MO_HELP) {
		SendHelp();
	} else if (op==MO_CROP_LABELS) {
		g_config.enable_partial_text = not g_config.enable_partial_text;
		Entity::CalculateAll(true);
	} else if (op==MO_TOGGLE_COMMENTS) {
		g_config.show_comments= not g_config.show_comments;
		Entity::CalculateAll(true);
	} else if (op==MO_TOGGLE_COLORS) {
		g_config.shape_colors = not g_config.shape_colors;
	} else if (op==MO_CHANGE_STYLE) {
		g_config.nassi_shneiderman = not g_config.nassi_shneiderman;
		Entity::CalculateAll(true);
		ProcessMenu(MO_ZOOM_EXTEND);
	}
}	

void fix_mouse_coords(int &x, int &y) {
	y = g_view.win_h-y; y /= g_view.zoom; x /= g_view.zoom;
}

void mouse_dcb(int x, int y) {
	fix_mouse_coords(x,y);
	Entity::AllIterator it = Entity::AllBegin();
	while (it!=Entity::AllEnd()) {
		if (it->CheckMouse(x,y)) {
			if (it->type==ET_PROCESO and it!=g_code.start) break; // para no editar el "FinProceso"
			it->SetEdit();
			return;
		}
		++it;
	}
}

void FinishMultipleSelection(int x0, int y0, int x1, int y1) {
	if (x0>x1) { int aux = x0; x0 = x1; x1 = aux; }
	if (y0<y1) { int aux = y0; y0 = y1; y1 = aux; }
	g_state.selecting_entities = false;
	// encontrar la primera que entra en la selección
	Entity *aux = g_code.start;
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
	if (g_process_selector->IsActive()) { g_process_selector->ProcessClick(button,state,x,y); return; }
	to_set_mouse = nullptr;
	if ((not g_view.panning) and (not g_state.selecting_zoom) and (not g_state.selecting_entities))
		if (g_shapes_bar->ProcessMouse(button,state,x,y)) return;
	fix_mouse_coords(x,y);
	if (button==ZMB_WHEEL_DOWN||button==ZMB_WHEEL_UP) {
		double k = g_canvas->GetModifiers()==MODIFIER_SHIFT ? 1.01 : 1.10;
		double f=button==ZMB_WHEEL_UP?1.0/k:k;
		g_view.zoom *= f;
		double dx = x/f-x, dy = y/f-y;
		g_view.d_dx += dx; g_view.d_dy += dy;
	} else if (state==ZMB_DOWN) {
		if (button==ZMB_MIDDLE) { // click en el menu
			g_state.cur_x = g_state.m_x0 = x; g_state.cur_y = g_state.m_y0 = y; 
			g_state.selecting_zoom = true;
			return;
		}
		// click en una entidad? izquierdo=mover, derecho=editar label
		if (g_state.mouse) g_state.mouse->UnSetMouse();
		Entity::AllIterator it = Entity::AllBegin();
		while (it!=Entity::AllEnd()) {
			if (it->CheckMouse(x,y)) {
				if (it->type==ET_PROCESO && it!=g_code.start) break; // para no editar el "FinProceso"
				if (button==ZMB_RIGHT) {
					it->SetEdit(); return;
				} else {
					if (it->type!=ET_PROCESO and g_canvas->GetModifiers()==MODIFIER_SHIFT) { // no duplicar "Proceso..." y "FinProceso"
						g_state.mouse = DuplicateEntity(it.GetPtr());
						it.SetPtr(g_state.mouse); it->SetEdit();
					} 
					if (it.GetPtr()!=g_state.edit) {
						if (g_state.edit) g_state.edit->UnsetEdit();
					}
					to_set_mouse = it.GetPtr(); mouse_setted_x=x; mouse_setted_y=y; // aux->SetMouse(); retrasado
					if (it->type==ET_AUX_PARA) to_set_mouse=it->GetParent(); // para que no haga drag del hijo del para, sino de todo el para completo
					return;
				}
				break;
			}
			++it;
		}
		if (button==ZMB_LEFT and g_canvas->GetModifiers()==MODIFIER_SHIFT) {
			g_state.cur_x = g_state.m_x0 = x; g_state.cur_y = g_state.m_y0 = y;
			g_state.selecting_entities=true;
		} else {
			g_state.m_x0 = x; g_state.m_y0=y; g_view.panning = true;
		}
	} else {
		if (button==ZMB_LEFT) {
			if (g_state.mouse) {
				if (g_trash->IsSelected() and g_state.mouse->type==ET_OPCION) {
					Entity *p = g_state.mouse->GetParent();
					g_state.mouse->UnLink(); 
					p->Calculate();
				} 
			}
			if (g_state.mouse) g_state.mouse->UnSetMouse();
			if (g_state.selecting_entities) {
				FinishMultipleSelection(g_state.m_x0,g_state.m_y0,g_state.cur_x,g_state.cur_y);
			}
//			// doble click (por alguna extraña razon en mi wx un doble click genera un evento de down y dos de up)
//			Entity *aux=g_code.start;
//			do {
//				if (aux->CheckMouse(x,y)) {
//					static int last_click_time=0;
//					static Entity *last_click_mouse=nullptr;
//					int click_time=glutGet(GLUT_ELAPSED_TIME);
//					if (click_time-last_click_time<500 && (last_click_mouse==aux ||  (aux->type==ET_PARA && aux->parent==last_click_mouse)) ) aux->SetEdit();
//					last_click_mouse=aux; last_click_time=click_time;
//				}
//				aux=aux->all_next;
//			} while (aux!=g_code.start);
		} else if (button==ZMB_MIDDLE) {
			ZoomExtend(g_state.m_x0-g_view.d_dx,g_state.m_y0-g_view.d_dy,x-g_view.d_dx,y-g_view.d_dy);
			g_state.selecting_zoom = false;
			return;
		}
		g_view.panning = false;
		Entity::CalculateAll();
	}
}

void keyboard_cb(unsigned char key/*, int x, int y*/) {
	if (key=='\t') g_shapes_bar->ToggleFixed();
	if (not g_state.edit) {
		if (key==27) Salir();
		return;
	} else {
//		if (g_canvas->GetModifiers()&MODIFIER_CTRL) return;
		g_state.edit->EditLabel(key);
	}
}

void ToggleEditable() {
	static bool old_edit_on;
	if (g_state.edit_on) {
		old_edit_on=true;
		g_state.edit_on=false; if (g_state.edit) g_state.edit->UnsetEdit();
	} else {
		g_state.edit_on = old_edit_on;
	}
	Entity::CalculateAll();
}

void keyboard_esp_cb(int key/*, int x, int y*/) {
	if (key==WXK_F2) ProcessMenu(MO_SAVE);
	else if (key==WXK_F3) ProcessMenu(MO_FUNCTIONS);
	else if (key==WXK_F4) ProcessMenu(MO_CLOSE);
	else if (key==WXK_F5) ProcessMenu(MO_DEBUG);
	else if (key==WXK_F9) ProcessMenu(MO_RUN);
	else if (key==WXK_F1) ProcessMenu(MO_HELP);
	else if (key==WXK_F7) { if (not g_state.debugging) ToggleEditable(); }
	else if (key==WXK_F11) ProcessMenu(MO_TOGGLE_FULLSCREEN);
	else if (key==WXK_F12) ProcessMenu(MO_ZOOM_EXTEND);
	else if (g_state.edit) g_state.edit->EditSpecialLabel(key);
}

void FocusEntity(LineInfo *li) {
	if (not li) g_state.debug_current = nullptr;
	if (li->proceso) {
		if (g_code.start!=li->proceso) SetProc(li->proceso);
		g_state.debug_current = li->entidad;
		if (li->entidad) {
			double fx=(g_view.win_w/2)/g_view.zoom;
			double fy=(g_view.win_h/2)/g_view.zoom;
			// que se vea el centro en x de la entidad
			int dx=-li->entidad->x+fx;
			if (dx<g_view.d_dx-fx or dx>g_view.d_dx+fy) g_view.d_dx = dx;
			if (li->entidad->bwl+li->entidad->bwr+fx/20<fx*2) { // si se puede ver todo el ancho... 
				// ..asegurar que se ven los bordes laterales
				int dx0 = -(li->entidad->x-li->entidad->bwl-fx/40);
				int dx1 = -(li->entidad->x+li->entidad->bwr+fx/40-2*fx);
				if (dx0>g_view.d_dx) g_view.d_dx = dx0;
				else if (dx1<g_view.d_dx) g_view.d_dx = dx1;
			}
			// que se vea el centro en y de la entidad
			int dy=-li->entidad->y+li->entidad->bh/2+fy;
			if (dy<g_view.d_dy-fy or dy>g_view.d_dy+fy) g_view.d_dy = dy;
		}
	}
	// este else esconde la flecha cuando pasamos por una instruccion
	// que no tiene entidad en el diagrama (como un finsi)
//	else if (not li->proceso) debug_current=nullptr; 
}

void SetModified( ) {
	if (g_state.modified) return;
	g_state.modified = true;
	if (not g_state.loading) NotifyModification();
}

