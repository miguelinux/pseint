#include "mxExportPreview.h"
#include <wx/sizer.h>
#include "ids.h"
#include "mxUtils.h"
#include <wx/combobox.h>
#include <wx/stattext.h>
#include "mxMainWindow.h"
#include "string_conversions.h"

//mxExportPreview *export_preview=NULL;

BEGIN_EVENT_TABLE(mxExportPreview,wxFrame)
	EVT_BUTTON(mxID_EXPPREV_EXPORT,mxExportPreview::OnButtonExport)
	EVT_BUTTON(mxID_EXPPREV_UPDATE,mxExportPreview::OnButtonUpdate)
	EVT_COMBOBOX(mxID_EXPPREV_LANG,mxExportPreview::OnComboLang)
	EVT_CLOSE(mxExportPreview::OnClose)
END_EVENT_TABLE()

mxExportPreview::mxExportPreview():wxFrame(main_window,wxID_ANY,_Z("Exportar - Vista previa"),wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *top_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags sz; sz.Center();
	top_sizer->Add( new wxStaticText(this,wxID_ANY,_Z("Lenguaje: ")),sz );
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	wxArrayString langs_list;
	for (int id = mxID_FILE_EXPORT_LANG_FIRST+1; id < int(mxID_FILE_EXPORT_LANG_LAST); id++)
		langs_list.Add(utils->GetExportLangName(id).AfterFirst(' '));
	combo_lang = new wxComboBox(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,langs_list,wxCB_READONLY);
	combo_lang->Select(mxID_FILE_EXPORT_LANG_CPP03-mxID_FILE_EXPORT_LANG_FIRST-1);
	top_sizer->Add(combo_lang,sz);
	top_sizer->Add(new wxButton(this,mxID_EXPPREV_UPDATE,_Z("Actualizar")),sz);
	top_sizer->AddStretchSpacer();
	top_sizer->Add(new wxButton(this,mxID_EXPPREV_EXPORT,_Z("Exportar...")),sz);
	main_sizer->Add(top_sizer,wxSizerFlags().Proportion(0).Expand());
	code_ctrl = new wxStyledTextCtrl(this,wxID_ANY);
	main_sizer->Add(code_ctrl,wxSizerFlags().Proportion(1).Expand());
	SetSizer(main_sizer);
//	export_preview = this;
	Show();
}

void mxExportPreview::OnButtonExport (wxCommandEvent & event) {
	int pos = combo_lang->GetCurrentSelection();
	wxCommandEvent evt(0,mxID_FILE_EXPORT_LANG_FIRST+1+pos);
	main_window->OnFileExportLang(evt);
}

void mxExportPreview::OnButtonUpdate (wxCommandEvent & event) {
	UpdatePrev();
}

void mxExportPreview::OnComboLang (wxCommandEvent & event) {
	UpdatePrev();
}

void mxExportPreview::OnClose (wxCloseEvent & event) {
	/*export_preview=NULL; */Destroy();
}

void mxExportPreview::UpdatePrev ( ) {
	
}

