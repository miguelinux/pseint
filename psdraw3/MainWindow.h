#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <wx/frame.h>

class Canvas;

class MainWindow : public wxFrame {
public:
	MainWindow(wxString title);
//	void SetAccelerators();
//	void OnChar(wxKeyEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnMouseWheel(wxMouseEvent &event);
	void ToggleFullScreen();
	void AskForExit();
	DECLARE_EVENT_TABLE();
};

extern MainWindow *main_window;
#endif

