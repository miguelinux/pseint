#include <wx/sizer.h>
#include <wx/dir.h>
#include <wx/stattext.h>
#include <wx/textfile.h>
#include <wx/imaglist.h>
#include "mxProfile.h"
#include "ConfigManager.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "mxConfig.h"
#include "mxArt.h"
#include "mxMainWindow.h"
#include "RTSyntaxManager.h"

#include "string_conversions.h"
#include "Logger.h"

BEGIN_EVENT_TABLE(mxProfile,wxDialog)
	EVT_TEXT(wxID_FIND,mxProfile::OnSearchText)
	EVT_LIST_ITEM_SELECTED(wxID_ANY,mxProfile::OnListSelect)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY,mxProfile::OnListActivate)
	EVT_BUTTON(wxID_ABOUT,mxProfile::OnOptionsButton)
	EVT_BUTTON(wxID_OPEN,mxProfile::OnLoadButton)
	EVT_BUTTON(wxID_OK,mxProfile::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxProfile::OnCancelButton)
	EVT_CLOSE(mxProfile::OnClose)
END_EVENT_TABLE()
	
static int comp_nocase(const wxString& first, const wxString& second) {
	return first.CmpNoCase(second);
}

mxProfile::mxProfile(wxWindow *parent) : 
	wxDialog(parent,wxID_ANY,_Z("Opciones del Lenguaje"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
	text(NULL), lang(cfg_lang)
{

	_LOG("mxProfile::mxProfile Start");
	
	wxDir dir(config->profiles_dir);
	if ( dir.IsOpened() ) {
		wxString filename;
		wxString spec;
		bool cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
		while ( cont ) {
			perfiles.Add(filename);
			cont = dir.GetNext(&filename);
		}
	}
	perfiles.Sort(comp_nocase);
	for(unsigned int i=0;i<perfiles.GetCount();i++) { 
		LangSettings l(LS_INIT); 
		l.Load(DIR_PLUS_FILE(config->profiles_dir,perfiles[i]),true);
		descripciones.Add(_Z(l.descripcion.c_str()));
	}
	
	list = new wxListCtrl(this,wxID_ANY,wxDefaultPosition,wxSize(250,300),wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
	list->InsertColumn(0,_Z("Perfil"));
	wxImageList *iml = new wxImageList(24,24,true);
	wxBitmap noimage(DIR_PLUS_FILE(DIR_PLUS_FILE(config->profiles_dir,"icons"),"null.png"),wxBITMAP_TYPE_PNG);
	for(unsigned int i=0;i<perfiles.GetCount();i++) {
		wxString ficon=DIR_PLUS_FILE(DIR_PLUS_FILE(config->profiles_dir,"icons"),perfiles[i]+".png");
		if (wxFileName::FileExists(ficon))
			iml->Add(wxBitmap(ficon,wxBITMAP_TYPE_PNG));
		else
			iml->Add(noimage);
	}
	iml->Add(wxBitmap(DIR_PLUS_FILE(DIR_PLUS_FILE(config->profiles_dir,"icons"),"personalizado.png"),wxBITMAP_TYPE_PNG));
	list->AssignImageList(iml,wxIMAGE_LIST_SMALL);
	text = new wxTextCtrl(this,wxID_ANY,_T(""),wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	
	
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	
	sizer->Add(new wxStaticText(this,wxID_ANY,_Z(" Puede buscar por nombre de la institución, materia, docente, siglas, etc.")),wxSizerFlags().Expand().Proportion(0).Border(wxTOP,5));
	
	wxBoxSizer *search_sizer = new wxBoxSizer(wxHORIZONTAL);
	search=new wxTextCtrl(this,wxID_FIND,"");
	search_sizer->Add(new wxStaticText(this,wxID_ANY,_Z("Buscar: ")),wxSizerFlags().Center());
	search_sizer->Add(search,wxSizerFlags().Proportion(1).Expand());
	sizer->Add(search_sizer,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	wxBoxSizer *sizer_prof = new wxBoxSizer(wxHORIZONTAL);
	sizer_prof->Add(list,wxSizerFlags().Expand().Proportion(2).FixedMinSize());
	sizer_prof->Add(text,wxSizerFlags().Expand().Proportion(3).FixedMinSize());
	sizer->Add(sizer_prof,wxSizerFlags().Expand()/*.Proportion(3)*/.FixedMinSize());
	
	sizer->AddSpacer(10);
	
	
	
//	sizer->Add(new wxStaticText(this,wxID_ANY,_Z(" Seleccione un perfil para configurar las reglas del lenguaje: ")),wxSizerFlags().Expand().Proportion(0).Border(wxTOP,5));
//	sizer->Add(list,wxSizerFlags().Expand().Proportion(3).FixedMinSize());
//	sizer->Add(new wxStaticText(this,wxID_ANY,_Z("")),wxSizerFlags().Expand().Proportion(0));
//	sizer->Add(new wxStaticText(this,wxID_ANY,_Z(" Descripción del perfil seleccionado:")),wxSizerFlags().Expand().Proportion(0));
//	sizer->Add(text,wxSizerFlags().Expand().Proportion(1).FixedMinSize());
//	sizer->Add(new wxStaticText(this,wxID_ANY,""),wxSizerFlags().Expand().Proportion(0));
//	sizer->Add(button_sizer,wxSizerFlags().Expand().Proportion(0));

	sizer->AddSpacer(10);
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *options_button = new mxBitmapButton (this, wxID_ABOUT, bitmaps->buttons.options, _Z("Personalizar..."));
	wxButton *load_button = new mxBitmapButton (this, wxID_OPEN, bitmaps->buttons.load, _Z("Cargar..."));
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, _Z("Aceptar"));
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, _Z("Cancelar"));
	button_sizer->Add(options_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(load_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->AddStretchSpacer(1);
	button_sizer->Add(cancel_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(ok_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	sizer->Add(button_sizer,wxSizerFlags().Expand().Proportion(0));
	
	
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	SetSizerAndFit(sizer);
	
	this->Layout(); // para ajustar el tamaño de la columna de la lista
	list->SetColumnWidth(0,list->GetSize().GetWidth()-32);
	
	search->SetFocus();
	Search();
	UpdateDetails();
	
	_LOG("mxProfile::mxProfile End");
}

void mxProfile::OnClose(wxCloseEvent &evt) {
	wxCommandEvent e;
	OnCancelButton(e);
}

void mxProfile::OnOkButton(wxCommandEvent &evt) {
	config->SetProfile(lang);
	EndModal(1); 
}

void mxProfile::OnCancelButton(wxCommandEvent &evt) {
	EndModal(0);
}
void mxProfile::OnListSelect(wxListEvent &evt) {
	evt.Skip(); wxString sel = GetListSelection();
	if (sel==CUSTOM_PROFILE && lang.source==LS_LIST) lang.source = LS_CUSTOM;
	else lang.Load(DIR_PLUS_FILE(config->profiles_dir,sel),true);
	UpdateDetails();
}

void mxProfile::OnListActivate(wxListEvent &evt) {
	evt.Skip();
	wxCommandEvent ce;
	OnOkButton(ce);
}

void mxProfile::OnOptionsButton(wxCommandEvent &evt) {
	LangSettings custom_lang = lang;
	if (mxConfig(this,custom_lang).ShowModal() && custom_lang!=lang) {
		lang = custom_lang;
		Search();
	}
	list->SetFocus();
}

void mxProfile::UpdateDetails() {
	if (!text) return;
	if (lang.source==LS_FILE) {
		text->SetValue(wxString()
					   <<_Z("Su personalización actual ha sido cargada desde el archivo: ") << _S2W(lang.name)); 
	} else if (lang.source==LS_LIST) {
		int p = perfiles.Index(GetListSelection());
		text->SetValue(p==wxNOT_FOUND 
					     ?  "Error: archivo de perfil no encontrado" 
					     : descripciones[p]);
	} else {
		text->SetValue(_Z("Puede utilizar el botón \"Personalizar\" para definir su propia configuración.")); 
	}
}

wxString mxProfile::GetListSelection ( ) {
	long item = -1;
	item = list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	return list->GetItemText(item);
}

void mxProfile::SetListSelection (int i) {
	list->SetItemState(i,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
	list->EnsureVisible(i);
}

void mxProfile::OnSearchText (wxCommandEvent & evt) {
	Search();
}

static wxString normalize(wxString s) {
	s.MakeLower();
	s.Replace(_Z("á"),_T("a"),true);
	s.Replace(_Z("é"),_T("e"),true);
	s.Replace(_Z("í"),_T("i"),true);
	s.Replace(_Z("ó"),_T("o"),true);
	s.Replace(_Z("ú"),_T("u"),true);
	s.Replace(_Z("ñ"),_T("n"),true);
	s.Replace(_Z("ü"),_T("u"),true);
	return s;
}

void mxProfile::Search ( ) {
	list->DeleteAllItems();
	int sel=-1, cont=0;
	wxString pat=normalize(search->GetValue());
	wxString name = lang.source==LS_LIST ? wxString(_S2W(lang.name)).Lower() : "";
	for(unsigned int i=0;i<perfiles.GetCount();i++) {
		if (pat.Len()==0 || perfiles[i].Lower().Contains(pat) || normalize(descripciones[i]).Contains(pat)) {
			list->InsertItem(cont,perfiles[i],i);
			if (perfiles[i].Lower()==name) sel=cont;
			cont++;
		}
		
	}
	if (!pat) {
		list->InsertItem(cont,CUSTOM_PROFILE,perfiles.GetCount());
		if (sel==-1) sel=cont;
		cont++;
	}
	if (sel!=-1) SetListSelection(sel); 
//	else text->SetValue(wxString()<<_Z("El perfil seleccionado actualmente (")<<lang.name<<_Z(") no aparece en esta búsqueda."));
}

void mxProfile::OnLoadButton (wxCommandEvent & evt) {
	wxString file = mxConfig::LoadFromFile(this);
	if (!file.IsEmpty()) {
		lang.Load(file,false);
		SetListSelection(perfiles.GetCount()); // personalizados
		UpdateDetails();
	}
}

