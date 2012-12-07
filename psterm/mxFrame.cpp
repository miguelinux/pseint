#include "mxFrame.h"
#include <wx/sizer.h>
#include "mxConsole.h"

mxFrame::mxFrame():wxFrame(NULL,wxID_ANY,"console test",wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	mxConsole *console=new mxConsole(this);
	sizer->Add(console,wxSizerFlags().Proportion(1).Expand());
	SetSizer(sizer);
	Show();
	console->Run("/mnt/ex/pseint/bin/pseint /home/zaskar/.pseint/temp.psc --color");
//	console->Run("/home/zaskar/.zinjai/sin_titulo.bin");
	console->SetFocus();
}

