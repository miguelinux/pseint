#include "Application.h"
#include "mxFrame.h"

IMPLEMENT_APP(mxApplication)

bool mxApplication::OnInit() {
	new mxFrame;
	return true;
}


