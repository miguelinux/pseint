#include <wx/txtstrm.h>
#include "RTSyntaxManager.h"
#include "mxProcess.h"
#include "ConfigManager.h"
#include "mxMainWindow.h"
#include "mxVarWindow.h"
#include "ids.h"
#include <iostream>
using namespace std;

#ifdef DEBUG
	#define _LOG(a) cerr<<a<<endl
#else
	#define _LOG(a)
#endif

RTSyntaxManager *RTSyntaxManager::the_one=NULL;
int RTSyntaxManager::lid=0;

RTSyntaxManager::RTSyntaxManager():wxProcess(wxPROCESS_REDIRECT) {
	processing=running=restart=false;
	timer = new wxTimer(main_window->GetEventHandler(),mxID_RT_TIMER);
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
	if (the_one && the_one->pid<=0) return;
	if (the_one) { the_one->Kill(the_one->pid,wxSIGKILL); the_one->src=NULL; }
}

void RTSyntaxManager::Restart ( ) {
	if (!the_one) Start();
	else { the_one->restart=true; Stop(); }
}

bool RTSyntaxManager::Process (mxSource * src) {
	if (!src) { 
		if (the_one && the_one->processing) { the_one->ContinueProcessing(); return true; }
		cerr<<"ERROR: the_one->Process(NULL) && ( !the_one || !the_one->processing)"<<endl;
		return false; // no deberia pasar (solo si no puede lanzar el interprete o revienta enseguida)
	}
	if (!the_one) Start(); else if (the_one->processing || the_one->restart) return false;
_LOG("PROCESS IN:  "<<src);
//	int mid=the_one->id; // ¿para que era esto?
	the_one->src=src;
	wxTextOutputStream output(*(the_one->GetOutputStream()));
	the_one->processing=true;
	for(int i=0;i<src->GetLineCount();i++) {
		wxString s=src->GetLine(i);
		while (s.Len()&&(s.Last()=='\r'||s.Last()=='\n')) s.RemoveLast();
		output<<s<<"\n"; wxMilliSleep(0); // el millisleep evita problemas cuando se manda mucho y no alcanza a "flushear" (en wx 2.9 ya hay un .flush(), tendria que probar con eso, en 2.8 no hay caso)
	}
	output<<"<!{[END_OF_INPUT]}!>\n";
	src->ClearErrors();
	src->ClearBlocks();
	the_one->fase_num=0;
	vars_window->BeginInput();
	the_one->ContinueProcessing();
_LOG("PROCESS OUT: "<<src);
	return true;
}

void RTSyntaxManager::ContinueProcessing() {
	if (!src) return;
_LOG("CONTINUE PROCESSING IN:     "<<src);
	wxTextInputStream input(*(GetInputStream()));	
	while(true) {
		wxString line; char c;
		while (IsInputAvailable()) {
			c=input.GetChar();
			if (c=='\n') break;
			if (c!='\r') line<<c;
		}
_LOG("     ?"<<line<<"¿");
		if (line.Len()) {
			if (line=="<!{[END_OF_OUTPUT]}!>") { 
				fase_num=1;
			} else if (line=="<!{[END_OF_VARS]}!>") {
				vars_window->EndInput();
				fase_num=2;
			} else if (line=="<!{[END_OF_BLOCKS]}!>") {
				processing=false;
_LOG("CONTINUE PROCESSING OUT 2:  "<<src);
				return;
			} else if (fase_num==0 && config->rt_syntax) {
				long l=-1,i=-1,n;
				line.AfterFirst(':').BeforeFirst(':').AfterLast(' ').ToLong(&n);
				line.AfterFirst(' ').BeforeFirst(' ').ToLong(&l);
				line.BeforeFirst(':').AfterLast(' ').BeforeLast(')').ToLong(&i);
				line=line.AfterFirst(':').AfterFirst(':').Mid(1);
_LOG("     fase 0a");
				src->MarkError(l-1,i-1,n,line,line.StartsWith("Falta cerrar "));
_LOG("     fase 0b");
			} else if (fase_num==1 && config->show_vars) {
				wxString what=line.BeforeFirst(' ');
				if (what=="PROCESO"||what=="SUBPROCESO")
					vars_window->Add(line.AfterFirst(' '),what=="PROCESO");
				else
					vars_window->Add(what,line.Last());
			} else if (config->highlight_blocks) {
				long l1,l2;
				if (line.BeforeFirst(' ').ToLong(&l1) && line.AfterFirst(' ').ToLong(&l2)) 
					src->AddBlock(l1-1,l2-1);
			}
		} else {
_LOG("CONTINUE PROCESSING OUT 1:  "<<src);
			timer->Start(100,true);
			return;
		}
	}
}

void RTSyntaxManager::OnTerminate (int pid, int status) {
	if (restart) {
		Start(); 
		main_window->UpdateRealTimeSyntax();
	} else 
		the_one=NULL;
}

bool RTSyntaxManager::IsLoaded ( ) {
	return the_one!=NULL;
}

RTSyntaxManager::~RTSyntaxManager ( ) {
	timer->Stop();
	the_one=NULL;
}

