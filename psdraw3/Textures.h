#ifndef TEXTURES_H
#define TEXTURES_H

#ifdef _USE_TEXTURES
#include "GLstuff.h"
extern bool use_textures;
bool LoadTextures();
struct Texture {
	GLuint id; 
	int w,h; float r, max_s, max_t;
	bool smooth;
	bool Load(const char *fname);
	void Select();
};
extern Texture texture_shapes;
extern Texture texture_commands;
extern Texture texture_menu;
extern Texture texture_trash;
#ifdef _USE_FONT
extern Texture texture_font;
#endif
#endif

#endif

