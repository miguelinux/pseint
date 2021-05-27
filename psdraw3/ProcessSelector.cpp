#include <cstddef>
#include "ProcessSelector.h"
#include "Trash.h"
#include "Global.h"
#include "Text.h"
#include "Load.h"
#include "Events.h"
#include "Draw.h"

#define no_selection -1

ProcessSelector *g_process_selector = nullptr;

ProcessSelector::ProcessSelector() 
	: m_state(0), m_selection(no_selection), 
	  m_anim_base(0), m_anim_delta(0)
{
	
}

void ProcessSelector::Show ( ) {
	m_anim_base = m_anim_delta = 0;
	m_state = 1; m_selection = no_selection;
}

void ProcessSelector::Hide ( ) {
	m_state = 0;
	g_trash->Hide();
}

void ProcessSelector::Draw ( ) {
	glClearColor(g_colors.menu_back[0],g_colors.menu_back[1],g_colors.menu_back[2],1);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glLineWidth(g_constants.menu_line_width);
	DrawTextRaster(g_colors.menu_front,10,g_view.win_h-25,
				   (g_lang[LS_PREFER_FUNCION]
					? (g_lang[LS_PREFER_ALGORITMO]
					   ? "Seleccione un algoritmo/funcion para visualizar:"
					   : "Seleccione un proceso/funcion para visualizar:")
					: (g_lang[LS_PREFER_ALGORITMO]
					   ? "Seleccione un proceso/subproceso para visualizar:"
					   : "Seleccione un algoritmo/subalgoritmo para visualizar:") ) );
	
	int base=g_view.win_h-m_anim_base, delta=m_anim_delta;
	glColor3fv(g_colors.menu_front); 
	glBegin(GL_LINES); 
	glVertex2i(0,base); glVertex2i(g_view.win_w,base);
	glEnd();
	for(int i=0;i<=int(g_code.procesos.size()-(g_state.edit_on?0:1));i++) {
		
		if (m_selection==i) {
			glColor3fv(g_colors.menu_sel_back);
			glBegin(GL_QUADS);
			glVertex2i(0,base);
			glVertex2i(g_view.win_w,base);
			glVertex2i(g_view.win_w,base-delta);
			glVertex2i(0,base-delta);
			glEnd();
		}
		base-=delta;
		
		if (i==int(g_code.procesos.size())) {
			DrawTextRaster(/*m_selection==i?g_colors.selection:*/g_colors.menu_front,20,base+10,
						   g_lang[LS_PREFER_FUNCION]?"Agregar Nueva Funcion"
						   :(g_lang[LS_PREFER_ALGORITMO]?"Agregar Nuevo SubAlgoritmo"
							 :"Agregar Nuevo SubProceso") );
		} else {
			
			const float *color = /*m_selection==i?g_colors.selection:*/g_colors.menu_front;
			glColor3fv(color);
			glPushMatrix();
			int px=20, py=base+10;
			if (m_state==3 and m_selection==i) glTranslated(g_state.cur_x+(px-g_state.m_x0),g_state.cur_y+(py-g_state.m_y0),0);
			else glTranslated(px,py,0);
			double sf = g_config.big_icons ? 1.5 : 1; glScaled(.08*sf,.12*sf,.1*sf);
			
			glLineWidth(g_config.big_icons?2:1);
			begin_texto();
			{
				string &sp = g_code.procesos[i]->lpre;
				int p=0; int l=sp.size();
				while (p<l)
					dibujar_caracter(sp[p++]);
			}
			
			{
				string &s = g_code.procesos[i]->label;
				int l=s.size(),p=0; size_t f=s.find('<');
				if (f==string::npos) f=s.find('='); else f++;
				if (f==string::npos) f=0; else f++;
				int t=f; while (t<l && s[t]!=' ' && s[t]!='(') t++;
				while (p<l) {
					if (p==int(f)) glColor3fv(g_colors.menu_front_bold);
					else if (p==t) glColor3fv(color);
					dibujar_caracter(s[p++]);
				}
			}
			
			end_texto();
			glPopMatrix();		
		}
		
		glLineWidth(1);
		glColor3fv(g_colors.menu_front); 
		glBegin(GL_LINES); 
		glVertex2i(0,base); glVertex2i(g_view.win_w,base);
		glEnd();
	}
	glEnd();
	mouse_cursor = Z_CURSOR_INHERIT;
	g_trash->Draw();
}

void ProcessSelector::ProcessMotion (int x, int y) {
	if (m_state) {
		g_state.cur_x = x; g_state.cur_y = g_view.win_h-y;
		if (m_state<3 && m_anim_delta) {
			m_selection=(y-m_anim_base)/m_anim_delta;
			if (y<m_anim_base or m_selection<0 or m_selection>int(g_code.procesos.size()-(g_state.edit_on?0:1))) m_selection=no_selection;
			
		}
		return;
	}
}

void ProcessSelector::ProcessClick (int button, int state, int x, int y) {
	if (m_selection==int(g_code.procesos.size())) {
		CreateEmptyProc(g_lang[LS_PREFER_FUNCION]?"Funcion":(g_lang[LS_PREFER_ALGORITMO]?"SubAlgoritmo":"SubProceso"));
	}
	if (m_selection!=no_selection) {
		if (state==ZMB_DOWN) {
			m_state=3;
			if (g_state.edit_on) g_trash->Show();
			g_state.cur_x = g_state.m_x0 = x; g_state.cur_y = g_state.m_y0 = g_view.win_h-y;
		} else if (g_trash->IsSelected()) {
			if (g_state.edit_on and (g_code.procesos[m_selection]->lpre!="Proceso " and g_code.procesos[m_selection]->lpre!="Algoritmo "))
				g_code.procesos.erase(g_code.procesos.begin()+m_selection); // no lo quita de la memoria, solo del arreglo, con eso alcanza, algun día corregiré el memory leak
			m_state=2;
			g_trash->Hide();
		} else {
			SetProc(g_code.procesos[m_selection]);
			m_state=0;
			g_trash->Hide();
			if (button==ZMB_RIGHT) g_code.start->SetEdit();
		}
	}
}

void ProcessSelector::Initialize ( ) {
	g_process_selector = new ProcessSelector();
}

void ProcessSelector::ProcessIddle ( ) {
	if (g_config.big_icons) {
		interpolate_good(m_anim_base,50);
		interpolate_good(m_anim_delta,40);
	} else {
		interpolate_good(m_anim_base,40);
		interpolate_good(m_anim_delta,30);
	}
}

