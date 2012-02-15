#ifndef MX_DESKTOP_VARS_EDITOR_H
#define MX_DESKTOP_VARS_EDITOR_H
#include <wx/dialog.h>

class wxListBox;
class wxTextCtrl;
class wxCheckBox;

class mxDesktopVarsEditor : public wxDialog {
private:
	wxListBox *list;
	wxTextCtrl *my_text;
	wxCheckBox *check_doit;
	int sel;
public:
	mxDesktopVarsEditor(wxWindow *parent);
	~mxDesktopVarsEditor();
	void OnClose(wxCloseEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnUp(wxCommandEvent &evt);
	void OnDown(wxCommandEvent &evt);
	void OnDelete(wxCommandEvent &evt);
	void OnDeleteAll(wxCommandEvent &evt);
	void OnAdd(wxCommandEvent &evt);
	void OnEdit(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

