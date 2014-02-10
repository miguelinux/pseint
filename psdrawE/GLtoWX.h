#ifndef GLTOWX_H
#define GLTOWX_H

class wxDC;
extern wxDC *dc;

enum {GL_NONE, GL_LINES,GL_LINE_STRIP,GL_LINE_LOOP,GL_QUADS,GL_QUAD_STRIP};
enum {GLUT_KEY_LEFT,GLUT_KEY_RIGHT,GLUT_KEY_HOME,GLUT_KEY_END};

void glColor3fv(const float *v);
void glBegin(int what);
void glEnd();
void glVertex2i(int x, int y);
void glVertex2iv(int *v);
void glVertex2d(double x, double y);
void glPushMatrix();
void glPopMatrix();
void glScaled(double x, double y, double z);
void glTranslated(double x, double y, double z);
void glLineWidth(float w);


#endif

