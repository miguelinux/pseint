#include "mxProfile.h"
#include <wx/sizer.h>
#include "ConfigManager.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include <wx/dir.h>
#include "mxConfig.h"
#include <wx/stattext.h>
#include <wx/textfile.h>
#include "mxArt.h"
#include "mxMainWindow.h"
#include "RTSyntaxManager.h"
#include <wx/imaglist.h>

#define _PERSONALIZADO "<personalizado>"
#include "string_conversions.h"
#include "Logger.h"

BEGIN_EVENT_TABLE(mxProfile,wxDialog)
	EVT_TEXT(wxID_FIND,mxProfile::OnSearchText)
	EVT_LIST_ITEM_SELECTED(wxID_ANY,mxProfile::OnListSelect)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY,mxProfile::OnListActivate)
	EVT_BUTTON(wxID_ABOUT,mxProfile::OnOptionsButton)
	EVT_BUTTON(wxID_OK,mxProfile::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxProfile::OnCancelButton)
	EVT_CLOSE(mxProfile::OnClose)
END_EVENT_TABLE()
	
static int comp_nocase(const wxString& first, const wxString& second) {
	return first.CmpNoCase(second);
}

mxProfile::mxProfile(wxWindow *parent):wxDialog(parent,wxID_ANY,_Z("Opciones del Lenguaje"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),old_config(LS_INIT) {

	_LOG("mxProfile::mxProfile Start");
	
	text=NULL; // para que no procese el evento de seleccion al crear la lista
	
	old_config=config->lang;
	old_profile=config->profile;
	
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer *search_sizer = new wxBoxSizer(wxHORIZONTAL);
	search=new wxTextCtrl(this,wxID_FIND,"");
	search_sizer->Add(new wxStaticText(this,wxID_ANY,_Z("Buscar: ")),wxSizerFlags().Center());
	search_sizer->Add(search,wxSizerFlags().Proportion(1).Expand());
	
	sizer->Add(search_sizer,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);

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
		LangSettings l(LS_INIT); l.Load(DIR_PLUS_FILE(config->profiles_dir,perfiles[i]));
		descripciones.Add(l.descripcion.c_str());
	}
	
	list = new wxListCtrl(this,wxID_ANY,wxDefaultPosition,wxSize(200,200),wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
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
	
	Search();
	
	text = new wxTextCtrl(this,wxID_ANY,_T(""),wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	LoadProfile();
	
	wxButton *options_button = new mxBitmapButton (this, wxID_ABOUT, bitmaps->buttons.next, _Z("Personalizar..."));
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, _Z("Aceptar"));
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, _Z("Cancelar"));
	button_sizer->Add(options_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->AddStretchSpacer(1);
	button_sizer->Add(cancel_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(ok_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	
	sizer->Add(new wxStaticText(this,wxID_ANY,_Z(" Seleccione un perfil para configurar las reglas del lenguaje: ")),wxSizerFlags().Expand().Proportion(0).Border(wxTOP,5));
	sizer->Add(list,wxSizerFlags().Expand().Proportion(3).FixedMinSize());
	sizer->Add(new wxStaticText(this,wxID_ANY,_Z("")),wxSizerFlags().Expand().Proportion(0));
	sizer->Add(new wxStaticText(this,wxID_ANY,_Z(" Descripción del perfil seleccionado:")),wxSizerFlags().Expand().Proportion(0));
	sizer->Add(text,wxSizerFlags().Expand().Proportion(1).FixedMinSize());
	sizer->Add(new wxStaticText(this,wxID_ANY,""),wxSizerFlags().Expand().Proportion(0));
	sizer->Add(button_sizer,wxSizerFlags().Expand().Proportion(0));
	
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	SetSizerAndFit(sizer);
	
	this->Layout(); // para ajustar el tamaño de la columna de la lista
	list->SetColumnWidth(0,list->GetSize().GetWidth()-32);
	
	search->SetFocus();
	
	_LOG("mxProfile::mxProfile ShowModal");
	
	ShowModal();
	
	_LOG("mxProfile::mxProfile End");
}

mxProfile::~mxProfile() {
}

void mxProfile::OnClose(wxCloseEvent &evt) {
	config->lang=old_config;
	config->profile=old_profile;
	Destroy();
}

void mxProfile::OnOkButton(wxCommandEvent &evt) {
	old_config=config->lang; // para que el evento on_close no lo deshaga
	old_profile=config->profile; // para que el evento on_close no lo deshaga
	main_window->ProfileChanged();
	Close(); 
}

void mxProfile::OnCancelButton(wxCommandEvent &evt) {
	Close(); // el evento OnClose vuelve al perfil anterior
}
void mxProfile::OnListSelect(wxListEvent &evt) {
	evt.Skip();
	LoadProfile();
}

void mxProfile::OnListActivate(wxListEvent &evt) {
	evt.Skip();
	wxCommandEvent ce;
	OnOkButton(ce);
}

void mxProfile::OnOptionsButton(wxCommandEvent &evt) {
	LangSettings old=config->lang;
	new mxConfig(this);
	if (config->lang!=old) {
		config->profile=_PERSONALIZADO;
		Search();
	}
	list->SetFocus();
}

void mxProfile::LoadProfile() {
	if (!text) return;
	wxString pname=GetListSelection();
	if (pname==_PERSONALIZADO) { 
		text->SetValue(_Z("Puede utilizar el botón \"Personalizar\" para definir su propia configuración.")); 
		return;
	}
	int p=perfiles.Index(pname);
	if (p==wxNOT_FOUND) return; // no deberia ocurrir nunca
	config->LoadProfile(pname);
	text->SetValue(descripciones[p]);
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

void mxProfile::Search ( ) {
	list->DeleteAllItems();
	int sel=-1, cont=0;
	wxString pat=search->GetValue().Lower();
	for(unsigned int i=0;i<perfiles.GetCount();i++) {
		if (pat.Len()==0 || perfiles[i].Lower().Contains(pat) || descripciones[i].Lower().Contains(pat)) {
			list->InsertItem(cont,perfiles[i],i);
			if (perfiles[i].Lower()==config->profile.Lower()) sel=cont;
			cont++;
		}
		
	}
	if (!pat) {
		list->InsertItem(cont,_PERSONALIZADO,perfiles.GetCount());
		if (sel==-1) sel=cont;
		cont++;
	}
	if (sel!=-1) SetListSelection(sel); 
	else text->SetValue(wxString()<<_Z("El perfil seleccionado actualmente (")<<config->profile<<_Z(") no aparece en esta búsqueda."));
}

