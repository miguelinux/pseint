#ifndef DRAW_H
#define DRAW_H
#include <string>
using namespace std;

enum CURSORES {	Z_CURSOR_INHERIT, Z_CURSOR_CROSSHAIR, Z_CURSOR_HAND, Z_CURSOR_TEXT, Z_CURSOR_DESTROY, Z_CURSOR_NONE, Z_CURSOR_MOVE, Z_CURSOR_COUNT };
extern CURSORES mouse_cursor;
void display_cb();
void SetStatus(const float *color, const string &text);

#endif

