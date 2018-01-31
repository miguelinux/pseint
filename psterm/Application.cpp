#include <wx/msgdlg.h>
#include <iostream>
#include "../wxPSeInt/osdep.h"
#include "Application.h"
#include "mxFrame.h"
#include "version.h"

IMPLEMENT_APP(mxApplication)
	
static wxString EscapeString(wxString what) {
	bool have_space=false;
	for(unsigned int i=0;i<what.Len();i++)
		if (what[i]=='\"'
#ifdef __WIN32__
			||(what[i]=='\\'&&what[i+1]=='\"')
#else
			||what[i]=='\''||what[i]=='\\'
#endif
			) { what=what.Mid(0,i)+"\\"+what.Mid(i); i++; }
		else if (what[i]==' ') have_space=true;
	if (have_space) return wxString("\"")+what+"\"";
	else return what;
}

bool mxApplication::OnInit() {
	
	_handle_version_query("psTerm");
	
	OSDep::AppInit();
	
	srand(time(0));
	
	bool no_arg=false, debug=false;
	long port=-1, src_id=-1;
	win_props props;
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
				} else if (arg=="--darktheme") {
					props.dark_theme=true;
				} else if (arg=="--alwaysontop") {
					props.always_on_top=true;
				} else if (arg.StartsWith("--id=")) {
					arg.AfterFirst('=').ToLong(&src_id);
				} else if (arg.StartsWith("--left=")) {
					props.set_left=arg.AfterFirst('=').ToLong(&props.left);
				} else if (arg.StartsWith("--right=")) {
					props.set_right=arg.AfterFirst('=').ToLong(&props.right);
				} else if (arg.StartsWith("--top=")) {
					props.set_top=arg.AfterFirst('=').ToLong(&props.top);
				} else if (arg.StartsWith("--bottom=")) {
					props.set_bottom=arg.AfterFirst('=').ToLong(&props.bottom);
				} else if (arg.StartsWith("--width=")) {
					arg.AfterFirst('=').ToLong(&props.width);
				} else if (arg.StartsWith("--height=")) {
					arg.AfterFirst('=').ToLong(&props.height);
				} else if (arg.StartsWith("--port=")) {
					arg.AfterFirst('=').ToLong(&port);
				} else if (arg.StartsWith("--font=")) {
					wxString fontspec = arg.AfterFirst('=');
					props.font_name = fontspec.BeforeLast(':');
					fontspec.AfterLast(':').ToLong(&props.font_size);
				}
			} else {
				command=EscapeString(argv[i]);
				no_arg=true;
			}
		}
	}
	new mxFrame(command,port,src_id,debug,props);
	return true;
}
