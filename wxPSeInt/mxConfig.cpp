#include "mxConfig.h"
#include <wx/sizer.h>
#include "ConfigManager.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "mxArt.h"
#include <wx/filedlg.h>
#include <wx/textdlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

BEGIN_EVENT_TABLE(mxConfig,wxDialog)
	EVT_BUTTON(wxID_OK,mxConfig::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxConfig::OnCancelButton)
	EVT_BUTTON(wxID_OPEN,mxConfig::OnOpenButton)
	EVT_BUTTON(wxID_SAVE,mxConfig::OnSaveButton)
	EVT_CLOSE(mxConfig::OnClose)
END_EVENT_TABLE()

mxConfig::mxConfig(wxWindow *parent):wxDialog(parent,wxID_ANY,"Opciones del Lenguaje",wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *opts_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	for(int i=0;i<LS_COUNT;i++) { 
		chk[i] = utils->AddCheckBox(opts_sizer,this,LangSettings::data[i].user_desc,LangSettings::data[i].default_value);
		chk[i]->SetToolTip(utils->FixTooltip(LangSettings::data[i].long_desc));
	}
	
	wxButton *load_button = new wxBitmapButton (this, wxID_OPEN, wxBitmap(DIR_PLUS_FILE(config->images_path,"boton_abrir.png"),wxBITMAP_TYPE_PNG));
	wxButton *save_button = new wxBitmapButton (this, wxID_SAVE, wxBitmap(DIR_PLUS_FILE(config->images_path,"boton_guardar.png"),wxBITMAP_TYPE_PNG));
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, "Aceptar");
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, "Cancelar");
	button_sizer->Add(load_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(save_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->AddStretchSpacer(1);
	button_sizer->Add(cancel_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(ok_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	
	sizer->Add(opts_sizer,wxSizerFlags().Proportion(1).Expand());
	sizer->Add(button_sizer,wxSizerFlags().Proportion(0).Expand());
	
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	ReadFromStruct(config->lang);
	
	SetSizerAndFit(sizer);
//	CentreOnParent();
	ShowModal();	
}

mxConfig::~mxConfig() {
	

}
void mxConfig::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxConfig::OnOkButton(wxCommandEvent &evt) {
	if (!chk[LS_WORD_OPERATORS]->GetValue()&&chk[LS_COLOQUIAL_CONDITIONS]->GetValue())
		wxMessageBox("No se puede desactivar la opción \"Permitir las palabras Y, O, NO y MOD para los operadores &&, |, ~ y %\" sin desactivar también \"Permitir condiciones en lenguaje coloquial\", por lo que la primera permanecerá activa.");
	CopyToStruct(config->lang);
	Close();
}

void mxConfig::OnCancelButton(wxCommandEvent &evt) {
	Close();
}


void mxConfig::OnOpenButton (wxCommandEvent & evt) {
	wxFileDialog dlg (this, "Cargar perfil desde archivo", config->last_dir, " ", "Cualquier Archivo (*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST );
	if (dlg.ShowModal() == wxID_OK) {
		config->last_dir=wxFileName(dlg.GetPath()).GetPath();
		LangSettings l(LS_INIT);
		l.Load(dlg.GetPath());
		ReadFromStruct(l);
	}
}

void mxConfig::OnSaveButton (wxCommandEvent & evt) {
	wxFileDialog dlg (this, "Guardar perfil en archivo", config->last_dir, " ", "Cualquier Archivo (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK) {
		config->last_dir=wxFileName(dlg.GetPath()).GetPath();
		LangSettings l(LS_INIT);
		l.descripcion=wxGetTextFromUser("Ingrese una descripción breve del perfil.","Guardar Perfil","",this);
		CopyToStruct(l);
		l.Save(dlg.GetPath());
	}
}

void mxConfig::ReadFromStruct (LangSettings l) {
	for(int i=0;i<LS_COUNT;i++) chk[i]->SetValue(l[LS_ENUM(i)]);
}

void mxConfig::CopyToStruct (LangSettings & l) {
	for(int i=0;i<LS_COUNT;i++) l[LS_ENUM(i)]=chk[i]->GetValue();
}

