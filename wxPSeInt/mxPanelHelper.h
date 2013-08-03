#ifndef MXPANELHELPER_H
#define MXPANELHELPER_H
#include <wx/bitmap.h>
#include <wx/panel.h>

// reemplaza all wxBitmapButton, porque no funcionaba en mac (el tamaño estaba mal 
// y no soportaba bien la transparencia), y asi ademas obtengo el mismo look-and-feel 
// en todas las plataformas, es para los botones laterales que abren los paneles
class mxPanelHelper:public wxPanel {
	wxBitmap bmp;
	wxWindowID mid;
	bool selected;
public:
	mxPanelHelper(wxWindow* parent, wxWindowID id, wxString bitmap);
	void OnEnter(wxMouseEvent &evt);
	void OnLeave(wxMouseEvent &evt);
	void OnClick(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
//	DECLARE_EVENT_TABLE();
};

#endif

