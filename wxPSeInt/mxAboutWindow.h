#ifndef MX_ABOUT_WINDOW_H
#define MX_ABOUT_WINDOW_H

#include <wx/dialog.h>
#include <wx/html/htmlwin.h>

class mxAboutWindow : public wxDialog {
	wxString version_info;
	wxString GetVersion(wxString exe);
public:
	mxAboutWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	void OnCloseButton(wxCommandEvent &event);
	void OnLink (wxHtmlLinkEvent &event);
	void OnClose(wxCloseEvent &event);
	wxString MakePageText(bool full);
	wxHtmlWindow *html;

private:
	DECLARE_EVENT_TABLE()

};

#endif
