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
#include <wx/bmpbuttn.h>

BEGIN_EVENT_TABLE(mxConfig,wxDialog)
	EVT_BUTTON(wxID_OK,mxConfig::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxConfig::OnCancelButton)
	EVT_BUTTON(wxID_OPEN,mxConfig::OnOpenButton)
	EVT_BUTTON(wxID_SAVE,mxConfig::OnSaveButton)
	EVT_CLOSE(mxConfig::OnClose)
END_EVENT_TABLE()

mxConfig::mxConfig(wxWindow *parent, LangSettings &settings )
	: wxDialog(parent,wxID_ANY,"Opciones del Lenguaje",wxDefaultPosition,wxDefaultSize),
	  lang(settings)
{
	lang.source = LS_CUSTOM; lang.name = CUSTOM_PROFILE;
	
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *opts_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	for(int i=0;i<LS_COUNT;i++) { 
		chk[i] = utils->AddCheckBox(opts_sizer,this,LangSettings::data[i].user_desc,LangSettings::data[i].default_value);
		chk[i]->SetToolTip(utils->FixTooltip(LangSettings::data[i].long_desc));
	}
	
	wxButton *load_button = new mxBitmapButton (this, wxID_OPEN, bitmaps->buttons.load, "Cargar...");
	wxButton *save_button = new mxBitmapButton (this, wxID_SAVE, bitmaps->buttons.save, "Guardar...");
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
	
	ReadFromStruct(lang);
	
	SetSizerAndFit(sizer);
//	CentreOnParent();
}

void mxConfig::OnClose(wxCloseEvent &evt) {
	wxCommandEvent e;
	OnCancelButton(e);
}

void mxConfig::OnOkButton(wxCommandEvent &evt) {
	if (!chk[LS_WORD_OPERATORS]->GetValue()&&chk[LS_COLOQUIAL_CONDITIONS]->GetValue())
		wxMessageBox("No se puede desactivar la opción \"Permitir las palabras Y, O, NO y MOD para los operadores &&, |, ~ y %\" sin desactivar también \"Permitir condiciones en lenguaje coloquial\", por lo que la primera permanecerá activa.");
	CopyToStruct(lang);
	EndModal(1);
}

void mxConfig::OnCancelButton(wxCommandEvent &evt) {
	EndModal(0);
}

wxString mxConfig::LoadFromFile (wxWindow *parent) {
	wxFileDialog dlg (parent, "Cargar perfil desde archivo", config->last_dir, " ", "Cualquier Archivo (*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST );
	if (dlg.ShowModal() != wxID_OK) return wxEmptyString;
	config->last_dir = wxFileName(dlg.GetPath()).GetPath();
	return dlg.GetPath();
}

void mxConfig::OnOpenButton (wxCommandEvent & evt) {
	wxString file = LoadFromFile(this);
	if (file.IsEmpty()) return;
	LangSettings l(LS_INIT);
	l.Load(file,false);
	ReadFromStruct(l);
}

bool TodoMayusculas(const wxString &desc) {
	return desc.Len()>10 && desc == desc.Upper();
}

bool TodoMayusculas(const std::string &desc) {
	std::string copy = desc;
	for(size_t i=0;i<copy.size();i++) copy[i]=toupper(copy[i]);
	return desc.size()>10 && desc==copy;
}

void mxConfig::OnSaveButton (wxCommandEvent & evt) {
	wxFileDialog dlg (this, "Guardar perfil en archivo", config->last_dir, "", "Cualquier Archivo (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK) {
		config->last_dir=wxFileName(dlg.GetPath()).GetPath();
		LangSettings l(LS_INIT);
		while(true) {
			l.descripcion = wxGetTextFromUser(_Z("Ingrese una descripción del perfil (por\n"
											  "favor incluya materia, carrera, institución\n"
											  "y nombre del docente)."),_Z("Guardar Perfil"),"",this);
			if (l.descripcion.empty()) return;
			if (l.descripcion.size()<10 || !TodoMayusculas(l.descripcion)) break;
			wxMessageBox(_Z("¡NO ME GRITE!"),_Z("Por favor"),wxOK|wxICON_ERROR);
		}
		CopyToStruct(l);
		l.Save(dlg.GetPath());
		wxMessageBox(_ZZ("Perfil guardado en \"")+dlg.GetPath()+_Z("\""));
	}
}

void mxConfig::ReadFromStruct (LangSettings l) {
	for(int i=0;i<LS_COUNT;i++) chk[i]->SetValue(l[LS_ENUM(i)]);
}

void mxConfig::CopyToStruct (LangSettings & l) {
	for(int i=0;i<LS_COUNT;i++) l[LS_ENUM(i)]=chk[i]->GetValue();
}

