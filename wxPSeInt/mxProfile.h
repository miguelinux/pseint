#ifndef MXPROFILE_H
#define MXPROFILE_H
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include "ConfigManager.h"
#include <wx/imaglist.h>
#include <wx/arrstr.h>

class mxProfile: public wxDialog {
private:
	wxListCtrl *list;
	wxTextCtrl *text;
	wxTextCtrl *search;
	wxArrayString perfiles;
	wxArrayString descripciones;
	LangSettings lang;
public:
	mxProfile(wxWindow *parent);
	void OnClose(wxCloseEvent &evt);
	void OnListSelect(wxListEvent &evt);
	void OnListActivate(wxListEvent &evt);
	wxString GetListSelection();
	void SetListSelection(int i);
	void OnOptionsButton(wxCommandEvent &evt);
	void OnLoadButton(wxCommandEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void UpdateDetails();
	void OnSearchText(wxCommandEvent &evt);
	void Search();
	DECLARE_EVENT_TABLE();
};

#endif

