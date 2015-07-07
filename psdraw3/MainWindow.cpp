#include "Canvas.h"
#include "MainWindow.h"
#include <wx/sizer.h>
#include "Global.h"
#include <wx/msgdlg.h>
#include "Load.h"
#include "Events.h"
#include "Comm.h"
#include <wx/toolbar.h>
#include <iostream>

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
END_EVENT_TABLE()

MainWindow::MainWindow(wxString title):wxFrame(NULL,wxID_ANY,title,wxDefaultPosition,wxSize(win_w,win_h),wxDEFAULT_FRAME_STYLE) {
	main_window=this;
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	
	toolbar = new wxToolBar(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTB_HORIZONTAL|wxNO_BORDER|wxTB_FLAT);
	color_menu_back[0] = toolbar->GetBackgroundColour().Red()/255.f;
	color_menu_back[1] = toolbar->GetBackgroundColour().Green()/255.f;
	color_menu_back[2] = toolbar->GetBackgroundColour().Blue()/255.f;
	sizer->Add(toolbar,wxSizerFlags().Expand().Proportion(0));
//	toolbar->AddTool(MID_SETTINGS,"","imgs/floaw/tb_settings.png");
	toolbar->AddTool(MID_SUB  ,"",wxBitmap("imgs/flow/tb_sub.png",wxBITMAP_TYPE_PNG));
	toolbar->AddSeparator();
	toolbar->AddTool(MID_RUN  ,"",wxBitmap("imgs/flow/tb_run.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_DEBUG,"",wxBitmap("imgs/flow/tb_debug.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_EXPORT ,"",wxBitmap("imgs/flow/tb_save.png",wxBITMAP_TYPE_PNG));
	toolbar->AddSeparator();
	toolbar->AddTool(MID_ZOOM ,"",wxBitmap("imgs/flow/tb_zoom.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_FULLSCREEN ,"",wxBitmap("imgs/flow/tb_fullscreen.png",wxBITMAP_TYPE_PNG));
	toolbar->AddSeparator();
	toolbar->AddTool(MID_STYLE ,"",wxBitmap("imgs/flow/tb_style.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_COMMENTS ,"",wxBitmap("imgs/flow/tb_comments.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_CROPLABEL ,"",wxBitmap("imgs/flow/tb_crop.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_COLORS,"",wxBitmap("imgs/flow/tb_colors.png",wxBITMAP_TYPE_PNG));
	toolbar->AddSeparator();
	toolbar->AddTool(MID_HELP ,"",wxBitmap("imgs/flow/tb_help.png",wxBITMAP_TYPE_PNG));
	toolbar->AddTool(MID_CLOSE,"",wxBitmap("imgs/flow/tb_close.png",wxBITMAP_TYPE_PNG));
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
//	toolbar->Show(!fs);
	main_window->ShowFullScreen(fs);
}

void MainWindow::AskForExit ( ) {
	int ans=wxMessageBox("Hay cambios sin aplicar al pseudocódigo.\n¿Aplicar los cambios antes de cerrar el editor?",main_window->GetTitle(),wxYES_NO|wxCANCEL|wxICON_QUESTION,this);
	if (ans==wxYES) {
		Save(); SendUpdate(MO_SAVE); Salir(true);
	} else if (ans==wxNO) {
		Salir(true);
	} else {
		return;
	}
}

void MainWindow::OnToolOver (wxCommandEvent & event) {
	menu_sel = MIDtoMO(event.GetSelection()); 
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

