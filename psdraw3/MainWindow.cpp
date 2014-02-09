#include "Canvas.h"
#include "MainWindow.h"
#include <wx/sizer.h>
#include "Global.h"
#include <wx/msgdlg.h>
#include "Load.h"
#include "Events.h"
#include "Comm.h"

MainWindow *main_window=NULL;

enum { MID_MAIN = wxID_HIGHEST };

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
//	EVT_CHAR(Canvas::OnChar)
#if WIN32
	EVT_MOUSEWHEEL(MainWindow::OnMouseWheel)
#endif
	EVT_CLOSE(MainWindow::OnClose)
END_EVENT_TABLE()

MainWindow::MainWindow(wxString title):wxFrame(NULL,wxID_ANY,title,wxDefaultPosition,wxSize(win_w,win_h),wxDEFAULT_FRAME_STYLE) {
	main_window=this;
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
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
	main_window->ShowFullScreen(!main_window->IsFullScreen());
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

