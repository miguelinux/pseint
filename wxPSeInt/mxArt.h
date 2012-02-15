#ifndef MXART_H
#define MXART_H

#include <wx/bitmap.h>
#include <wx/clrpicker.h>

#define ICON_SIZE 20

//! Estructura para almacenar recuros gráficos comunes
class mxArt {
public:
	struct {
		wxBitmap *ok;
		wxBitmap *cancel;
		wxBitmap *stop;
		wxBitmap *help;
		wxBitmap *next;
		wxBitmap *prev;
		wxBitmap *find;
		wxBitmap *replace;
	} buttons;
//	struct {
//		wxBitmap *question;
//		wxBitmap *warning;
//		wxBitmap *info;
//		wxBitmap *error;
//	} icons;
	mxArt(wxString img_dir);
	~mxArt();
};

extern mxArt *bitmaps;

#endif

