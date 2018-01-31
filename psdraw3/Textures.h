#ifndef TEXTURES_H
#define TEXTURES_H

#ifdef _USE_TEXTURES
#include "GLstuff.h"
#include <set>
#include <string>
#include <wx/string.h>
using namespace std;

extern bool use_textures;
class Texture {
	static set<Texture*> m_to_load; // delayed loading, requires a GL context to properly load a texture
	bool Load();
	wxString m_fname;
	GLuint m_id;
public:
	Texture() : m_id(-1) { }
	void SetTexture(wxString fname) { m_fname = fname; m_to_load.insert(this); }
	Texture(wxString fname) : m_fname(fname), m_id(-1) { m_to_load.insert(this); }
	~Texture() { m_to_load.erase(this); }
	int w,h; 
	float r, max_s, max_t;
	void Select() { glBindTexture(GL_TEXTURE_2D,m_id); }
	static bool LoadTextures();
};
#ifdef _USE_FONT
extern Texture texture_font;
#endif
#endif

#endif

