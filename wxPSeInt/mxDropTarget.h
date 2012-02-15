#ifndef MXDROPTARGET_H
#define MXDROPTARGET_H

#include <wx/dnd.h>

class mxDropTarget:public wxDropTarget {
private:
	wxFileDataObject *data;
public:
	bool OnDrop(wxCoord x, wxCoord y);
	wxDragResult  OnData(wxCoord x, wxCoord y, wxDragResult def);
	mxDropTarget();
	~mxDropTarget();
};

#endif
