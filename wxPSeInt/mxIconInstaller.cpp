#if !defined(__WIN32__) && !defined(__APPLE__)

#include "mxIconInstaller.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include "ConfigManager.h"
#include <wx/textfile.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include "mxArt.h"

BEGIN_EVENT_TABLE(mxIconInstaller,wxDialog)
	EVT_BUTTON(wxID_OK,mxIconInstaller::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxIconInstaller::OnButtonCancel)
	EVT_CLOSE(mxIconInstaller::OnClose)
END_EVENT_TABLE()

mxIconInstaller::mxIconInstaller(bool first_run):wxDialog(NULL,wxID_ANY,_T("Iconos lanzadores")) {
	
	xdg_not_found=icon_installed=false;
	
	wxString res = utils->GetOutput("xdg-desktop-menu --version");
	if (!res.Len()||res.Find("bash")!=wxNOT_FOUND) { 
		if (!first_run)
			wxMessageBox(_T("Debe instalar xdg-utils para tener acceso todas las funcionalidades"),_T("Iconos lanzadores"));
		xdg_not_found=true;
//		return;
	}
	
	wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	
	if (first_run) sizer->Add(new wxStaticText(this,wxID_ANY,_T("¿Desea crear un icono para acceder a PSeInt desde el menu del sistema o el escritorio?")),wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	
	desktop = utils->AddCheckBox(sizer,this,_T("Crear un icono en el escritorio"),true);
	menu = utils->AddCheckBox(sizer,this,_T("Crear un icono en el menu (en la categoria Programacion/Desarrollo)"),!xdg_not_found);
	if (xdg_not_found) menu->Enable(false);
//	psc = utils->AddCheckBox(sizer,this,"Asociar los archivos pseudocodigo (.psc)",true);
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	mxBitmapButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, _T("&Cancelar")); 
	mxBitmapButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, _T("&Aceptar"));
	ok_button->SetMinSize(wxSize(ok_button->GetSize().GetWidth()<80?80:ok_button->GetSize().GetWidth(),ok_button->GetSize().GetHeight()));
	ok_button->SetDefault(); 
	bottomSizer->Add(cancel_button,wxSizerFlags().Border(wxALL,5));
	bottomSizer->Add(ok_button,wxSizerFlags().Border(wxALL,5));
	
	sizer->Add(bottomSizer,wxSizerFlags().Border(wxALL,5).Right());
	
	SetSizerAndFit(sizer);
	
	ShowModal(); desktop->SetFocus();
	
}

void mxIconInstaller::OnButtonOk (wxCommandEvent & evt) {
	if (desktop->GetValue()) InstallDesktop(false);
	if (menu->GetValue()) InstallDesktop(true);
//	if (psc->GetValue()) InstallMime();
	Close();
}

void mxIconInstaller::OnButtonCancel (wxCommandEvent & evt) {
	Close();
}

void mxIconInstaller::OnClose (wxCloseEvent & evt) {
	Destroy();
}

void mxIconInstaller::InstallIcons ( ) {
	if (icon_installed) return;
	wxString icon_file=DIR_PLUS_FILE(config->temp_dir,"pseint.png");
	
	wxCopyFile(DIR_PLUS_FILE(config->pseint_dir,DIR_PLUS_FILE("imgs","icon128.png")),icon_file,true);
	wxExecute(wxString("xdg-icon-resource install --novendor --size ")<<128<<" \""<<icon_file<<"\"",wxEXEC_NODISABLE|wxEXEC_SYNC);
	
	wxCopyFile(DIR_PLUS_FILE(config->pseint_dir,DIR_PLUS_FILE("imgs","icon64.png")),icon_file,true);
	wxExecute(wxString("xdg-icon-resource install --novendor --size ")<<64<<" \""<<icon_file<<"\"",wxEXEC_NODISABLE|wxEXEC_SYNC);
	
	wxCopyFile(DIR_PLUS_FILE(config->pseint_dir,DIR_PLUS_FILE("imgs","icon32.png")),icon_file,true);
	wxExecute(wxString("xdg-icon-resource install --novendor --size ")<<32<<" \""<<icon_file<<"\"",wxEXEC_NODISABLE|wxEXEC_SYNC);
	
	icon_installed=true;
}

void mxIconInstaller::InstallDesktop ( bool menu ) {
	if (xdg_not_found) { MakeDesktopIcon(); return; }
	InstallIcons();
	wxString desk_file=DIR_PLUS_FILE(config->temp_dir,_T("pseint.desktop"));
	wxTextFile fil(desk_file);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	fil.AddLine(_T("[Desktop Entry]"));
	fil.AddLine(_T("Comment=Editor e intérprete de pseudocódigo"));
	fil.AddLine(_T("Encoding=UTF-8"));
	fil.AddLine(_T("Name=PSeInt"));
	fil.AddLine(_T("Type=Application"));
	fil.AddLine(_T("Categories=Development"));
	fil.AddLine(_T("Icon=pseint"));
	fil.AddLine(wxString(_T("Exec="))<<DIR_PLUS_FILE(config->pseint_dir,_T("wxPSeInt")));
	fil.Write();
	fil.Close();
	if (menu)
		wxExecute(wxString("xdg-desktop-menu install --novendor \"")<<desk_file<<"\"",wxEXEC_NODISABLE|wxEXEC_SYNC);
	else
		wxExecute(wxString("xdg-desktop-icon install --novendor \"")<<desk_file<<"\"",wxEXEC_NODISABLE|wxEXEC_SYNC);
}

void mxIconInstaller::InstallMime() { // todavia no se usa
	wxString mime_type="text/pseint-psc";
	wxString mime_desc="pseudocódigo PSeInt";
	wxString icon="pseint-psc.png";
	wxArrayString exts;
	exts.Add("*.psc");
	exts.Add("*.PSC");
	InstallMime(mime_type,mime_desc,icon,exts);
}


void mxIconInstaller::InstallMime ( wxString mime_type, wxString mime_desc, wxString icon, wxArrayString exts ) {
	
	wxString xml_file=DIR_PLUS_FILE(config->temp_dir,_T("pseint-psc.xml"));
	wxTextFile fil(xml_file);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	fil.AddLine("<?xml version=\"1.0\"?>");
	fil.AddLine("<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>");
	fil.AddLine(wxString("<mime-type type=\"")<<mime_type<<"\">");
	fil.AddLine(wxString("<comment>")<<mime_desc<<"</comment>");
	for (unsigned int i=0;i<exts.GetCount();i++)
		fil.AddLine(wxString("<glob pattern=\"")<<exts[i]<<"\"/>");
	fil.AddLine("</mime-type>");
	fil.AddLine("</mime-info>");
	fil.Write();
	fil.Close();

	wxExecute(wxString("xdg-mime install \"")<<xml_file<<"\"",wxEXEC_NODISABLE|wxEXEC_SYNC);
	wxString icon_file;
	const char sizes[][5]={"32","64","128","256"};
	for (int i=0;i<4;i++) {
		icon_file=DIR_PLUS_FILE(config->pseint_dir,DIR_PLUS_FILE("imgs",DIR_PLUS_FILE("icons",DIR_PLUS_FILE(sizes[i],icon))));
		wxExecute(wxString("xdg-icon-resource install --context mimetypes --size ")<<sizes[i]<<" \""<<icon_file<<"\" "<<mime_type,wxEXEC_NODISABLE|wxEXEC_SYNC);
	}
	wxExecute(wxString("xdg-mime default pseint.desktop ")<<mime_type,wxEXEC_NODISABLE|wxEXEC_SYNC);
}

void mxIconInstaller::MakeDesktopIcon() { // si no hay xdg, crear el .desktop a pata
		
	wxString desk_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Desktop"));
	if (!wxFileName::DirExists(desk_dir) && wxFileName::DirExists(DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Escritorio"))))
		desk_dir=DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Escritorio"));
	
	wxTextFile fil(DIR_PLUS_FILE(desk_dir,_T("PSeInt.desktop")));
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	fil.AddLine(_T("[Desktop Entry]"));
	fil.AddLine(_T("Comment=Editor e intérprete de pseudocódigo"));
	fil.AddLine(_T("Encoding=UTF-8"));
	fil.AddLine(wxString(_T("Icon="))<<DIR_PLUS_FILE(config->pseint_dir,_T("imgs/icon64.png")));
	fil.AddLine(_T("Name=PSeInt"));
	fil.AddLine(_T("Type=Link"));
	fil.AddLine(wxString(_T("URL="))<<DIR_PLUS_FILE(config->pseint_dir,_T("wxPSeInt")));
	fil.Write();
	fil.Close();
	
}



#endif


