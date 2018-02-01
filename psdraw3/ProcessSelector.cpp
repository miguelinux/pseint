#include <cstddef>
#include "ProcessSelector.h"
#include "Trash.h"
#include "Global.h"
#include "Text.h"
#include "Load.h"
#include "Events.h"
#include "Draw.h"

#define no_selection -1

ProcessSelector *process_selector = NULL;

ProcessSelector::ProcessSelector() 
	: m_state(0), m_selection(no_selection), 
	  m_anim_base(0), m_anim_delta(0)
{
	
}

void ProcessSelector::Show ( ) {
	m_anim_base = m_anim_delta = 0;
	m_state = 1; 
}

void ProcessSelector::Hide ( ) {
	m_state = 0;
	trash->Hide();
}

void ProcessSelector::Draw ( ) {
	glLineWidth(menu_line_width);
	DrawTextRaster(color_menu,10,win_h-25,
				   (lang[LS_PREFER_FUNCION]
					? (lang[LS_PREFER_ALGORITMO]
					   ? "Seleccione un algoritmo/funcion para visualizar:"
					   : "Seleccione un proceso/funcion para visualizar:")
					: (lang[LS_PREFER_ALGORITMO]
					   ? "Seleccione un proceso/subproceso para visualizar:"
					   : "Seleccione un algoritmo/subalgoritmo para visualizar:") ) );
	
	int base=win_h-m_anim_base, delta=m_anim_delta;
	glColor3fv(color_menu); 
	glBegin(GL_LINES); 
	glVertex2i(0,base); glVertex2i(win_w,base);
	glEnd();
	for(int i=0;i<=int(procesos.size()-(edit_on?0:1));i++) {
		
		if (m_selection==i) {
			glColor3fv(color_shape[ET_PROCESO]);
			glBegin(GL_QUADS);
			glVertex2i(0,base);
			glVertex2i(win_w,base);
			glVertex2i(win_w,base-delta);
			glVertex2i(0,base-delta);
			glEnd();
		}
		base-=delta;
		
		if (i==int(procesos.size())) {
			DrawTextRaster(m_selection==i?color_selection:color_menu,20,base+10,
						   lang[LS_PREFER_FUNCION]?"Agregar Nueva Funcion"
						   :(lang[LS_PREFER_ALGORITMO]?"Agregar Nuevo SubAlgoritmo"
							 :"Agregar Nuevo SubProceso") );
		} else {
			
			string &s=procesos[i]->label;
			int l=s.size(),p=0; size_t f=s.find('<');
			if (f==string::npos) f=s.find('='); else f++;
			if (f==string::npos) f=0; else f++;
			int t=f; while (t<l && s[t]!=' ' && s[t]!='(') t++;
			
			glColor3fv(m_selection==i?color_selection:color_menu);
			glPushMatrix();
			int px=20, py=base+10;
			if (m_state==3 && m_selection==i) glTranslated(cur_x+(px-m_x0),cur_y+(py-m_y0),0);
			else glTranslated(px,py,0);
			glScaled(.08,.12,.1);
			
			begin_texto();
			string &sp=procesos[i]->lpre;
			p=0; l=sp.size();
			while (p<l)
				dibujar_caracter(sp[p++]);
			
			p=0; l=s.size();
			while (p<l) {
				if (p==int(f)) glLineWidth(2);
				if (p==t) glLineWidth(1);
				dibujar_caracter(s[p++],true);
			}
			end_texto();
			glPopMatrix();		
		}
		
		glLineWidth(1);
		glColor3fv(color_menu); 
		glBegin(GL_LINES); 
		glVertex2i(0,base); glVertex2i(win_w,base);
		glEnd();
	}
	glEnd();
	mouse_cursor = Z_CURSOR_INHERIT;
	trash->Draw();
}

void ProcessSelector::ProcessMotion (int x, int y) {
	if (m_state) {
		cur_x = x; cur_y = win_h-y;
		if (m_state<3 && m_anim_delta) {
			m_selection=(y-m_anim_base)/m_anim_delta;
			if (m_selection<0||m_selection>int(procesos.size()-(edit_on?0:1))) m_selection=no_selection;
		}
		return;
	}
}

void ProcessSelector::ProcessClick (int button, int state, int x, int y) {
	if (m_selection==int(procesos.size())) {
		CreateEmptyProc(lang[LS_PREFER_FUNCION]?"Funcion":(lang[LS_PREFER_ALGORITMO]?"SubAlgoritmo":"SubProceso"));
	}
	if (m_selection!=no_selection) {
		if (state==ZMB_DOWN) {
			m_state=3;
			if (edit_on) trash->Show();
			cur_x=m_x0=x; cur_y=m_y0=win_h-y;
		} else if (trash->IsSelected()) {
			if (edit_on && (procesos[m_selection]->lpre!="Proceso "&&procesos[m_selection]->lpre!="Algoritmo "))
				procesos.erase(procesos.begin()+m_selection); // no lo quita de la memoria, solo del arreglo, con eso alcanza, algun día corregiré el memory leak
			m_state=2;
			trash->Hide();
		} else {
			SetProc(procesos[m_selection]);
			m_state=0;
			trash->Hide();
			if (button==ZMB_RIGHT) start->SetEdit();
		}
	}
}

void ProcessSelector::Initialize ( ) {
	process_selector = new ProcessSelector();
}

void ProcessSelector::ProcessIddle ( ) {
	interpolate_good(m_anim_base,40);
	interpolate_good(m_anim_delta,30);
}

