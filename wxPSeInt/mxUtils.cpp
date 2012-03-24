#include "mxUtils.h"
#include "ConfigManager.h"
#include <wx/filename.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/sizer.h>
#include "mxBitmapButton.h"

mxUtils *utils;

mxUtils::mxUtils() {
	
}

mxUtils::~mxUtils() {
	
}

wxMenuItem *mxUtils::AddItemToMenu(wxMenu *menu, wxWindowID id,wxString caption, wxString help, wxString filename) {
	wxMenuItem *item = new wxMenuItem(menu,id,caption,help);
	filename=JoinDirAndFile(JoinDirAndFile(config->images_path,_T("menu")),filename);
	if (wxFileName::FileExists(filename))
		item->SetBitmap(wxBitmap(filename,wxBITMAP_TYPE_PNG));
	menu->Append (item);
	return item;
}

wxString mxUtils::JoinDirAndFile(wxString dir, wxString fil) {
	if (dir.Len()==0 || (fil.Len()>1 && (fil[0]=='\\' || fil[0]=='/' || fil[1]==':')))
		return fil;
	else if (dir.Last()==wxFileName::GetPathSeparator())
		return dir+fil;
	else
		return dir+wxFileName::GetPathSeparator()+fil;
}

void mxUtils::AddTool(wxToolBar *toolbar, wxWindowID id, wxString caption, wxString filename, wxString status_text) {
	filename=JoinDirAndFile(JoinDirAndFile(config->images_path,_T("tools")),filename);
	if (wxFileName::FileExists(filename)) {
		toolbar->AddTool(id, caption, wxBitmap(filename,wxBITMAP_TYPE_PNG),caption);
		toolbar->SetToolLongHelp(id,status_text);
	}
}

void mxUtils::AddImgButton(wxSizer *sizer, wxWindow *parent, wxWindowID id, wxString file, wxString text) {
	wxString cfile = DIR_PLUS_FILE(config->images_path,file);
	if (wxFileName::FileExists(cfile))
		sizer->Add(new mxBitmapButton(parent,id,new wxBitmap(cfile,wxBITMAP_TYPE_PNG),text,wxDefaultSize),wxSizerFlags().Proportion(1).Expand());
	else	
		sizer->Add(new wxButton(parent,id,text),wxSizerFlags().Proportion(1).Expand());
}

wxCheckBox *mxUtils::AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value) {
	wxCheckBox *checkbox;
	sizer->Add(checkbox = new wxCheckBox(panel, wxID_ANY, text+_T("   ")),wxSizerFlags().Border(wxALL, 5).Expand());
	checkbox->SetValue(value);
	return checkbox;	
}

bool mxUtils::IsTrue(wxString value) const {
	return (value[0]=='1' || value[0]=='V' || value[0]=='v' || value[0]=='s' || value[0]=='S' || value[0]=='T' || value[0]=='t');
}

wxMenuItem *mxUtils::AddCheckToMenu(wxMenu *menu, wxWindowID id,wxString caption, wxString help, bool value) {
	wxMenuItem *item = menu->AppendCheckItem (id, caption,help);
	item->Check(value);
	return item;
}

wxString mxUtils::GetOutput(wxString command) {
	wxString ret;
	wxArrayString output;
	wxExecute(command, output, wxEXEC_NODISABLE|wxEXEC_SYNC);
	for (unsigned int i=0;i<output.GetCount();i++)
		if (ret.Len()==0)
			ret<<output[i];
		else
			ret<<_T("\n")<<output[i];
	return ret;
}

/** 
* Devuelve una cadena convertida a HTML. Reemplaza los espacios y operadores por
* los codigos de escape adecuados (\&...).
* @param text La cadena a convertir
* @retval La cadena en formato HTML
**/
wxString mxUtils::ToHtml(wxString text) {
	wxString tabs=_T("&nbsp;&nbsp;&nbsp;");
	text.Replace(_T("&"),_T("&amp;"));
	text.Replace(_T("\t"),tabs);
	text.Replace(_T("<"),_T("&lt;"));
	text.Replace(_T(">"),_T("&gt;"));
	text.Replace(_T("\n"),_T("<BR>"));
	wxChar doce[]=_T(" ");
	doce[0]=12;
	text.Replace(doce,_T("<BR>"));
	text.Replace(_T(" "),_T("&nbsp;"));
	return text;
}
