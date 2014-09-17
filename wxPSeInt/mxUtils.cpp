#include "mxUtils.h"
#include "ConfigManager.h"
#include <wx/filename.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/sizer.h>
#include "mxBitmapButton.h"
#include <wx/textfile.h>
#include "string_conversions.h"

mxUtils *utils;

wxMenuItem *mxUtils::AddItemToMenu(wxMenu *menu, wxWindowID id,wxString caption, wxString help, wxString filename) {
	wxMenuItem *item = new wxMenuItem(menu,id,caption,help);
	filename=JoinDirAndFile(JoinDirAndFile(config->images_path,"menu"),filename);
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
	filename=JoinDirAndFile(JoinDirAndFile(config->images_path,"tools"),filename);
	if (wxFileName::FileExists(filename)) {
		toolbar->AddTool(id, caption, wxBitmap(filename,wxBITMAP_TYPE_PNG),caption);
		toolbar->SetToolLongHelp(id,status_text);
	}
}

wxButton *mxUtils::AddImgButton(wxSizer *sizer, wxWindow *parent, wxWindowID id, wxString file, wxString text) {
	wxString cfile = DIR_PLUS_FILE(config->images_path,file);
	wxButton *btn;
	if (wxFileName::FileExists(cfile))
		sizer->Add(btn=new mxBitmapButton(parent,id,new wxBitmap(cfile,wxBITMAP_TYPE_PNG),text,wxDefaultSize),wxSizerFlags().Proportion(1).Expand());
	else	
		sizer->Add(btn=new wxButton(parent,id,text),wxSizerFlags().Proportion(1).Expand());
	return btn;
}

wxCheckBox *mxUtils::AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value) {
	wxCheckBox *checkbox;
	sizer->Add(checkbox = new wxCheckBox(panel, wxID_ANY, text+"   "),wxSizerFlags().Border(wxALL, 5).Expand());
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
			ret<<"\n"<<output[i];
	return ret;
}

/** 
* Devuelve una cadena convertida a HTML. Reemplaza los espacios y operadores por
* los codigos de escape adecuados (\&...).
* @param text La cadena a convertir
* @retval La cadena en formato HTML
**/
wxString mxUtils::ToHtml(wxString text) {
	wxString tabs="&nbsp;&nbsp;&nbsp;";
	text.Replace("&","&amp;");
	text.Replace("\t",tabs);
	text.Replace("<","&lt;");
	text.Replace(">","&gt;");
	text.Replace("\n","<BR>");
	wxString doce=" "; doce[0]=12;
	text.Replace(doce,"<BR>");
	text.Replace(" ","&nbsp;");
	return text;
}

wxString mxUtils::FixTooltip (wxString tooltip) {
#ifdef __WIN32__
	int p=0, l=tooltip.Len(),c=0;
	while (p<l) {
		if (c>50 && tooltip[p]==' ') {
			tooltip[p]='\n'; c=0;
		}
		c++; p++;
	}
#endif
	return tooltip;
}

wxString mxUtils::GetVersion(wxString exe) {
	wxArrayString out; // para que no muestre la consola (ver ayuda de wxExecute)
	wxString filename=DIR_PLUS_FILE(config->temp_dir,"version.tmp"), retval=exe+_Z(": error desconocido");
	if (wxFileName::FileExists(filename)) 
		wxRemoveFile(filename);
	if (wxFileName::FileExists(filename)) {
		retval=exe+_Z(": error 1: No se pudo determinar la versión.");
	} else {
		wxExecute(exe+" --version \""+filename+"\"",out,wxEXEC_SYNC|wxEXEC_NODISABLE);
		wxTextFile fil(filename);
		if (!fil.Exists()) {
			retval=exe+_Z(": error 2: No se pudo determinar la versión.");
		} else {
			fil.Open();
			if (!fil.GetLineCount()) {
				retval=exe+_Z(": error 3: No se pudo determinar la versión.");
			} else {
				retval=fil.GetFirstLine();
				fil.Close();
				if (!retval.Len()) retval=exe+_Z(": error 4: No se pudo determinar la versión");
			}
		}
	}
	return retval;
}
