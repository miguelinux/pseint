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
		wxBitmap *help;
		wxBitmap *options;
		wxBitmap *find;
		wxBitmap *replace;
		wxBitmap *load;
		wxBitmap *save;
	} buttons;
	mxArt(wxString img_dir);
	~mxArt();
};

extern mxArt *bitmaps;

#endif

