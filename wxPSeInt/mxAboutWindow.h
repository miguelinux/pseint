#ifndef MX_ABOUT_WINDOW_H
#define MX_ABOUT_WINDOW_H

#include <wx/dialog.h>

class wxHtmlWindow;

class mxAboutWindow : public wxDialog {

public:
	mxAboutWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	void OnCloseButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	wxString MakePageText();
	wxHtmlWindow *html;

private:
	DECLARE_EVENT_TABLE()

};

#endif
