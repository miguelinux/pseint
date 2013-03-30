#ifdef _FOR_EXPORT
#include <GLtoWX.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <cstdlib>
#include "Entity.h"
#include "Global.h"
#include "Draw.h"
using namespace std;

void Entity::DrawShapeSolid(const float *color,int x, int y, int w, int h) {
	glColor3fv(color);
	glBegin(GL_QUAD_STRIP);
	if (type==ET_PARA) {
		w=w/2; h=h/2; y-=h;
		for(int i=1;i<circle_steps;i+=2) { 
			glVertex2i(x,y);
			glVertex2i(x+cosx[i-1]*w,y+sinx[i-1]*h);
			glVertex2i(x+cosx[i+1]*w,y+sinx[i+1]*h);
			glVertex2i(x+cosx[i]*w,y+sinx[i]*h);
		}
	} else if (type==ET_PROCESO) {
		glVertex2i(x-w/2+3*h/8,y-5*h/8); glVertex2i(x-w/2+3*h/8,y-3*h/8);
		glVertex2i(x-w/2+h/2,y-1*h/8); glVertex2i(x-w/2+h/2,y-7*h/8);
		glVertex2i(x-w/2+h,y); glVertex2i(x-w/2+h,y-h);
		glVertex2i(x+w/2-h,y); glVertex2i(x+w/2-h,y-h); 
		glVertex2i(x+w/2-h/2,y-1*h/8);
		glVertex2i(x+w/2-h/2,y-7*h/8);
		glVertex2i(x+w/2-3*h/8,y-3*h/8);
		glVertex2i(x+w/2-3*h/8,y-5*h/8);
	} else if (type==ET_REPETIR||type==ET_MIENTRAS||type==ET_SI) {
		glVertex2i(x,y); glVertex2i(x+w/2,y-h/2);
		glVertex2i(x-w/2,y-h/2); glVertex2i(x,y-h);
	} else if (type==ET_ESCRIBIR||type==ET_LEER) {
		glVertex2i(x+w/2+margin,y); glVertex2i(x-w/2+margin,y); 
		glVertex2i(x+w/2-margin,y-h); glVertex2i(x-w/2-margin,y-h);
	} else if (type==ET_SEGUN) {
		glVertex2i(x,y); glVertex2i(x+w/2,y-h); glVertex2i(x-w/2,y-h); glVertex2i(x,y-h);
	} else {
		glVertex2i(x-w/2,y); glVertex2i(x+w/2,y);
		glVertex2i(x-w/2,y-h); glVertex2i(x+w/2,y-h);
	}
	glEnd();
}

void Entity::DrawShapeBorder(const float *color,int x, int y, int w, int h) {
	glColor3fv(color);
	glBegin(GL_LINE_LOOP);
	if (type==ET_PARA) {
		w=w/2; h=h/2; y-=h;
		for(int i=0;i<circle_steps;i++) { 
			glVertex2i(x+cosx[i]*w,y+sinx[i]*h);
			glVertex2i(x+cosx[i+1]*w,y+sinx[i+1]*h);
		}
	} else if (type==ET_PROCESO) {
		glVertex2i(x-w/2+h,y); glVertex2i(x+w/2-h,y);
		glVertex2i(x+w/2-h/2,y-1*h/8);
		glVertex2i(x+w/2-3*h/8,y-3*h/8);
		glVertex2i(x+w/2-3*h/8,y-5*h/8);
		glVertex2i(x+w/2-h/2,y-7*h/8);
		glVertex2i(x+w/2-h,y-h); glVertex2i(x-w/2+h,y-h);
		glVertex2i(x-w/2+h/2,y-7*h/8);
		glVertex2i(x-w/2+3*h/8,y-5*h/8);
		glVertex2i(x-w/2+3*h/8,y-3*h/8);
		glVertex2i(x-w/2+h/2,y-1*h/8);
	} else if (type==ET_REPETIR||type==ET_MIENTRAS||type==ET_SI) {
		glVertex2i(x,y); glVertex2i(x+w/2,y-h/2);
		glVertex2i(x,y-h); glVertex2i(x-w/2,y-h/2);
	} else if (type==ET_ESCRIBIR||type==ET_LEER) {
		glVertex2i(x-w/2+margin,y); glVertex2i(x+w/2+margin,y);
		glVertex2i(x+w/2-margin,y-h); glVertex2i(x-w/2-margin,y-h);
	} else if (type==ET_SEGUN) {
		glVertex2i(x,y); glVertex2i(x+w/2,y-h); glVertex2i(x-w/2,y-h);
	} else {
		glVertex2i(x-w/2,y); glVertex2i(x+w/2,y);
		glVertex2i(x+w/2,y-h); glVertex2i(x-w/2,y-h);
		if (edit_on && type==ET_OPCION && mouse!=this) {
			glVertex2i(x-w/2,y); glVertex2i(x-w/2+flecha_w,y); glVertex2i(x-w/2+flecha_w,y-h); glVertex2i(x-w/2,y-h);
		}
	}
	glEnd();
}

inline void DrawTrue(int x, int y) {
	glVertex2d(x,y); glVertex2d(x-margin/2,y+2*vf_size);
	glVertex2d(x,y); glVertex2d(x+margin/2,y+2*vf_size);
}

inline void DrawFalse(int x, int y) {
	glVertex2d(x-vf_size/2,y); glVertex2d(x-vf_size/2,y+2*vf_size);
	glVertex2d(x-vf_size/2,y+2*vf_size); glVertex2d(x+vf_size/2,y+2*vf_size);
	glVertex2d(x-vf_size/2,y+vf_size); glVertex2d(x+vf_size/2,y+vf_size);
}

inline void DrawFlechaDown(int x, int y1, int y2) {
	glVertex2d(x,y1); glVertex2d(x,y2);
	glVertex2d(x-flecha_d,y2+flecha_d); glVertex2d(x,y2);
	glVertex2d(x+flecha_d,y2+flecha_d); glVertex2d(x,y2);
}

inline void DrawFlechaDownHead(int x, int y2) {
	glVertex2d(x-flecha_d,y2+flecha_d); glVertex2d(x,y2);
	glVertex2d(x+flecha_d,y2+flecha_d); glVertex2d(x,y2);
}

inline void DrawFlechaUp(int x, int y1, int y2) {
	glVertex2d(x,y1); glVertex2d(x,y2);
	glVertex2d(x-flecha_d,y2-flecha_d); glVertex2d(x,y2);
	glVertex2d(x+flecha_d,y2-flecha_d); glVertex2d(x,y2);
}

inline void DrawFlechaR(int x1, int x2, int y) {
	glVertex2d(x1,y); glVertex2d(x2,y);
	glVertex2d(x2-flecha_d,y-flecha_d); glVertex2d(x2,y);
	glVertex2d(x2-flecha_d,y+flecha_d); glVertex2d(x2,y);
}

inline void DrawFlechaL(int x1, int x2, int y) {
	glVertex2d(x1,y); glVertex2d(x2,y);
	glVertex2d(x2+flecha_d,y-flecha_d); glVertex2d(x2,y);
	glVertex2d(x2+flecha_d,y+flecha_d); glVertex2d(x2,y);
}

void Entity::DrawClasico(bool force) {
	if (!force && (type==ET_OPCION || type==ET_AUX_PARA)) return;
	if (this==mouse && (prev||parent)) // si se esta moviendo con el mouse, dibujar un ghost donde lo agregariamos al soltar
		DrawShapeBorder(color_ghost,d_dx+x,d_dy+y,bwr+bwl,h);
	else /*if (type!=ET_OPCION && type!=ET_SEGUN)*/ // sombra
		DrawShapeSolid(color_shadow,d_fx+shadow_delta_x,d_fy-shadow_delta_y,d_w,d_h);
	// flechas
	glBegin(GL_LINES); 
	glColor3fv(color_arrow);
	if (!nolink) {
		if (type==ET_OPCION) {
			glVertex2i((child[0]?child[0]->d_x:d_x),d_y-d_h); glVertex2i((child[0]?child[0]->d_x:d_x),d_y-d_h-flecha_h);
		} else if (type==ET_SEGUN) {
			for(int i=0;i<n_child;i++) {
				if (!child[i]->child[0]) {
					DrawFlechaDownHead(d_x+child_dx[i],d_y-d_h-child[i]->bh);
					DrawFlechaDown(d_x+child_dx[i],d_y-d_h-child_bh[i],d_y-d_bh+flecha_h); 
				}
				else if (child[i]!=mouse) {
					DrawFlechaDown(child[i]->child[0]->d_x,d_y-d_h-child_bh[i],d_y-d_bh+flecha_h); 
					DrawFlechaDownHead(child[i]->child[0]->d_x,child[i]->child[0]->d_y-child[i]->child[0]->d_bh); 
				}
			}
			// linea horizontal de abajo
			glVertex2d(d_x+child_dx[0]+(child[0]?child[0]->child_dx[0]:0),d_y-d_bh+flecha_h); glVertex2d(d_x,d_y-d_bh+flecha_h);
			glVertex2d(d_x,d_y-d_bh+flecha_h); glVertex2d(d_x+child_dx[n_child-1]+(child[n_child-1]?child[n_child-1]->child_dx[0]:0),d_y-d_bh+flecha_h);
		} else if (type==ET_MIENTRAS) {
			DrawTrue(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
			DrawFalse(d_fx+d_w/2+2*vf_size,d_fy-d_h/2+vf_size);
			glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra
			glVertex2d(d_x,d_y-d_bh+3*flecha_h); glVertex2d(d_x,d_y-d_bh+2*flecha_h); // sale de la ultima instruccion
			DrawFlechaL(d_x,d_x-d_bwl,d_y-d_bh+2*flecha_h); // sigue a la izquierda
			DrawFlechaUp(d_x-d_bwl,d_y-d_bh+2*flecha_h,d_y); // sube
			DrawFlechaR(d_x-d_bwl,d_x,d_y); // entra arriba de la condicion
			glVertex2d(d_x,d_y-flecha_h-d_h/2); glVertex2d(d_x+d_bwr,d_y-flecha_h-d_h/2); // sale de la condicion para la derecha
			DrawFlechaDown(d_x+d_bwr,d_y-flecha_h-d_h/2,d_y-d_bh+flecha_h); // baja
			glVertex2d(d_x+d_bwr,d_y-d_bh+flecha_h); glVertex2d(d_x,d_y-d_bh+flecha_h); // va al punto de salida
			glVertex2i(d_fx,d_fy-d_h); glVertex2i(d_fx,d_fy-d_h-flecha_h);
		} else if (type==ET_PARA) {
			glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra del bloque
			glVertex2i(d_x,d_y-child_bh[0]-flecha_h); glVertex2i(d_x,d_y-d_bh+flecha_h); // flecha que sale del bloque
			glVertex2i(d_fx,d_y); glVertex2i(d_fx,d_fy); // flecha que sale del circulo
			DrawFlechaR(d_fx,d_x,d_y); 
			glVertex2i(d_fx,d_y); glVertex2i(d_fx,d_fy); // flecha que sale del circulo
			DrawFlechaUp(d_fx,d_y-d_bh+flecha_h,d_fy-d_h); // flecha que entra al circulo
			glVertex2i(d_fx,d_y-d_bh+flecha_h); glVertex2i(d_x,d_y-d_bh+flecha_h);
		} else if (type==ET_REPETIR) {
			DrawFlechaDownHead(d_fx,d_fy);
			if (variante) {
				DrawFalse(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
				DrawTrue(d_fx-d_w/2-2*vf_size,d_fy-d_h/2+vf_size);
			} else {
				DrawTrue(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
				DrawFalse(d_fx-d_w/2-2*vf_size,d_fy-d_h/2+vf_size);
			}
			glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra
			DrawFlechaL(d_x,d_x-d_bwl,d_fy-d_h/2); // sigue a la izquierda
			DrawFlechaUp(d_x-d_bwl,d_fy-d_h/2,d_y); // sube
			DrawFlechaR(d_x-d_bwl,d_x,d_y); // entra arriba de la condicion
			glVertex2i(d_fx,d_fy+flecha_h); glVertex2i(d_fx,d_fy); // flecha a la siguiente instruccion
		} else if (type==ET_SI) {
			DrawTrue(d_fx+d_w/2+2*vf_size,d_fy-d_h/2+vf_size);
			DrawFalse(d_fx-d_w/2-2*vf_size,d_fy-d_h/2+vf_size);
			// linea horizontal de arriba
			glVertex2d(d_x+child_dx[0],d_y-d_h/2); glVertex2d(d_x+child_dx[1],d_y-d_h/2);
			// flechas que bajan por el verdadero
			if (child[0]) { glVertex2i(d_x+child_dx[0],d_y-d_h/2); glVertex2i(d_x+child_dx[0],d_y-d_h-flecha_h); }
			else DrawFlechaDown(d_x+child_dx[0],d_y-d_h/2,d_y-d_h-flecha_h); 
			DrawFlechaDown(d_x+child_dx[0],d_y-d_h-child_bh[0]-flecha_h,d_y-d_bh+flecha_h); 
			// flechas que bajan por el falso
			if (child[1]) { glVertex2i(d_x+child_dx[1],d_y-d_h/2); glVertex2i(d_x+child_dx[1],d_y-d_h-flecha_h); }
			else DrawFlechaDown(d_x+child_dx[1],d_y-d_h/2,d_y-d_h-flecha_h); 
			DrawFlechaDown(d_x+child_dx[1],d_y-d_h-child_bh[1]-flecha_h,d_y-d_bh+flecha_h); 
			// linea horizontal de abajo
			glVertex2d(d_x+child_dx[0],d_y-d_bh+flecha_h); glVertex2d(d_x+child_dx[1],d_y-d_bh+flecha_h);
		}
		// punta de flecha que viene del anterior
		if (type!=ET_OPCION && (prev||parent)) DrawFlechaDownHead(d_x,d_y-flecha_in); // no en inicio
		// linea de flecha que va al siguiente
		if ((next||parent)&&(type!=ET_OPCION)) { glVertex2i(d_x,d_y-d_bh); glVertex2i(d_x,d_y-d_bh+flecha_h); } // no en fin
	} else if (mouse==this && (next||parent)) {
		// flecha que va al siguiente item cuando este esta flotando
		glVertex2i(d_dx+x,d_dy+y-bh); glVertex2i(d_dx+x,d_dy+y-bh+flecha_h);
		if (type!=ET_OPCION) DrawFlechaDownHead(d_dx+x,d_dy+y); // no en inicio
	}
	glEnd();
	// relleno de la forma
	DrawShapeSolid(color_shape,d_fx,d_fy,d_w,d_h);
	// borde de la forma
	DrawShapeBorder(mouse==this?color_selection:color_border,d_fx,d_fy,d_w,d_h);
	
	if (type==ET_OPCION) { // + para agregar opciones
		if (edit_on && mouse!=this) {
			glBegin(GL_LINES);
			glColor3fv(color_label);
			glVertex2i(d_x-d_bwl+3*flecha_w/4,d_y-d_h/2); glVertex2i(d_x-d_bwl+1*flecha_w/4,d_y-d_h/2);
			glVertex2i(d_x-d_bwl+flecha_w/2,d_y-1*d_h/3); glVertex2i(d_x-d_bwl+flecha_w/2,d_y-2*d_h/3);
			glEnd();
		}
	} else 
		if (!nolink && (type==ET_ESCRIBIR||type==ET_LEER) ) { // flecha en la esquina
			glBegin(GL_LINES);
			glColor3fv(color_label);
			glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2+margin,d_y+margin);
			if (type==ET_LEER) {
				glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2-margin+margin,d_y-margin);
				glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2-margin,d_y-margin+margin);
			} else {
				glVertex2d(d_x+d_w/2+margin,d_y+margin); glVertex2d(d_x+d_w/2+margin-margin,d_y+margin);
				glVertex2d(d_x+d_w/2+margin,d_y+margin); glVertex2d(d_x+d_w/2+margin,d_y+margin-margin);
			}
			
			glEnd();
		}
	// texto;
	DrawText();
	if (!nolink) {
		if (type==ET_SEGUN) {
			for(int i=0;i<n_child;i++) { 
				child[i]->Draw(true);
			}
		} else  if (type==ET_PARA) {
			glColor3fv(color_border);
			glBegin(GL_LINES);
			glVertex2i(d_fx-w/2,d_fy-d_h/2); glVertex2i(d_fx+w/2,d_fy-d_h/2); // separadores de las cuatro partes del circulo
			if (!variante) {
				glVertex2i(d_x+child_dx[1],d_fy-d_h/2); glVertex2i(d_x+child_dx[1],d_fy-d_h+margin);
				glVertex2i(d_x+child_dx[2],d_fy-d_h/2); glVertex2i(d_x+child_dx[2],d_fy-d_h+margin);
				glEnd();
				child[1]->DrawText();
				child[2]->DrawText();
				child[3]->DrawText();
			} else {
				glEnd();
				child[2]->DrawText();
			}
		}
	}
}

void Entity::CalculateClasico() { // calcula lo propio y manda a calcular al siguiente y a sus hijos, y acumula en gw,gh el tamaño de este item (para armar el tamaño del bloque)
	
	// calcular tamaños de la forma segun el texto
	if (!t_w) w=margin*6; else { w=t_w; if (type!=ET_PROCESO) w+=2*margin; } h=t_h+2*margin; 
	if (type==ET_REPETIR||type==ET_MIENTRAS||type==ET_SI) {
		w*=2; h*=2;
	} else if (type==ET_ESCRIBIR||type==ET_LEER) {
		w+=2*margin;
	} else if (type==ET_PARA) {
		h=2*h+3*margin; w=1.3*w+2*margin;
	} else if (type==ET_SEGUN) {
		h*=2;
	} else if (type==ET_PROCESO) {
		w+=2*h;
	}
	
	t_dy=t_dx=0; fx=x; fy=y; bh=h+flecha_h; bwr=bwl=w/2; // esto es si fuera solo la forma
	
	// si son estructuras de control, es un viaje
	if (!nolink && n_child) {
		if (type==ET_OPCION) {
			bwr=bwl=(w=t_w+2*margin)/2;
			if (edit_on) 
			{ bwr+=flecha_w; bwl+=flecha_w; } // el + para agregar opciones
			child_dx[0]=0; child_bh[0]=0;
			if (child[0]) {
				child[0]->x=x; child[0]->y=y-bh;
				int cwl=0,cwr=0,ch=0;
				child[0]->Calculate(cwl,cwr,ch);
				bh+=ch; 
				if (cwl+cwr>bwl+bwr) bwl=bwr=(cwl+cwr)/2;
				bwl+=flecha_w/2;
				bwr+=flecha_w/2;
				child_dx[0]-=(cwr-cwl)/2;
				child[0]->MoveX(child_dx[0]);
			}
			// el ancho se lo define el segun padre
			w=bwl+bwr;
		} else if (type==ET_SEGUN) {
			t_dy=-t_h/2-margin;
			bwr=bwl=(w=t_w*2)/2;
//				w=bwr=bwl=0; // todo: ver como corregir esto
			int sw=0, sh=0;
			for (int i=0;i<n_child;i++) {
				int cwl=0,cwr=0,ch=0;
				child[i]->x=0; child[i]->y=y-h;
				child[i]->Calculate(cwl,cwr,ch);
				child_bh[i]=ch; child_dx[i]=sw+cwl;
				sw+=cwl+cwr-2;
				if (ch>sh) sh=ch;
			}
			if (sw>w) w=sw; 
			bwr=bwl=w/2; 
			bh+=sh;
			for (int i=0;i<n_child;i++) {
				child_dx[i]-=w/2;
				child[i]->MoveX(child_dx[i]);
			}
			Entity *dom=child[n_child-1];
			if (dom->x+dom->bwr<x+bwr) {
				int dif=(x+bwr)-(dom->x+dom->bwr);
				dom->bwl+= dif/2;
				dom->bwr+= dif/2;
				dom->w+= dif;
				dom->MoveX(dif/2);
				child_dx[n_child-1]+=dif/2;
			}
		} if (type==ET_SI) {
			int c1l=0,c1r=0,c1h=0;
			if (child[0]) {
				child[0]->y=y-bh; child[0]->x=x;
				child[0]->Calculate(c1l,c1r,c1h);
			} 
			int c2l=0,c2r=0,c2h=0;
			if (child[1]) {
				child[1]->y=y-bh; child[1]->x=x;
				child[1]->Calculate(c2l,c2r,c2h);
			} 
			bh += (c1h>c2h?c1h:c2h) + flecha_h;
			child_bh[0]=c1h; child_bh[1]=c2h;
			child_dx[0] = -flecha_w-(c1r>bwl?c1r:bwl);
			child_dx[1] =  flecha_w+(c2l>bwr?c2l:bwr);
			if (child[0]) child[0]->MoveX(child_dx[0]);
			if (child[1]) child[1]->MoveX(child_dx[1]);
			if (c1l-child_dx[0]>bwl) bwl=c1l-child_dx[0];
			if (c2r+child_dx[1]>bwr) bwr=c2r+child_dx[1];
		} else if (type==ET_MIENTRAS||type==ET_REPETIR) {
			int c1l=0,c1r=0,c1h=0;
			if (child[0]) {
				child[0]->y=y-(type==ET_MIENTRAS?bh+flecha_h:flecha_h); child[0]->x=x;
				child[0]->Calculate(c1l,c1r,c1h);
			} 
			child_dx[0]=0; child_bh[0]=c1h;
			if (c1l>bwl) bwl=c1l;
			if (c1r>bwr) bwr=c1r;
			bwr+=flecha_w; bwl+=flecha_w; bh+=c1h;
			fy-= (type==ET_MIENTRAS)?flecha_h:c1h+flecha_h;
			bh+= (type==ET_MIENTRAS)?flecha_h*3:flecha_h;
		} else if (type==ET_PARA) {
			t_dy=(t_h+margin)/2;
			
			// averiguar cuanto miden las tres etiquetas de abajo en el circulo
			child[1]->x=child[2]->x=child[3]->x=x;
			int vl=0,vr=0,vh=0;
			child[1]->Calculate(vl,vr,vh); 
			int v1=vl+vr; vl=vr=0;
			child[2]->Calculate(vl,vr,vh); 
			int v2=vl+vr; vl=vr=0;
			child[3]->Calculate(vl,vr,vh); 
			int v3=vl+vr;
			if (variante) { v1=v3=0; child[1]->w=child[3]->w=0; }
			
			// calcular el acnho del circulo, puede estar dominado por las tres etiquetas de abajo o por la propia 
			int v=v1+v2+v3-2*margin;
			w=(v>t_w?v:t_w)*1.3+2*margin;
			v+=2*margin;
			
			// acomodar el circulo
			bwr=0; bwl=w;
			int c1l=0,c1r=0,c1h=0;
			if (child[0]) {
				child[0]->y=y-flecha_h; child[0]->x=x;
				child[0]->Calculate(c1l,c1r,c1h);
			} 
			child_dx[0]=0; child_bh[0]=c1h;
			if (c1r>bwr) bwr=c1r;
			bwl=bwl+c1l+flecha_w;
			if (c1h>bh) bh=c1h;
			bh+=2*flecha_h;
			fx=-c1l-w/2-flecha_w;
			fy=y+(flecha_h-bh+h)/2;
			
			// acomodar las tres etiquetas
			int cy=fy-h/2-margin/2;
			child[1]->fy=cy; child[1]->MoveX(fx-x+(-v+v1)/2); child_dx[1]=child[1]->fx+v1/2;
			child[2]->fy=cy; child[2]->MoveX(fx-x+(-v+v2)/2+v1); child_dx[2]=child[2]->fx+v2/2;
			child[3]->fy=cy; child[3]->MoveX(fx-x+(v-v3)/2); child_dx[3]=child[3]->fx+v3/2;
		}
	}
}
