#ifndef MXPROFILE_H
#define MXPROFILE_H
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include "ConfigManager.h"
#include <wx/imaglist.h>
#include <wx/arrstr.h>

class mxProfile:wxDialog {
private:
	wxListCtrl *list;
	wxTextCtrl *text;
	wxTextCtrl *search;
	wxArrayString perfiles;
	wxArrayString descripciones;
	wxString old_profile; // nombre del perfil que estaba al ingresar a este cuadro (para restaurar si se cancela)
	LangSettings old_config; // detalles de la configuración que estaba al ingresar a este cuadro (para restaurar si se cancela)
public:
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
	void OnSearchText(wxCommandEvent &evt);
	void Search();
	DECLARE_EVENT_TABLE();
};

#endif

