#include "GLtoWX.h"
#include <stack>
#include <wx/dc.h>
#include "../wxPSeInt/string_conversions.h"

struct matrix {
	double sx,sy; // scale
	double tx,ty; // translation
	matrix():sx(1),sy(1),tx(0),ty(0){}
	void scale(double x, double y) { 
		sx*=x;
		sy*=y;
	}
	void translate(double x, double y) {
		tx+=x*sx;
		ty+=y*sy;
	}
	int x(double _x) { return int(_x*sx+tx); }
	int x(int _x) { return int(_x*sx+tx); }
	int y(double _y) { return int(_y*sy+ty); }
	int y(int _y) { return int(_y*sy+ty); }
};

static matrix m; // la matriz de transformacion actual
static std::stack<matrix> ms; // el stack de matrices
static int cb=GL_NONE; // current glbegin value
static int lw=0; // line width
static int p[100][2], np=0; // vertices enviados por glvertex
static wxColour c(0,0,0); // color actual
static wxPen pt(wxPenInfo(c,0).Style(wxPENSTYLE_TRANSPARENT));
wxDC *dc=NULL;

void glColor3fv(const float *v) {
	c.Set(int(255*v[0]),int(255*v[1]),int(255*v[2]));
}

void glBegin(int what) {
	cb=what; np=0;
}

void glEnd() {
	if (cb==GL_LINE_LOOP && np==2) {
		dc->SetPen(wxPen(c,lw));
		dc->DrawLine(p[1][0],p[1][1],p[0][0],p[0][1]);
	}
	cb=GL_NONE; np=0;
}

static void post_vertex() {
	switch (cb) {
	case GL_LINES: if (np==2) {
		dc->SetPen(wxPen(c,lw));
		dc->DrawLine(p[0][0],p[0][1],p[1][0],p[1][1]);
		np=0;
	} break;
	case GL_LINE_STRIP: if (np==2) {
		dc->SetPen(wxPen(c,lw));
		dc->DrawLine(p[0][0],p[0][1],p[1][0],p[1][1]);
		np=1; p[0][0]=p[1][0]; p[0][1]=p[1][1];
	} break;
	case GL_LINE_LOOP: if (np==2) {
		dc->SetPen(wxPen(c,lw));
		dc->DrawLine(p[0][0],p[0][1],p[1][0],p[1][1]);
	} else if (np==3) {
		dc->SetPen(wxPen(c,lw));
		dc->DrawLine(p[1][0],p[1][1],p[2][0],p[2][1]);
		np=2; p[1][0]=p[2][0]; p[1][1]=p[2][1];
	} break;
	case GL_QUADS: if (np==4) {
		dc->SetPen(pt);
		dc->SetBrush(wxBrush(c));
		wxPoint v[4]; 
		v[0]=wxPoint(p[0][0],p[0][1]); 
		v[1]=wxPoint(p[1][0],p[1][1]); 
		v[2]=wxPoint(p[2][0],p[2][1]); 
		v[3]=wxPoint(p[3][0],p[3][1]); 
		dc->DrawPolygon(4,v);
		np=0;
	} break;
	case GL_TRIANGLES: if (np==3) {
		dc->SetPen(pt);
		dc->SetBrush(wxBrush(c));
		wxPoint v[3]; 
		v[0]=wxPoint(p[0][0],p[0][1]); 
		v[1]=wxPoint(p[1][0],p[1][1]); 
		v[2]=wxPoint(p[2][0],p[2][1]); 
		dc->DrawPolygon(3,v);
		np=0;
	} break;
	case GL_QUAD_STRIP: if (np==4) {
		dc->SetPen(pt);
		dc->SetBrush(wxBrush(c));
		wxPoint v[4]; 
		v[0]=wxPoint(p[0][0],p[0][1]); 
		v[1]=wxPoint(p[1][0],p[1][1]); 
		v[3]=wxPoint(p[2][0],p[2][1]); 
		v[2]=wxPoint(p[3][0],p[3][1]); 
		dc->DrawPolygon(4,v);
		np=2; 
		p[0][0]=p[2][0]; p[0][1]=p[2][1];
		p[1][0]=p[3][0]; p[1][1]=p[3][1];
	} break;
	}
}

void glVertex2i(int x, int y) {
	p[np][0]=m.x(x);
	p[np][1]=m.y(y);
	np++; post_vertex();
}

void glVertex2iv(int *v) {
	glVertex2i(v[0],v[1]);
}

void glVertex2d(double x, double y) {
	p[np][0]=m.x(x);
	p[np][1]=m.y(y);
	np++; post_vertex();
}

void glPushMatrix() {
	ms.push(m);
}

void glPopMatrix() {
	m=ms.top();
	ms.pop();
}

void glScaled(double x, double y, double z) {
	m.scale(x,y);
}

void glTranslated(double x, double y, double z) {
	m.translate(x,y);
}

void glLineWidth(float w) {
	lw=w; if (lw<1) lw=1;
}

static wxFont &get_font() {
	static bool initialized = false;
	static wxFont font(wxFontInfo(m.sx*150).Family(wxFONTFAMILY_MODERN));
	if (!initialized) { 
		font.SetPixelSize(wxSize(0,250*m.sx)); initialized=true; 
#ifndef __APPLE__
		if (wxFont::AddPrivateFont("Inconsolata-Regular.ttf"))
#endif
			font.SetFaceName("Inconsolata");
	}
	return font;
}

// copiar de mxSource.cpp
#define UOP_ASIGNACION L'\u2190'
#define UOP_LEQUAL L'\u2264'
#define UOP_GEQUAL L'\u2265'
#define UOP_NEQUAL L'\u2260'
#define UOP_AND L'\u2227'
#define UOP_OR L'\u2228'
#define UOP_NOT L'\u00AC'
#define UOP_POWER L'\u2191'

// copiar de Entity.cpp
static const unsigned char SC_FLECHA = 27;
static const unsigned char SC_DISTINTO = 29;
static const unsigned char SC_MEN_IGUAL = 30;
static const unsigned char SC_MAY_IGUAL = 31;


void dibujar_caracter(char _c) {
	dc->SetFont(get_font());
	dc->SetTextForeground(c);
	wxString s = _S2W(std::string(1,_c)); 
	switch (_c){
	case SC_FLECHA:    s = wxString(UOP_ASIGNACION,1); break;
	case SC_DISTINTO:  s = wxString(UOP_NEQUAL,1); break;
	case SC_MEN_IGUAL: s = wxString(UOP_LEQUAL,1); break;
	case SC_MAY_IGUAL: s = wxString(UOP_GEQUAL,1); break;
	}
	int w=100*m.sx,tw,th;
	dc->GetTextExtent(s,&tw,&th);
	dc->DrawText(s,m.x(0)+(w-tw)/2,m.y(0)-3*th/4);
	m.tx+=100*m.sx*8/7;
}

void begin_texto( ) {
}

void end_texto( ) {
}
