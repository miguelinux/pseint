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

BEGIN_EVENT_TABLE(mxProfile,wxDialog)
	EVT_LIST_ITEM_SELECTED(wxID_ANY,mxProfile::OnListSelect)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY,mxProfile::OnListActivate)
	EVT_BUTTON(wxID_FIND,mxProfile::OnOptionsButton)
	EVT_BUTTON(wxID_OK,mxProfile::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxProfile::OnCancelButton)
	EVT_CLOSE(mxProfile::OnClose)
END_EVENT_TABLE()

mxProfile::mxProfile(wxWindow *parent):wxDialog(parent,wxID_ANY,_T("Opciones del Lenguaje"),wxDefaultPosition,wxDefaultSize) {
	
	text=NULL; // para que no procese el evento de seleccion al crear la lista
	
	old_config=config->lang;
	
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
//	wxBoxSizer *opts_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	

	wxArrayString perfiles;
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
	perfiles.Sort();
	int profnum=-1;
	profnum = perfiles.Index(config->profile,false);
//	perfiles.Add(_T("<personalizado>"));
	
	if (profnum==wxNOT_FOUND) profnum=perfiles.GetCount()-1;
	
	list = new wxListCtrl(this,wxID_ANY,wxDefaultPosition,wxSize(250,250),wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
	list->InsertColumn(0,"Perfil");
	wxImageList *iml = new wxImageList(24,24,true);
	wxBitmap noimage(DIR_PLUS_FILE(DIR_PLUS_FILE("perfiles","icons"),"null.png"),wxBITMAP_TYPE_PNG);
	for(unsigned int i=0;i<perfiles.GetCount();i++) {
		wxString ficon=DIR_PLUS_FILE(DIR_PLUS_FILE("perfiles","icons"),perfiles[i]+".png");
		if (wxFileName::FileExists(ficon))
			iml->Add(wxBitmap(ficon,wxBITMAP_TYPE_PNG));
		else
			iml->Add(noimage);
	}
	iml->Add(wxBitmap(DIR_PLUS_FILE(DIR_PLUS_FILE("perfiles","icons"),"personalizado.png"),wxBITMAP_TYPE_PNG));
	list->AssignImageList(iml,wxIMAGE_LIST_SMALL);
	for(unsigned int i=0;i<perfiles.GetCount();i++) list->InsertItem(i,perfiles[i],i);
	list->InsertItem(perfiles.GetCount(),"<personalizado>",perfiles.GetCount());
	
	SetListSelection(profnum);
	text = new wxTextCtrl(this,wxID_ANY,_T(""),wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	LoadProfile();
	
	wxButton *options_button = new mxBitmapButton (this, wxID_FIND, bitmaps->buttons.next, _T("Personalizar..."));
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, _T("Aceptar"));
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, _T("Cancelar"));
	button_sizer->Add(options_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->AddStretchSpacer(1);
	button_sizer->Add(cancel_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(ok_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	
	sizer->Add(new wxStaticText(this,wxID_ANY,_T(" Seleccione un perfil para configurar las reglas de su pseudocodigo:  ")),wxSizerFlags().Expand().Proportion(0).Border(wxTOP,5));
	sizer->Add(list,wxSizerFlags().Expand().Proportion(2).FixedMinSize());
	sizer->Add(new wxStaticText(this,wxID_ANY,_T("")),wxSizerFlags().Expand().Proportion(0));
	sizer->Add(new wxStaticText(this,wxID_ANY,_T(" Descripcion del perfil seleccionado:")),wxSizerFlags().Expand().Proportion(0));
	sizer->Add(text,wxSizerFlags().Expand().Proportion(1).FixedMinSize());
	sizer->Add(new wxStaticText(this,wxID_ANY,_T("")),wxSizerFlags().Expand().Proportion(0));
	sizer->Add(button_sizer,wxSizerFlags().Expand().Proportion(0));
	
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	SetSizerAndFit(sizer);
	
	this->Layout(); // para ajustar el tamaño de la columna de la lista
	list->SetColumnWidth(0,list->GetSize().GetWidth()-32);
	
	list->SetFocus();
	
	ShowModal();
}

mxProfile::~mxProfile() {
}

void mxProfile::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxProfile::OnOkButton(wxCommandEvent &evt) {
	main_window->ProfileChanged();
	Close(); 
}

void mxProfile::OnCancelButton(wxCommandEvent &evt) {
	config->lang=old_config;
	Close();
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
		SetListSelection(list->GetItemCount()-1);
		LoadProfile();
	}
	list->SetFocus();
}

void mxProfile::LoadProfile() {
#warning NO FUNCIONA EN WINDOWS
	if (text) text->SetValue(config->LoadProfile(GetListSelection()));
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

