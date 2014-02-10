#ifndef MXCONFIG_H
#define MXCONFIG_H
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include "ConfigManager.h"

class mxConfig:wxDialog {
private:
	wxCheckBox *chk_force_init_vars;
	wxCheckBox *chk_force_define_vars;
	wxCheckBox *chk_force_semicolon;
	wxCheckBox *chk_base_zero_arrays;
	wxCheckBox *chk_allow_concatenation;
	wxCheckBox *chk_overload_equal;
	wxCheckBox *chk_coloquial_conditions;
	wxCheckBox *chk_lazy_syntax;
	wxCheckBox *chk_word_operators;
	wxCheckBox *chk_allow_dinamyc_dimensions;
	wxCheckBox *chk_use_alternative_io;
	wxCheckBox *chk_use_nassi_schneiderman;
	wxCheckBox *chk_enable_string_functions;
	wxCheckBox *chk_enable_user_functions;
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

