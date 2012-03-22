#ifndef DRAW_H
#define DRAW_H
#include <string>
using namespace std;

void dibujar_caracter(const char chr);

void GetTextSize(const string &label, int &w, int &h);

void DrawText(const float *color, int x, int y, const char *t);

void display_cb();

#endif

