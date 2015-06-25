#ifndef TEXT_H
#define TEXT_H
#include <string>
using namespace std;

extern bool use_textures_font;

void begin_texto(); 
void dibujar_caracter(const char chr); 
void dibujar_caracter(const char chr, bool extra);
void end_texto(); 

void GetTextSize(const string &label, int &w, int &h);
//void DrawText(const float *color, int x, int y, const char *t);
void DrawTextRaster(const float *color, int x, int y, const char *t);

#endif

