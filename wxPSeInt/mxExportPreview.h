#ifndef MXEXPORTPREVIEW_H
#define MXEXPORTPREVIEW_H
#include <wx/frame.h>

class wxComboBox;
class wxStyledTextCtrl;

class mxExportPreview : public wxFrame {
	
	wxStyledTextCtrl *code_ctrl;
	wxComboBox *combo_lang;
		
public:
	mxExportPreview();
	void OnButtonExport(wxCommandEvent &event);
	void OnButtonUpdate(wxCommandEvent &event);
	void OnComboLang(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void UpdatePrev();
	DECLARE_EVENT_TABLE();
};

//extern mxExportPreview *export_preview;

#endif

