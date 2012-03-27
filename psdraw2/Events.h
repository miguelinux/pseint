#ifndef EVENTS_H
#define EVENTS_H

enum MENU_OPTIONS { MO_NONE=0, MO_ZOOM_EXTEND, MO_SAVE, MO_RUN, MO_SAVE_CLOSE, MO_CLOSE, MO_HELP};

void initialize();

void Salir(bool force=false);

void Raise();

void ZoomExtend(int x0, int y0, int x1, int y1, float max=10);

void ProcessMenu(int op);

#endif

