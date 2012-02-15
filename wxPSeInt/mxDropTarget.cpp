#include "mxDropTarget.h"

#include "mxUtils.h"
#include "mxMainWindow.h"

mxDropTarget::mxDropTarget() {
	SetDataObject (data = new wxFileDataObject());
}

mxDropTarget::~mxDropTarget() {
	
}

bool mxDropTarget::OnDrop(wxCoord x, wxCoord y) {
	return true;
}

wxDragResult  mxDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def) {
	GetData();
	wxArrayString files = data->GetFilenames();
	for (unsigned int i=0;i<files.GetCount();i++)
		main_window->OpenProgram(files[i]);
	return def;
}
