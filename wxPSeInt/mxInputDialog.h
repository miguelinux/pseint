#ifndef MXINPUTDIALOG_H
#define MXINPUTDIALOG_H
#include <wx/dialog.h>

class wxCheckBox;
class wxTextCtrl;
class mxInputDialog : public wxDialog {
private:
	wxCheckBox *check_use, *check_partial;
	wxTextCtrl *values;
	wxString old_values;
protected:
public:
	mxInputDialog(wxWindow *parent);
	void Show();
	void OnCheckUse(wxCommandEvent &evt);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	bool HasInput();
	bool IsPartial();
	wxString GetInput();
	DECLARE_EVENT_TABLE();
};

#endif

