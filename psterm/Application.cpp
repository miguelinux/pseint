#include <wx/msgdlg.h>
#include <iostream>
using namespace std;
#include "Application.h"
#include "mxFrame.h"
#include "version.h"

IMPLEMENT_APP(mxApplication)
	
static wxString EscapeString(wxString what) {
	bool have_space=false;
	for(unsigned int i=0;i<what.Len();i++)
		if (what[i]=='\''||what[i]=='\"'||what[i]=='\\') { what=what.Mid(0,i)+"\\"+what.Mid(i); i++; }
		else if (what[i]==' ') have_space=true;
	if (have_space) return wxString("\"")+what+"\"";
	else return what;
}

bool mxApplication::OnInit() {
	
	if (argc==2 && wxString(argv[1])=="--version") {
		_print_version_info("psTerm");
		return false;
	}
	
	bool no_arg=false, debug=false;
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
				} else if (arg=="--debugmode") {
					debug=true;
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
	new mxFrame(command,port,src_id,debug);
	return true;
}
