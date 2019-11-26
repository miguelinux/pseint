#include "Text.h"
#define GL_LITE_IMPLEMENTATION // antes de incluir GLstuff
#include "GLstuff.h"
#undef GL_LITE_IMPLEMENTATION // antes de incluir GLstuff
#include "Textures.h"
#include <iostream>
#include "Global.h"

void GetTextSize(const string &label, int &w, int &h) {
	w=label.size()*10;
	h=15;
}

#define c2i(ch) ((int)((unsigned char)(ch)))

void DrawTextRaster(const float *color, int x, int y, const char *t) {
	glColor3fv(color);
	glRasterPos2f(x,y);
	
	struct char_info { 
		char c; bool acento, mayusculas, enie; 
		void set_char(char c) { acento = mayusculas = enie = false; this->c = c; }
		void set_acento(char x) { c = x; acento = true; mayusculas = x>='A'&&x<='Z'; }
		void set_enie(char x) { c = x; enie = true; mayusculas = x>='A'&&x<='Z'; }
	};
	static char_info char_map[256];
	static bool map_inicializado = false;
	if (!map_inicializado) {
		for(int i=0;i<256;i++)
			char_map[i].set_char((char)i);
		char_map[c2i('á')].set_acento('a');
		char_map[c2i('é')].set_acento('e');
		char_map[c2i('í')].set_acento('i');
		char_map[c2i('ó')].set_acento('o');
		char_map[c2i('ú')].set_acento('u');
		char_map[c2i('Á')].set_acento('A');
		char_map[c2i('É')].set_acento('E');
		char_map[c2i('Í')].set_acento('I');
		char_map[c2i('Ó')].set_acento('O');
		char_map[c2i('Ú')].set_acento('U');
		char_map[c2i('ñ')].set_enie('n');
		char_map[c2i('Ñ')].set_enie('N');
		map_inicializado = true;
	}
	for(int i=0;t[i]!='\0';++i) {
		char_info &ci = char_map[c2i(t[i])];
		glutBitmapCharacter(big_icons?GLUT_BITMAP_TIMES_ROMAN_24:GLUT_BITMAP_9_BY_15,ci.c);
		if (ci.acento) {
			glLineWidth(1);
			glBegin(GL_LINES);
			if (ci.mayusculas) {
				glVertex2i(x+i*9+4,y+11);
				glVertex2i(x+i*9+6,y+14);
			} else {
				glVertex2i(x+i*9+4,y+9);
				glVertex2i(x+i*9+6,y+12);
			}
			glEnd();
		} else if (ci.enie) {
			glLineWidth(1);
			glBegin(GL_LINES);
			if (ci.mayusculas) {
				glVertex2i(x+i*9+2,y+11);
				glVertex2i(x+i*9+7,y+11);
			} else {
				glVertex2i(x+i*9+3,y+8);
				glVertex2i(x+i*9+6,y+8);
			}
			glEnd();
		}
	}
}


static double x_texto = 0;

#ifdef _USE_DF

bool CompilerInfo(GLuint id){
  int len; glGetShaderiv(id,GL_INFO_LOG_LENGTH,&len); // cant de caracteres
  if (len){
    char * infoLog=(char *)malloc(sizeof(char)*(len+1));
    glGetShaderInfoLog(id,len+1,NULL,infoLog);
    cout << "ERROR COMPILING SHADER: " << infoLog << endl;
    free(infoLog);
  }
  int status; glGetShaderiv(id,GL_COMPILE_STATUS,&status); return status;
}

// imprime la info del linker de la GPU
//bool LinkerInfo(GLuint id){
//  int len; glGetProgramiv(id,GL_INFO_LOG_LENGTH,&len); // cant de caracteres
//  if (len){
//    char * infoLog=(char *)malloc(sizeof(char)*(len+1));
//    glGetProgramInfoLog(id,len+1,NULL,infoLog);
//    cout << "ERROR LINKING SHADER: " << infoLog << endl;
//    free(infoLog);
//  }
//  int status; glGetProgramiv(id,GL_LINK_STATUS,&status); return status;
//}
GLuint progID;
bool init_shaders() {
	static bool inited = false;
	static bool shader_ok = false;
	if (inited) return shader_ok;
	
	if (!gl_lite_init()) return false;
	inited = true;
	
	const char *sf4 = // 4 muestrar regulares
		"#version 110\n"
		"uniform sampler2D texture1;\n"
		"void main() {\n"
		"    gl_FragColor.rgb = gl_Color.rgb;\n"
		"    vec2 st = gl_TexCoord[0].st;"
		"	 vec2 d = fwidth(gl_TexCoord[0].st)/2.0; st -= d/2.0;"
		"    float tol = d.x>0.003 ? 0.3 : 0.5; " // si se aleja, bajar la tol para que no queden tan finas
		"    float      a = texture2D(texture1, st).a >tol ? 1.0 : 0.0;\n"
		"    st.x+=d.x; a+= texture2D(texture1, st).a >tol ? 1.0 : 0.0;\n"
		"    st.y+=d.y; a+= texture2D(texture1, st).a >tol ? 1.0 : 0.0;\n"
		"    st.x-=d.x; a+= texture2D(texture1, st).a >tol ? 1.0 : 0.0;\n"
		"    gl_FragColor.a = a/4.0;\n"
		"}\n";
//	const char *sfR = // 4 muestras rotadas... no queda tan bien como debería
//		"#version 110\n"
//		"uniform sampler2D texture1;\n"
//		"void main() {\n"
//		"    gl_FragColor.rgb = gl_Color.rgb;\n"
//		"    vec2 st = gl_TexCoord[0].st;"
//		"	 vec2 d = fwidth(gl_TexCoord[0].st);"
//		"    float      a = texture2D(texture1, st+vec2(-d.x*0.17,+d.y*0.36)).a >0.5 ? 1.0 : 0.0;\n"
//		"               a+= texture2D(texture1, st+vec2(+d.x*0.17,-d.y*0.36)).a >0.5 ? 1.0 : 0.0;\n"
//		"               a+= texture2D(texture1, st+vec2(-d.x*0.36,+d.y*0.17)).a >0.5 ? 1.0 : 0.0;\n"
//		"               a+= texture2D(texture1, st+vec2(+d.x*0.36,-d.y*0.17)).a >0.5 ? 1.0 : 0.0;\n"
//		"    gl_FragColor.a = a/4.0;\n"
//		"}\n";
//	const char *sf9 = // 9 muestras, queda mejor pero es mas caro
//		"#version 110\n"
//		"uniform sampler2D texture1;\n"
//		"void main() {\n"
//		"    gl_FragColor.rgb = gl_Color.rgb;\n"
//		"    vec2 st = gl_TexCoord[0].st;"
//		"	 vec2 d = fwidth(gl_TexCoord[0].st)/3.0;"
//		"    float      a = texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    st.x+=d.x; a += texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    st.y+=d.y; a += texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    st.x-=d.x; a += texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    st.x-=d.x; a += texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    st.y-=d.y; a += texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    st.y-=d.y; a += texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    st.x+=d.x; a += texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    st.x+=d.x; a += texture2D(texture1, st).a >0.5 ? 1.0 : 0.0;\n"
//		"    gl_FragColor.a = a/9.0;\n"
//		"}\n";
	
	GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(f,1,&sf4,NULL);
	glCompileShader(f);
	shader_ok = CompilerInfo(f);
	if (!shader_ok) return false;
	
	progID = glCreateProgram();
	glAttachShader(progID,f);
	glLinkProgram(progID); 
//	shader_ok = LinkerInfo(progID); 
	
	return shader_ok = true;
}

//# endif
#endif


void begin_texto( ) {
	x_texto = 0;
	texture_font.Select();
#ifdef _USE_DF
	if (init_shaders()) {
		glUseProgram(progID);
	} else {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER,0.5);
		glBlendFunc(GL_ONE,GL_ZERO);
	}
#endif
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
}

void end_texto( ) {
	glEnd();
	glDisable(GL_TEXTURE_2D);
# ifdef _USE_DF
	if (init_shaders()) {
		glUseProgram(0);
	} else {
		glPopAttrib();
	}
# endif
}


void dibujar_caracter(const char chr) {
	const double f=1.0/16.0;
	const int w=120, h=150,y0=-25;
	double x0=x_texto;
	unsigned char ch=chr;
	int r=15-ch/16, c=ch%16;
	glTexCoord2f(c*f,r*f);
	glVertex2i(x0,y0);
	glTexCoord2f(c*f+f,r*f);
	glVertex2i(x0+w,y0);
	glTexCoord2f(c*f+f,r*f+f);
	glVertex2i(x0+w,y0+h);
	glTexCoord2f(c*f,r*f+f);
	glVertex2i(x0,y0+h);
	x_texto += 800.0/7.0;
}

