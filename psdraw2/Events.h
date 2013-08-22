#ifndef EVENTS_H
#define EVENTS_H
#include <cstddef>
using namespace std;

enum MENU_OPTIONS { MO_NONE=0, MO_ZOOM_EXTEND, MO_FUNCTIONS, MO_RUN, MO_DEBUG, MO_EXPORT, /*MO_SAVE_CLOSE,*/ MO_CLOSE, MO_HELP, 
	MO_SAVE, MO_UPDATE};

void initialize();

void Salir(bool force=false);

void Raise();

void ZoomExtend(int x0, int y0, int x1, int y1, float max=10);

void ProcessMenu(int op);

void ToggleEditable();

class LineInfo;
void FocusEntity(LineInfo *li);

void SetModified();

#endif

