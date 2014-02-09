#ifndef EVENTS_H
#define EVENTS_H
#include <cstddef>
using namespace std;

enum MENU_OPTIONS { MO_NONE=0, MO_ZOOM_EXTEND, MO_FUNCTIONS, MO_RUN, MO_DEBUG, MO_EXPORT, /*MO_SAVE_CLOSE,*/ MO_CLOSE, MO_HELP, 
	MO_SAVE, MO_UPDATE};

void Salir(bool force=false);

void Raise();

void ZoomExtend(int x0, int y0, int x1, int y1, float max=10);

void ProcessMenu(int op);

void ToggleEditable();

class LineInfo;
void FocusEntity(LineInfo *li);

void SetModified();


enum MOUSE_BUTTONS { MB_MIDDLE, MB_LEFT, MB_RIGHT, MB_WHEEL_UP, MB_WHEEL_DOWN, MB_DOWN, MB_UP };

// old glut callbacks
void reshape_cb (int w, int h);
void idle_func();
void passive_motion_cb(int x, int y);
void motion_cb(int x, int y);
void mouse_cb(int button, int state, int x, int y);
void keyboard_cb(unsigned char key/*, int x, int y*/);
void keyboard_esp_cb(int key/*, int x, int y*/);
	
	
#endif

