#ifndef MXHELPWINDOW_H
#define MXHELPWINDOW_H

#include <wx/frame.h>
#include <wx/sashwin.h>
#include <wx/html/htmlwin.h>
#include "mxUtils.h"

class wxHtmlWindow;
class wxTextCtrl;
class wxTreeCtrl;
class wxSashLayoutWindow;
class wxBoxSizer;

class mxHelpWindow:public wxFrame {
private:
	DECLARE_EVENT_TABLE();
	wxBoxSizer *bottomSizer;
	wxHtmlWindow *html;
	wxTextCtrl *search_text;
	wxTreeCtrl *tree;
	wxSashLayoutWindow *index_sash;
	HashStringTreeItem items;
	void OnTree(wxTreeEvent &event);
	void OnLink (wxHtmlLinkEvent &event);
public:
	void ShowIndex();
	void ShowHelp(wxString file);
	void OnHideTree(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnHome(wxCommandEvent &event);
	void OnPrev(wxCommandEvent &event);
	void OnNext(wxCommandEvent &event);
	void OnCopy(wxCommandEvent &event);
	void OnSearch(wxCommandEvent &event);
	void OnSearchAll(wxCommandEvent &event);
	void OnSashDrag (wxSashEvent& event);
	void OnCharHook(wxKeyEvent &evt);
	mxHelpWindow(wxString file=_T("index.html"));
	~mxHelpWindow();
};

extern mxHelpWindow *helpw;

#endif

