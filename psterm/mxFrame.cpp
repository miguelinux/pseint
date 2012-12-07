#include "mxFrame.h"
#include <wx/sizer.h>
#include "mxConsole.h"
#include <wx/button.h>

enum { FRAME_ID_BASE=wxID_HIGHEST+1000, FRAME_ID_RELOAD };

BEGIN_EVENT_TABLE(mxFrame,wxFrame)
	EVT_BUTTON(FRAME_ID_RELOAD,mxFrame::OnButtonReload)
END_EVENT_TABLE()

mxFrame::mxFrame(wxString command):wxFrame(NULL,wxID_ANY,"console test",wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	console=new mxConsole(this);
	sizer->Add(console,wxSizerFlags().Proportion(1).Expand());
	wxBoxSizer *sizer_but = new wxBoxSizer(wxHORIZONTAL);
	sizer_but->Add(new wxButton(this,FRAME_ID_RELOAD,"Reload"));
	sizer->Add(sizer_but,wxSizerFlags().Proportion(0).Expand());
	SetSizer(sizer);
	Show();
	console->Run(command);
	console->SetFocus();
}

void mxFrame::OnButtonReload (wxCommandEvent & evt) {
	console->Reload();
	console->SetFocus();
}

