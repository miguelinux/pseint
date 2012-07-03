#include "RTSyntaxManager.h"
#include "mxProcess.h"
#include "ConfigManager.h"
#include <wx/txtstrm.h>
#include "mxMainWindow.h"

RTSyntaxManager *RTSyntaxManager::the_one=NULL;
int RTSyntaxManager::lid=0;

RTSyntaxManager::RTSyntaxManager():wxProcess(wxPROCESS_REDIRECT) {
	processing=running=restart=false;
	id=++lid;
}

void RTSyntaxManager::Start ( ) {
	if (the_one) {
		if (the_one->running) return;
		delete the_one;
	}
	the_one=new RTSyntaxManager;
	the_one->restart=false;
	wxString command;
	command<<config->pseint_command<<_T(" --forrealtimesyntax")<<mxProcess::GetProfileArgs();
	the_one->pid=wxExecute(command,wxEXEC_ASYNC,the_one);
}

void RTSyntaxManager::Stop ( ) {
	if (the_one->pid<=0) return;
	the_one->Kill(the_one->pid,wxSIGKILL);
}

void RTSyntaxManager::Restart ( ) {
	if (!the_one) Start();
	else { the_one->restart=true; Stop(); }
}

bool RTSyntaxManager::Process (mxSource * src) {
	if (!the_one) Start(); else if (the_one->restart || the_one->processing) return false;
	int mid=the_one->id;
	wxTextOutputStream output(*(the_one->GetOutputStream()));
	wxTextInputStream input(*(the_one->GetInputStream()));	
	the_one->processing=true;
	for(int i=0;i<src->GetLineCount();i++) {
		wxString s=src->GetLine(i);
		while (s.Len()&&(s.Last()=='\r'||s.Last()=='\n')) s.RemoveLast();
		output<<s<<"\n";
	}
	output<<"<!{[END_OF_INPUT]}!>\n";
	src->ClearErrors();
	while (true) {
		wxString line; char c;
		while (the_one->IsInputAvailable()) {
			c=input.GetChar();
			if (c=='\n') break;
			if (c!='\r') line<<c;
		}
		if (line.Len()) {
			if (line=="<!{[END_OF_OUTPUT]}!>") break;
			long l=-1,i=-1;
			line.AfterFirst(' ').BeforeFirst(' ').ToLong(&l);
			line.BeforeFirst(':').AfterLast(' ').BeforeLast(')').ToLong(&i);
			line=line.AfterFirst(':').AfterFirst(':').Mid(1);
			src->MarkError(l-1,i-1,line,line.StartsWith("Falta cerrar "));
		} else {
			wxYield();
			if (!the_one||the_one->id!=mid) return false;
		}
	}	
	the_one->processing=false;
	return true;
}

void RTSyntaxManager::OnTerminate (int pid, int status) {
	if (restart) {
		Start(); 
		main_window->UpdateRealTimeSyntax();
	} else the_one=NULL;
}

bool RTSyntaxManager::IsLoaded ( ) {
	return the_one!=NULL;
}

