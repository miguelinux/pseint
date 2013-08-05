#include "mxPanelHelper.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxPanelHelper,wxPanel)
	EVT_ENTER_WINDOW(mxPanelHelper::OnEnter)
	EVT_LEAVE_WINDOW(mxPanelHelper::OnLeave)
	EVT_PAINT(mxPanelHelper::OnPaint)
	EVT_LEFT_DOWN(mxPanelHelper::OnClick)
END_EVENT_TABLE()
	
mxPanelHelper::mxPanelHelper(wxWindow *parent, wxWindowID id, wxString bitmap):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize) {
	mid=id; selected=false;
	bmp.LoadFile(bitmap,wxBITMAP_TYPE_PNG);
	SetMinSize(wxSize(bmp.GetWidth()+6,bmp.GetHeight()+8));
	SetSize(wxSize(bmp.GetWidth()+6,bmp.GetHeight()+8));
}

void mxPanelHelper::OnEnter(wxMouseEvent &evt) { 
	selected=true; Refresh(); 
}

void mxPanelHelper::OnLeave(wxMouseEvent &evt) { 
	selected=false; Refresh(); 
}

void mxPanelHelper::OnClick(wxMouseEvent &evt) {
	wxCommandEvent cmd(wxEVT_COMMAND_BUTTON_CLICKED,mid);
	main_window->GetEventHandler()->ProcessEvent(cmd);
}

void mxPanelHelper::OnPaint(wxPaintEvent &evt) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetBackground(wxSystemSettings::GetColour(selected?wxSYS_COLOUR_3DHIGHLIGHT:wxSYS_COLOUR_3DFACE));
	dc.Clear();
	dc.DrawBitmap(bmp,3,4,true);
}
