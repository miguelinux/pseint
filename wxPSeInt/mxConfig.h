#ifndef MXCONFIG_H
#define MXCONFIG_H
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include "ConfigManager.h"
#include "../pseint/LangSettings.h"

class mxConfig:wxDialog {
private:
	wxCheckBox *chk[LS_COUNT];
	void ReadFromStruct(LangSettings l);
	void CopyToStruct(LangSettings &l);
public:
	mxConfig(wxWindow *parent);
	~mxConfig();
	void OnClose(wxCloseEvent &evt);
	void OnOpenButton(wxCommandEvent &evt);
	void OnSaveButton(wxCommandEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

