#include <wx/dcclient.h>
#include <wx/clrpicker.h>
#include <wx/timer.h>
#include <wx/dcmemory.h>

#include "mxSplashScreen.h"
#include "ConfigManager.h"
#include "mxUtils.h"

BEGIN_EVENT_TABLE(mxSplashScreen, wxFrame)
	EVT_PAINT(mxSplashScreen::OnPaint)
	EVT_TIMER(wxID_ANY,mxSplashScreen::OnTimer)
	EVT_LEFT_DOWN(mxSplashScreen::OnMouse)
	EVT_CLOSE(mxSplashScreen::OnClose)
END_EVENT_TABLE()

mxSplashScreen::mxSplashScreen() : 
	wxFrame(nullptr,wxID_ANY,_T("Cargando PSeInt..."),wxDefaultPosition,wxDefaultSize, wxNO_BORDER | wxSTAY_ON_TOP | wxFRAME_NO_TASKBAR),
	m_timer(this->GetEventHandler(),wxID_ANY)
{
	m_bmp = wxBitmap(DIR_PLUS_FILE(config->images_path, (config->use_dark_theme 
														 ? "splash_dark.png" 
														 : "splash_light.png") ),wxBITMAP_TYPE_PNG);
	SetSize(m_bmp.GetWidth(),m_bmp.GetHeight());
	Move(config->pos_x+(config->size_x-m_bmp.GetWidth() )/2,
		 config->pos_y +(config->size_y-m_bmp.GetHeight())/2 );
	
	
	Show(true);
	Update(); // Without this, you see a blank screen for an instant (from wxSplashScreen, for WIN32)
	wxYield();
	
	m_timer.Start(3000,true);
}

void mxSplashScreen::OnPaint(wxPaintEvent& WXUNUSED(evt)) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.DrawBitmap(m_bmp, 0, 0, true);
}

void mxSplashScreen::OnTimer(wxTimerEvent &evt) {
	Close();
}

void mxSplashScreen::OnMouse(wxMouseEvent &evt) {
	Hide();
}

void mxSplashScreen::OnClose(wxCloseEvent &evt) {
	m_timer.Stop();
	Destroy();
}
