#include <iostream>
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include "../wxPSeInt/string_conversions.h"
#include "MainWindow.h"
#include "ToolBar.h"
#include "Canvas.h"
#include "Global.h"
#include "Load.h"
#include "Events.h"
#include "Comm.h"

MainWindow *main_window=NULL;

enum { MID_MAIN = wxID_HIGHEST, MID_ZOOM, MID_FULLSCREEN, MID_STYLE, MID_COMMENTS, MID_CROPLABEL, MID_COLORS, MID_SUB, MID_RUN, MID_DEBUG, MID_EXPORT, MID_CLOSE, MID_HELP };

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
//	EVT_CHAR(Canvas::OnChar)
#if WIN32
	EVT_MOUSEWHEEL(MainWindow::OnMouseWheel)
#endif
	EVT_TOOL_ENTER(wxID_ANY,MainWindow::OnToolOver)
	EVT_TOOL(wxID_ANY,MainWindow::OnTool)
	EVT_CLOSE(MainWindow::OnClose)
	EVT_ACTIVATE(MainWindow::OnActivated)
END_EVENT_TABLE()

MainWindow::MainWindow(wxString title):wxFrame(NULL,wxID_ANY,title,wxDefaultPosition,wxSize(win_w,win_h),wxDEFAULT_FRAME_STYLE) {
	main_window=this;
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	
	wxToolBar *toolbar = CreateToolBar(wxTB_HORIZONTAL|wxNO_BORDER|wxTB_FLAT,wxID_ANY);
	color_menu_back[0] = toolbar->GetBackgroundColour().Red()/255.f;
	color_menu_back[1] = toolbar->GetBackgroundColour().Green()/255.f;
	color_menu_back[2] = toolbar->GetBackgroundColour().Blue()/255.f;
	wxColour color_sel = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	color_menu_sel[0] = color_sel.Red()/255.f;
	color_menu_sel[1] = color_sel.Green()/255.f;
	color_menu_sel[2] = color_sel.Blue()/255.f;
	wxBitmap abmp(imgs_path+"tb_sub.png",wxBITMAP_TYPE_PNG);
	toolbar->SetToolBitmapSize(wxSize(abmp.GetWidth(),abmp.GetHeight()));
//	toolbar->AddTool(MID_SETTINGS,"",imgs_path+"tb_settings.png");
	if (lang[LS_ENABLE_USER_FUNCTIONS]) {
		toolbar->AddTool(MID_SUB  ,"",wxBitmap(imgs_path+"tb_sub.png",wxBITMAP_TYPE_PNG));
		toolbar->AddSeparator();
	}
	toolbar->AddTool(MID_RUN  ,"",wxBitmap(imgs_path+"tb_run.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_DEBUG,"",wxBitmap(imgs_path+"tb_debug.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_EXPORT ,"",wxBitmap(imgs_path+"tb_save.png",wxBITMAP_TYPE_PNG));
	toolbar->AddSeparator();
	toolbar->AddTool(MID_ZOOM ,"",wxBitmap(imgs_path+"tb_zoom.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_FULLSCREEN ,"",wxBitmap(imgs_path+"tb_fullscreen.png",wxBITMAP_TYPE_PNG));
	toolbar->AddSeparator();
	toolbar->AddTool(MID_STYLE ,"",wxBitmap(imgs_path+"tb_style.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_COMMENTS ,"",wxBitmap(imgs_path+"tb_comments.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_CROPLABEL ,"",wxBitmap(imgs_path+"tb_crop.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_COLORS,"",wxBitmap(imgs_path+"tb_colors.png",wxBITMAP_TYPE_PNG));
	toolbar->AddSeparator();
	toolbar->AddTool(MID_HELP ,"",wxBitmap(imgs_path+"tb_help.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_CLOSE,"",wxBitmap(imgs_path+"tb_close.png",wxBITMAP_TYPE_PNG));
	toolbar->Realize();
	sizer->Add(new Canvas(this),wxSizerFlags().Expand().Proportion(1));
	SetSizer(sizer);
	canvas->SetFocusFromKbd();
//	SetAccelerators();
	Show();
}

void MainWindow::OnClose (wxCloseEvent & evt) {
	if (modified) AskForExit(); else evt.Skip();
}

//void MainWindow::SetAccelerators() {
//	wxAcceleratorEntry entries[15];
//	entries[0].Set(wxACCEL_CTRL, 'V', AID_PASTE);
//	wxAcceleratorTable accel(14, entries);
//	SetAcceleratorTable(accel);
//}

//void MainWindow::OnChar (wxKeyEvent & event) {
////	canvas->OnChar(event);
//}

void MainWindow::OnMouseWheel (wxMouseEvent & event) {
	canvas->OnMouseWheel(event);
}

void MainWindow::ToggleFullScreen ( ) {
	bool fs = !main_window->IsFullScreen();
	main_window->ShowFullScreen(fs);
}

void MainWindow::AskForExit ( ) {
	int ans=wxMessageBox(_Z("Hay cambios sin aplicar al pseudocódigo.\n¿Aplicar los cambios antes de cerrar el editor?"),main_window->GetTitle(),wxYES_NO|wxCANCEL|wxICON_QUESTION,this);
	if (ans==wxYES) {
		Save(); SendUpdate(MO_SAVE); Salir(true);
	} else if (ans==wxNO) {
		Salir(true);
	} else {
		return;
	}
}

void MainWindow::OnToolOver (wxCommandEvent & event) {
	toolbar->SetSelection( MIDtoMO(event.GetSelection()) ); 
	canvas->Refresh();
}

void MainWindow::OnTool (wxCommandEvent & event) {
	ProcessMenu(MIDtoMO(event.GetId()));
}

int MainWindow::MIDtoMO (int mid) {
	switch(mid) {
	case MID_ZOOM: return MO_ZOOM_EXTEND;
	case MID_FULLSCREEN: return MO_TOGGLE_FULLSCREEN;
	case MID_CROPLABEL: return MO_CROP_LABELS;
	case MID_COMMENTS: return MO_TOGGLE_COMMENTS;
	case MID_STYLE: return MO_CHANGE_STYLE;
	case MID_COLORS: return MO_TOGGLE_COLORS;
	case MID_SUB: return MO_FUNCTIONS;
	case MID_RUN: return MO_RUN;
	case MID_DEBUG: return MO_DEBUG;
	case MID_EXPORT: return MO_EXPORT;
	case MID_CLOSE:return MO_CLOSE;
	case MID_HELP:return MO_HELP;
	default: return MO_NONE;
	}
}

void MainWindow::OnActivated(wxActivateEvent &event) {
	event.Skip();
	canvas->SetModifiers(0); // para que Alt+Tab no deje el Alt como apretado
}

