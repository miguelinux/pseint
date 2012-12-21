#include "mxAboutWindow.h"
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>

#include "ConfigManager.h"
#include "mxBitmapButton.h"
#include "mxUtils.h"
#include "version.h"
#include <wx/html/htmlwin.h>
#include <wx/msgdlg.h>
#include <wx/textfile.h>

BEGIN_EVENT_TABLE(mxAboutWindow, wxDialog)

	EVT_BUTTON(wxID_OK,mxAboutWindow::OnCloseButton)
	EVT_CLOSE(mxAboutWindow::OnClose)

END_EVENT_TABLE()

mxAboutWindow::mxAboutWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, _T("Acerca de..."), pos, size, style) {
		
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);

	wxButton *ok_button = new wxButton (this, wxID_OK, _T("Cerrar"));
	ok_button->SetDefault(); 
	SetEscapeId(wxID_OK);
	
	mySizer->Add(
		//new wxStaticBitmap(this,wxID_ANY, wxBitmap(DIR_PLUS_FILE(config->Files.skin_dir,_T("about.png")), wxBITMAP_TYPE_PNG))
		html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(500,210))
		,wxSizerFlags().Proportion(1).Expand());
	mySizer->Add(ok_button,wxSizerFlags().Expand());
	SetSizerAndFit(mySizer);
	html->SetPage(MakePageText());
	ShowModal();

}

void mxAboutWindow::OnCloseButton(wxCommandEvent &event){
	Destroy();
}

void mxAboutWindow::OnClose(wxCloseEvent &event){
	Destroy();
}

wxString mxAboutWindow::MakePageText() {
	wxString text(_T("<HTML><HEAD><TITLE>PSeInt</TITLE></HEAD><BODY>"));
//	text<<_T("<CENTER><IMG src=\"")<<DIR_PLUS_FILE(config->images_path,_T("about1.png"))<<_T("\"/>");
//	text<<_T("<IMG src=\"")<<DIR_PLUS_FILE(config->images_path,_T("about2.png"))<<_T("\"/></CENTER><HR><BR>");
	text<<_T("<CENTER><TABLE><TR><TD>");
	text<<_T("<IMG src=\"")<<DIR_PLUS_FILE(config->images_path,_T("logo.png"))<<_T("\"/>");
	text<<_T("</TD><TD>");
	
	text<<_T("<CENTER><B>Copyleft 2003-2012<BR>por Pablo Novara<BR>");
	text<<_T("zaskar_84@yahoo.com.ar<BR><BR>");
	text<<_T("Este software es Libre y gratuito.<BR>Se distribuye bajo licencia GPL<BR>(General Public License)");
	text<<_T("<BR><BR>http://pseint.sourceforge.net</B><BR></CENTER>");
	
	text<<_T("</TD></TR></TABLE></CENTER><HR><BR>");
	
	text<<_T("Versión general de la instalación: ");
	wxTextFile fil(_T("version")); 
	if (fil.Exists()) {
		fil.Open();
		text<<fil.GetFirstLine();;
		fil.Close();
	} else {
		text<<"Error: No se pudo determinar.";
	}
	text<<"<BR><BR> Versiones individuales:<BR>";
	
	text<<_T("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;wxPSeInt ")<<VERSION<<_T("-"ARCHITECTURE"<BR>");
	wxArrayString out_int, out_drw, out_exp, out_drw2, out_trm;
	wxString command_int, command_drw, command_exp, command_drw2, command_trm;
	command_int<<config->pseint_command<<_T(" --version");
	command_drw<<config->psdraw_command<<_T(" --version");
	command_drw2<<config->psdraw2_command<<_T(" --version");
	command_exp<<config->psexport_command<<_T(" --version");
	command_trm<<config->psterm_command<<_T(" --version");
	wxExecute(command_int, out_int, wxEXEC_SYNC|wxEXEC_NODISABLE);
	wxExecute(command_drw, out_drw, wxEXEC_SYNC|wxEXEC_NODISABLE);
	wxExecute(command_drw2, out_drw2, wxEXEC_SYNC|wxEXEC_NODISABLE);
	wxExecute(command_exp, out_exp, wxEXEC_SYNC|wxEXEC_NODISABLE);
	wxExecute(command_trm, out_trm, wxEXEC_SYNC|wxEXEC_NODISABLE);
	text<<_T("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")<<(out_int.GetCount()>0?out_int[0]:wxString(_T("Error: No se pudo determinar la version")))<<_T("<BR>");
	text<<_T("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")<<(out_drw.GetCount()>0?out_drw[0]:wxString(_T("Error: No se pudo determinar la version")))<<_T("<BR>");
	text<<_T("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")<<(out_drw2.GetCount()>0?out_drw2[0]:wxString(_T("Error: No se pudo determinar la version")))<<_T("<BR>");
	text<<_T("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")<<(out_exp.GetCount()>0?out_exp[0]:wxString(_T("Error: No se pudo determinar la version")))<<_T("<BR>");
	text<<_T("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")<<(out_trm.GetCount()>0?out_trm[0]:wxString(_T("Error: No se pudo determinar la version")))<<_T("<BR>");
	
	text<<_T("<BR><BR>");
	return text;
}
