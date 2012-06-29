#include "Entity.h"
#include "Global.h"
#include "Draw.h"
#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
using namespace std;

static void DrawRectangle(const float *color, int x, int y, int wl, int wr, int h) {
	glColor3fv(color);
	glBegin(GL_LINE_LOOP);
	glVertex2i(x-wl,y);
	glVertex2i(x-wl,y-h);
	glVertex2i(x+wr,y-h);
	glVertex2i(x+wr,y);
	glEnd();
}

static void DrawSolidRectangle(const float *color, int x, int y, int wl, int wr, int h) {
	glColor3fv(color);
	glBegin(GL_QUADS);
	glVertex2i(x-wl,y);
	glVertex2i(x-wl,y-h);
	glVertex2i(x+wr,y-h);
	glVertex2i(x+wr,y);
	glEnd();
}

static void DrawTextNS(const float *color, int x, int y, string label) {
	glColor3fv(color);
	glPushMatrix();
	glTranslated(x,y,0);
	glScaled(.105,.15,.1);
	for (unsigned int i=0;i<label.size();i++)
		dibujar_caracter(label[i]);
	glPopMatrix();
}

void Entity::DrawNassiSchne(bool force) {
//	if (!force && (type==ET_OPCION || type==ET_AUX_PARA)) return;
//	DrawSolidRectangle(color_shape,d_fx,d_fy,d_bwl,d_bwr,d_bh>2*h+margin?2*h+margin:d_bh);
	DrawRectangle(color_border,d_fx,d_fy,d_bwl,d_bwr,d_bh);
//	// flechas
//	glBegin(GL_LINES); 
//	glColor3fv(color_arrow);
	if (!nolink) {
//		if (type==ET_OPCION) {
//			glVertex2i((child[0]?child[0]->d_x:d_x),d_y-d_h); glVertex2i((child[0]?child[0]->d_x:d_x),d_y-d_h-flecha_h);
//		} else if (type==ET_SEGUN) {
//			for(int i=0;i<n_child;i++) {
//				if (!child[i]->child[0]) {
//					DrawFlechaDownHead(d_x+child_dx[i],d_y-d_h-child[i]->bh);
//					DrawFlechaDown(d_x+child_dx[i],d_y-d_h-child_bh[i],d_y-d_bh+flecha_h); 
//				}
//				else if (child[i]!=mouse) {
//					DrawFlechaDown(child[i]->child[0]->d_x,d_y-d_h-child_bh[i],d_y-d_bh+flecha_h); 
//					DrawFlechaDownHead(child[i]->child[0]->d_x,child[i]->child[0]->d_y-child[i]->child[0]->d_bh); 
//				}
//			}
//			// linea horizontal de abajo
//			glVertex2d(d_x+child_dx[0]+(child[0]?child[0]->child_dx[0]:0),d_y-d_bh+flecha_h); glVertex2d(d_x,d_y-d_bh+flecha_h);
//			glVertex2d(d_x,d_y-d_bh+flecha_h); glVertex2d(d_x+child_dx[n_child-1]+(child[n_child-1]?child[n_child-1]->child_dx[0]:0),d_y-d_bh+flecha_h);
//		} else if (type==ET_MIENTRAS) {
//			DrawTrue(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
//			DrawFalse(d_fx+d_w/2+2*vf_size,d_fy-d_h/2+vf_size);
//			glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra
//			glVertex2d(d_x,d_y-d_bh+3*flecha_h); glVertex2d(d_x,d_y-d_bh+2*flecha_h); // sale de la ultima instruccion
//			DrawFlechaL(d_x,d_x-d_bwl,d_y-d_bh+2*flecha_h); // sigue a la izquierda
//			DrawFlechaUp(d_x-d_bwl,d_y-d_bh+2*flecha_h,d_y); // sube
//			DrawFlechaR(d_x-d_bwl,d_x,d_y); // entra arriba de la condicion
//			glVertex2d(d_x,d_y-flecha_h-d_h/2); glVertex2d(d_x+d_bwr,d_y-flecha_h-d_h/2); // sale de la condicion para la derecha
//			DrawFlechaDown(d_x+d_bwr,d_y-flecha_h-d_h/2,d_y-d_bh+flecha_h); // baja
//			glVertex2d(d_x+d_bwr,d_y-d_bh+flecha_h); glVertex2d(d_x,d_y-d_bh+flecha_h); // va al punto de salida
//			glVertex2i(d_fx,d_fy-d_h); glVertex2i(d_fx,d_fy-d_h-flecha_h);
//		} else if (type==ET_PARA) {
//			glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra del bloque
//			glVertex2i(d_x,d_y-child_bh[0]-flecha_h); glVertex2i(d_x,d_y-d_bh+flecha_h); // flecha que sale del bloque
//			glVertex2i(d_fx,d_y); glVertex2i(d_fx,d_fy); // flecha que sale del circulo
//			DrawFlechaR(d_fx,d_x,d_y); 
//			glVertex2i(d_fx,d_y); glVertex2i(d_fx,d_fy); // flecha que sale del circulo
//			DrawFlechaUp(d_fx,d_y-d_bh+flecha_h,d_fy-d_h); // flecha que entra al circulo
//			glVertex2i(d_fx,d_y-d_bh+flecha_h); glVertex2i(d_x,d_y-d_bh+flecha_h);
//		} else if (type==ET_REPETIR) {
//			DrawFlechaDownHead(d_fx,d_fy);
//			if (variante) {
//				DrawFalse(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
//				DrawTrue(d_fx-d_w/2-2*vf_size,d_fy-d_h/2+vf_size);
//			} else {
//				DrawTrue(d_fx+2*vf_size,d_fy-d_h-5*vf_size/2);
//				DrawFalse(d_fx-d_w/2-2*vf_size,d_fy-d_h/2+vf_size);
//			}
//			glVertex2i(d_x,d_y); glVertex2i(d_x,d_y-flecha_in); // flecha que entra
//			DrawFlechaL(d_x,d_x-d_bwl,d_fy-d_h/2); // sigue a la izquierda
//			DrawFlechaUp(d_x-d_bwl,d_fy-d_h/2,d_y); // sube
//			DrawFlechaR(d_x-d_bwl,d_x,d_y); // entra arriba de la condicion
//			glVertex2i(d_fx,d_fy+flecha_h); glVertex2i(d_fx,d_fy); // flecha a la siguiente instruccion
//		} else 
		if (type==ET_SI) {
			DrawTextNS(color_arrow,d_x-d_bwl+20,d_y-2*h,"Si");
			DrawTextNS(color_arrow,d_x+d_bwr-60,d_y-2*h,"No");
			glColor3fv(color_border);
			glBegin(GL_LINE_STRIP);
			glVertex2i(d_x-d_bwl,d_y);
			int px=d_x;
			if (child[0]) px+=child_dx[0]+child[0]->d_bwr;
			else if (child[1]) px+=child_dx[1]-child[1]->d_bwl;
			glVertex2i(px,d_y-2*h-margin);
			glVertex2i(d_x+d_bwr,d_y);
			glEnd();
		}
//		// punta de flecha que viene del anterior
//		if (type!=ET_OPCION && (prev||parent)) DrawFlechaDownHead(d_x,d_y-flecha_in); // no en inicio
//		// linea de flecha que va al siguiente
//		if ((next||parent)&&(type!=ET_OPCION)) { glVertex2i(d_x,d_y-d_bh); glVertex2i(d_x,d_y-d_bh+flecha_h); } // no en fin
//	} else if (mouse==this && (next||parent)) {
//		// flecha que va al siguiente item cuando este esta flotando
//		glVertex2i(d_dx+x,d_dy+y-bh); glVertex2i(d_dx+x,d_dy+y-bh+flecha_h);
//		if (type!=ET_OPCION) DrawFlechaDownHead(d_dx+x,d_dy+y); // no en inicio
	}
//	glEnd();
//	// relleno de la forma
//	DrawShapeSolid(color_shape,d_fx,d_fy,d_w,d_h);
//	// borde de la forma
//	DrawShapeBorder(mouse==this?color_selection:color_border,d_fx,d_fy,d_w,d_h);
//	
//	if (type==ET_OPCION) { // + para agregar opciones
//		if (edit_on && mouse!=this) {
//			glBegin(GL_LINES);
//			glColor3fv(color_label);
//			glVertex2i(d_x-d_bwl+3*flecha_w/4,d_y-d_h/2); glVertex2i(d_x-d_bwl+1*flecha_w/4,d_y-d_h/2);
//			glVertex2i(d_x-d_bwl+flecha_w/2,d_y-1*d_h/3); glVertex2i(d_x-d_bwl+flecha_w/2,d_y-2*d_h/3);
//			glEnd();
//		}
//	} else 
//		if (!nolink && (type==ET_ESCRIBIR||type==ET_LEER) ) { // flecha en la esquina
//			glBegin(GL_LINES);
//			glColor3fv(color_label);
//			glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2+margin,d_y+margin);
//			if (type==ET_LEER) {
//				glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2-margin+margin,d_y-margin);
//				glVertex2d(d_x+d_w/2-margin,d_y-margin); glVertex2d(d_x+d_w/2-margin,d_y-margin+margin);
//			} else {
//				glVertex2d(d_x+d_w/2+margin,d_y+margin); glVertex2d(d_x+d_w/2+margin-margin,d_y+margin);
//				glVertex2d(d_x+d_w/2+margin,d_y+margin); glVertex2d(d_x+d_w/2+margin,d_y+margin-margin);
//			}
//			
//			glEnd();
//		}
//	// texto;
	DrawText();
//	if (!nolink) {
//		if (type==ET_SEGUN) {
//			for(int i=0;i<n_child;i++) { 
//				child[i]->Draw(true);
//			}
//		} else  if (type==ET_PARA) {
//			glColor3fv(color_border);
//			glBegin(GL_LINES);
//			glVertex2i(d_fx-w/2,d_fy-d_h/2); glVertex2i(d_fx+w/2,d_fy-d_h/2); // separadores de las cuatro partes del circulo
//			if (!variante) {
//				glVertex2i(d_x+child_dx[1],d_fy-d_h/2); glVertex2i(d_x+child_dx[1],d_fy-d_h+margin);
//				glVertex2i(d_x+child_dx[2],d_fy-d_h/2); glVertex2i(d_x+child_dx[2],d_fy-d_h+margin);
//				glEnd();
//				child[1]->DrawText();
//				child[2]->DrawText();
//				child[3]->DrawText();
//			} else {
//				glEnd();
////			child[1]->DrawText();
//				child[2]->DrawText();
////			child[3]->DrawText();
//			}
//		}
//	}
}

void Entity::CalculateNassiSchne() { // calcula lo propio y manda a calcular al siguiente y a sus hijos, y acumula en gw,gh el tamaño de este item (para armar el tamaño del bloque)
	fx=x; fy=y; bh=h; bwr=bwl=w/2; // esto es si fuera solo la forma
	// si son estructuras de control...
	if (!nolink && n_child) {
//		if (type==ET_OPCION) {
//			bwr=bwl=(w=t_w+2*margin)/2;
//			if (edit_on) 
//			{ bwr+=flecha_w; bwl+=flecha_w; } // el + para agregar opciones
//			child_dx[0]=0; child_bh[0]=0;
//			if (child[0]) {
//				child[0]->x=x; child[0]->y=y-bh;
//				int cwl=0,cwr=0,ch=0;
//				child[0]->Calculate(cwl,cwr,ch);
//				bh+=ch; 
//				if (cwl+cwr>bwl+bwr) bwl=bwr=(cwl+cwr)/2;
//				bwl+=flecha_w/2;
//				bwr+=flecha_w/2;
//				child_dx[0]-=(cwr-cwl)/2;
//				child[0]->MoveX(child_dx[0]);
//			}
//			// el ancho se lo define el segun padre
//			w=bwl+bwr;
//		} else if (type==ET_SEGUN) {
//			bwr=bwl=(w=t_w*2)/2;
////				w=bwr=bwl=0; // todo: ver como corregir esto
//			int sw=0, sh=0;
//			for (int i=0;i<n_child;i++) {
//				int cwl=0,cwr=0,ch=0;
//				child[i]->x=0; child[i]->y=y-h;
//				child[i]->Calculate(cwl,cwr,ch);
//				child_bh[i]=ch; child_dx[i]=sw+cwl;
//				sw+=cwl+cwr-2;
//				if (ch>sh) sh=ch;
//			}
//			if (sw>w) w=sw; 
//			bwr=bwl=w/2; 
//			bh+=sh;
//			for (int i=0;i<n_child;i++) {
//				child_dx[i]-=w/2;
//				child[i]->MoveX(child_dx[i]);
//			}
//			Entity *dom=child[n_child-1];
//			if (dom->x+dom->bwr<x+bwr) {
//				int dif=(x+bwr)-(dom->x+dom->bwr);
//				dom->bwl+= dif/2;
//				dom->bwr+= dif/2;
//				dom->w+= dif;
//				dom->MoveX(dif/2);
//				child_dx[n_child-1]+=dif/2;
//			}
//		} 
		if (type==ET_SI) {
			bh*=2;
			bh+=margin;
			// calcular tamaño hijo por verdadero
			int c1l=0,c1r=0,c1h=0;
			if (child[0]) {
				child[0]->y=y-bh; child[0]->x=x;
				child[0]->Calculate(c1l,c1r,c1h);
			} else {
				c1r=c1l=c1h=20;
			}
			// calcular tamaño hijo por falso
			int c2l=0,c2r=0,c2h=0;
			if (child[1]) {
				child[1]->y=y-bh; child[1]->x=x;
				child[1]->Calculate(c2l,c2r,c2h);
			} else {
				c2r=c2l=c2h=20;
			}
			// ajustar tamaño propio
			bwl=bwr=(c1r+c1l+c2r+c2l)/2;
			bh += (c1h>c2h?c1h:c2h);
			child_bh[0]=c1h; child_bh[1]=c2h;
			// mover hijos horizontalmente
			child_dx[0] = -bwl+c1l;
			child_dx[1] = bwr-c2r;
			if (child[0]) child[0]->MoveX(child_dx[0]);
			if (child[1]) child[1]->MoveX(child_dx[1]);
		} 
		if (type==ET_MIENTRAS||type==ET_PARA||type==ET_REPETIR) {
			bh+=margin;
			// calcular tamaño hijo por verdadero
			int c1l=0,c1r=0,c1h=0;
			if (child[0]) {
				child[0]->y=y-(type==ET_REPETIR?0:bh); child[0]->x=x;
				child[0]->Calculate(c1l,c1r,c1h);
			} else {
				c1r=c1l=c1h=20;
			}
			// ajustar tamaño propio
			bwl=bwr=(c1r+c1l)/2;
			bh+=c1h;
			child_bh[0]=0;
		} 
//			else if (type==ET_MIENTRAS||type==ET_REPETIR) {
//			int c1l=0,c1r=0,c1h=0;
//			if (child[0]) {
//				child[0]->y=y-(type==ET_MIENTRAS?bh+flecha_h:flecha_h); child[0]->x=x;
//				child[0]->Calculate(c1l,c1r,c1h);
//			} 
//			child_dx[0]=0; child_bh[0]=c1h;
//			if (c1l>bwl) bwl=c1l;
//			if (c1r>bwr) bwr=c1r;
//			bwr+=flecha_w; bwl+=flecha_w; bh+=c1h;
//			fy-= (type==ET_MIENTRAS)?flecha_h:c1h+flecha_h;
//			bh+= (type==ET_MIENTRAS)?flecha_h*3:flecha_h;
//		} 
//			else if (type==ET_PARA) {
//			
//			// averiguar cuanto miden las tres etiquetas de abajo en el circulo
//			child[1]->x=child[2]->x=child[3]->x=x;
//			int vl=0,vr=0,vh=0;
//			child[1]->Calculate(vl,vr,vh); 
//			int v1=vl+vr; vl=vr=0;
//			child[2]->Calculate(vl,vr,vh); 
//			int v2=vl+vr; vl=vr=0;
//			child[3]->Calculate(vl,vr,vh); 
//			int v3=vl+vr;
//			if (variante) { v1=v3=0; child[1]->w=child[3]->w=0; }
//			
//			// calcular el acnho del circulo, puede estar dominado por las tres etiquetas de abajo o por la propia 
//			int v=v1+v2+v3-2*margin;
//			w=(v>t_w?v:t_w)*1.3+2*margin;
//			v+=2*margin;
//			
//			// acomodar el circulo
//			bwr=0; bwl=w;
//			int c1l=0,c1r=0,c1h=0;
//			if (child[0]) {
//				child[0]->y=y-flecha_h; child[0]->x=x;
//				child[0]->Calculate(c1l,c1r,c1h);
//			} 
//			child_dx[0]=0; child_bh[0]=c1h;
//			if (c1r>bwr) bwr=c1r;
//			bwl=bwl+c1l+flecha_w;
//			if (c1h>bh) bh=c1h;
//			bh+=2*flecha_h;
//			fx=-c1l-w/2-flecha_w;
//			fy=y+(flecha_h-bh+h)/2;
//			
//			// acomodar las tres etiquetas
//			int cy=fy-h/2-margin/2;
//			child[1]->fy=cy; child[1]->MoveX(fx-x+(-v+v1)/2); child_dx[1]=child[1]->fx+v1/2;
//			child[2]->fy=cy; child[2]->MoveX(fx-x+(-v+v2)/2+v1); child_dx[2]=child[2]->fx+v2/2;
//			child[3]->fy=cy; child[3]->MoveX(fx-x+(v-v3)/2); child_dx[3]=child[3]->fx+v3/2;
//		}
	}
	if (!next) {
		int max=bwl+bwr;
		Entity *et=prev;
		while (et) {
			if (et->bwl+et->bwr>max)
				max=et->bwl+et->bwr;
			et=et->prev;
		}
		et=this;
		while (et) {
			int old=et->bwl+et->bwr;
			et->bwl+=(max-old)/2;
			et->bwr+=(max-old)/2;
			et=et->prev;
		}
	}
}
