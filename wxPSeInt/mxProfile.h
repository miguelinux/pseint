#ifndef MXPROFILE_H
#define MXPROFILE_H
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include "ConfigManager.h"

class mxProfile:wxDialog {
private:
	wxListCtrl *list;
	wxTextCtrl *text;
public:
	LangSettings old_config;
	mxProfile(wxWindow *parent);
	~mxProfile();
	void OnClose(wxCloseEvent &evt);
	void OnListSelect(wxListEvent &evt);
	void OnListActivate(wxListEvent &evt);
	wxString GetListSelection();
	void SetListSelection(int i);
	void OnOptionsButton(wxCommandEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void LoadProfile();
	DECLARE_EVENT_TABLE();
};

#endif

