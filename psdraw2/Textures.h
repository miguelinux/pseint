#ifndef TEXTURES_H
#define TEXTURES_H

#ifdef _USE_TEXTURES
#include <GL/gl.h>
void LoadTextures();
struct Texture {
	GLuint id; 
	int w,h; float r;
	bool smooth;
	void Load(const char *fname);
	void Select();
};
extern Texture texture_shapes;
extern Texture texture_commands;
extern Texture texture_menu;
extern Texture texture_trash;
#endif

#endif

