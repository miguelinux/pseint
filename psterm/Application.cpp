#include "Application.h"
#include "mxFrame.h"
#include <wx/msgdlg.h>

IMPLEMENT_APP(mxApplication)
	
static wxString EscapeString(wxString what) {
	for(int i=0;i<what.Len();i++)
		if (what[i]=='\''||what[i]=='\"'||what[i]==' '||what[i]=='\\') { what=what.Mid(0,i)+"\\"+what.Mid(i); i++; }
	return what;
}

bool mxApplication::OnInit() {
	bool no_arg=false;
	long port=-1, src_id=-1;
	wxString command;
	for(int i=1;i<argc;i++) {
		if (no_arg) {
			if (command.Len()) command<<" ";
			command<<EscapeString(argv[i]);
		} else {
			wxString arg(argv[i]);
			if (arg.StartsWith("--")) {
				if (arg=="--") {
					no_arg=true;
				} else if (arg.StartsWith("--id=")) {
					arg.AfterFirst('=').ToLong(&src_id);
				} else if (arg.StartsWith("--port=")) {
					arg.AfterFirst('=').ToLong(&port);
				}
			} else {
				command=EscapeString(argv[i]);
				no_arg=true;
			}
		}
	}
	new mxFrame(command,port,src_id);
	return true;
}
