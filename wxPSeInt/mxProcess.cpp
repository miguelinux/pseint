#include "mxProcess.h"
#include "ConfigManager.h"
#include "mxUtils.h"

#include <wx/arrstr.h>
#include "mxMainWindow.h"
#include "mxStatusBar.h"
#include <wx/textfile.h>
#include "DebugManager.h"
#include <wx/filename.h>
#include "mxInputDialog.h"
#include "FlowEditionManager.h"
#include "mxDebugWindow.h"
#include "CommunicationsManager.h"
#include <iostream>
#include "Logger.h"
using namespace std;

mxProcess *proc_list;

enum { mxPW_NULL=0, mxPW_RUN, mxPW_DEBUG, mxPW_CHECK, mxPW_CHECK_AND_RUN, mxPW_CHECK_AND_DEBUG, mxPW_DRAW, mxPW_CHECK_AND_DRAW, mxPW_DRAWEDIT, mxPW_CHECK_AND_DRAWEDIT, mxPW_EXPORT, mxPW_CHECK_AND_EXPORT, mxPW_CHECK_AND_SAVEDRAW, mxPW_SAVEDRAW };
	
mxProcess *proc_for_killing=NULL;

int mxProcess::cont=0;

mxProcess::mxProcess(mxSource *src) {
	
	_LOG("mxProcess::mxProcess "<<src);
	
	prev=NULL;
	next=proc_list;
	proc_list=this;
	if (next)
		next->prev=this;
	filename = src->GetPageText();
	cont++;
	source = src;
	if (proc_for_killing) 
		delete proc_for_killing;
	what = mxPW_NULL;
	input = NULL;
	output = NULL;
}

mxProcess::~mxProcess() {
	
	_LOG("mxProcess::~mxProcess "<<this);
	
	if (this==proc_for_killing) proc_for_killing=NULL;
	if (prev)
		prev->next=next;
	else
		proc_list=next;
	if (next)
		next->prev=prev;
}

void mxProcess::OnTerminate(int pid, int status) {
	_LOG("mxProcess::OnTerminate "<<status);
	if (this==debug->process) {
		debug->debugging=false;
		debug_panel->SetState(DS_STOPPED);
		if (source) source->SetReadOnly(was_readonly);
		if (source) source->SetDebugLine();
		debug->process=NULL;
	}
	if (what==mxPW_RUN) {
		if (source && !config->use_psterm) main_window->ParseResults(source); // el if es por si el usuario cerro el fuente mientras este corria
	}
	if (proc_for_killing) {
		delete proc_for_killing;
		proc_for_killing=NULL;
	}
	proc_for_killing=this;
}

bool mxProcess::CheckSyntax(wxString file, wxString extra_args) {
	
	if (what==mxPW_NULL) what=mxPW_CHECK;
	
	wxString command;	
	command<<config->pseint_command<<_T(" --nouser --norun \"")<<file<<_T("\"");
	command<<GetProfileArgs();
	if (extra_args!=wxEmptyString) command<<" "<<extra_args;
	
	wxArrayString output;
	_LOG("mxProcess::CheckSyntax "<<command);
	wxExecute(command,output,wxEXEC_SYNC);
	
	main_window->RTreeReset();
	main_window->last_source=source;
	if (_avoid_results_tree) source->ClearErrorData();
	if (output.GetCount()) {
		if (output.GetCount()==1)
			main_window->RTreeAdd(filename+": Sintaxis Incorrecta: un error.",0);
		else
			main_window->RTreeAdd(filename+wxString(": Sintaxis Incorrecta: ")<<output.GetCount()<<" errores.",0);
		for (unsigned int i=0;i<output.GetCount();i++) 
			main_window->RTreeAdd(output[i],1,source);
		if (source) source->SetStatus(STATUS_SYNTAX_CHECK_ERROR);
		main_window->RTreeAdd("",3);
		main_window->RTreeAdd("Las lineas con errores se marcan con una cruz sobre el margen izquierdo. Seleccione un error para ver su descripción:",3);
		main_window->RTreeDone(true,true);
		proc_for_killing = this;
	} else {
		if (!source) return false; // si el fuente se cerro mientras se analizaba (muy poco probable)
		main_window->RTreeAdd(filename+": Sintaxis Correcta",0);
		if (what==mxPW_CHECK_AND_RUN)
			return Run(file,false);
		else if (what==mxPW_CHECK_AND_DEBUG)
			return Debug(file,false);
		else if (what==mxPW_CHECK_AND_DRAW)
			return Draw(file,false);
		else if (what==mxPW_CHECK_AND_DRAWEDIT)
			return DrawAndEdit(file,false);
		else if (what==mxPW_CHECK_AND_SAVEDRAW)
			return SaveDraw(file,false);
		else if (what==mxPW_CHECK_AND_EXPORT)
			return ExportCpp(file,false);
		else if (what==mxPW_CHECK) {
			source->SetStatus(STATUS_SYNTAX_CHECK_OK);
			main_window->RTreeAdd("Presione F9 para ejecutar el algoritmo.",3);
			main_window->RTreeDone(true,false);
		}
	}
	if (_avoid_results_tree) source->ClearErrorMarks();
	return output.GetCount()==0;
}

bool mxProcess::Run(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_RUN:mxPW_RUN;
	source->StopReloadTimer();
	if (check_first) return CheckSyntax(file);
	wxString command, tty_command=config->GetTTYCommand();
	if (tty_command.Len()) {
		command<<tty_command<<_T(" ");
		command.Replace(_T("$name"),_T("Ejecucion"));
	}
	if (config->use_psterm) command<<_T(" --port=")<<comm_manager->GetServerPort()<<" --id="<<source->GetId()<<" ";
	temp = source->GetTempFilenameOUT();
	command<<config->pseint_command<<_T(" --nocheck \"")<<file<<_T("\" \"")<<temp<<_T("\"");
	if (config->use_colors) command<<_T(" --color");
	if (config->use_psterm) command<<_T(" --forpseintterminal");
#ifdef __WIN32__
	if (!config->use_psterm) command<<" --fixwincharset";
#endif
	command<<GetProfileArgs()<<" "<<GetInputArgs();
	if (source)	source->SetStatus(STATUS_RUNNING);
	_LOG("mxProcess::Run "<<command);
	return wxExecute(command, wxEXEC_ASYNC|wxEXEC_MAKE_GROUP_LEADER, this)!=0;
}

bool mxProcess::Debug(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_DEBUG:mxPW_DEBUG;
	if (check_first) 
		return CheckSyntax(file);
	wxString command, tty_command=config->GetTTYCommand();
	if (tty_command.Len()) {
		command<<tty_command<<_T(" ");
		command.Replace(_T("$name"),_T("Ejecucion"));
	}
	if (config->use_psterm) {
		command<<_T(" --debugmode ");
		
		if (config->reorganize_for_debug) {
			int x,y,h,w, delta=10;
			main_window->notebook->GetScreenPosition(&x,&y);
			main_window->notebook->GetSize(&w,&h);
			command<<_T(" --right=")<<x+w-delta<<_T(" ");
			command<<_T(" --top=")<<y+delta<<_T(" ");
			command<<_T(" --alwaysontop ");
			if (w-500<400) command<<_T("--width=400 ");
		}
	}
	temp = source->GetTempFilenameOUT();
	debug->Start(this,source);
	int port=comm_manager->GetServerPort();
	int delay=_calc_delay(config->stepstep_tspeed);
	debug_panel->SetSpeed(debug->GetSpeed(delay));
	command<<config->pseint_command<<_T(" --port=")<<port<<_T(" --delay=")<<delay<<_T(" --nocheck \"")<<file<<_T("\" \"")<<temp<<_T("\"");
	if (config->use_colors) command<<_T(" --color");
	if (config->use_psterm) command<<_T(" --forpseintterminal");
	command<<GetProfileArgs()<<" "<<GetInputArgs();
	was_readonly = source->GetReadOnly();
	if (pid) source->SetReadOnly(true);
	_LOG("mxProcess::Debug "<<command);
	pid = wxExecute(command, wxEXEC_ASYNC|wxEXEC_MAKE_GROUP_LEADER, this);
	return pid!=0;
}

bool mxProcess::Draw(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_DRAW:mxPW_DRAW;
	if (check_first) return CheckSyntax(file,wxString("--draw --usecasemap \"")<<source->GetTempFilenamePSD()<<"\"");
	wxString command;
	command<<config->psdraw2_command<<" --noedit "<<(!config->lang.word_operators?"--nowordoperators ":"")<<(config->lang.use_nassi_schneiderman?"--nassischneiderman ":"")<<"\""<<source->GetTempFilenamePSD()<<_T("\"");
	_LOG("mxProcess::Draw "<<command);
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

bool mxProcess::DrawAndEdit(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_DRAWEDIT:mxPW_DRAWEDIT;
	if (check_first) return CheckSyntax(file,wxString("--draw --usecasemap \"")<<source->GetTempFilenamePSD()<<"\"");
	wxString command;
	command<<config->psdraw2_command;
	command<<" --port="<<comm_manager->GetServerPort()<<" --id="<<source->GetId();
	if (source->GetReadOnly()) command<<" --noedit";
	if (config->lang.use_nassi_schneiderman) command<<" --nassischneiderman";
	if (config->lang.force_semicolon) command<<" --forcesemicolons";
	if (!config->lang.word_operators) command<<" --nowordoperators";
	command<<_T(" \"")<<source->GetTempFilenamePSD()<<_T("\"");
	_LOG("mxProcess::DrawAndEdit "<<command);
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

bool mxProcess::SaveDraw(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_SAVEDRAW:mxPW_SAVEDRAW;
	if (check_first) return CheckSyntax(file,wxString("--draw --usecasemap \"")<<source->GetTempFilenamePSD()<<"\"");
	wxString command;
	command<<config->psdrawe_command<<_T(" \"")<<source->GetTempFilenamePSD()<<_T("\" ");
	command<<_("\"")<<DIR_PLUS_FILE(source->GetPathForExport(),source->GetNameForExport()+_T(".png"))<<_T("\"");
	if (config->lang.use_nassi_schneiderman) command<<" --nassischneiderman";
	_LOG("mxProcess::SaveDraw "<<command);
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

bool mxProcess::ExportCpp(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_EXPORT:mxPW_EXPORT;
	if (check_first) return CheckSyntax(file,wxString("--draw \"")<<source->GetTempFilenamePSD()<<"\"");
//	wxMessageBox(_T("Si el código define subprocesos o utiliza funciones de manejos de cadenas no se exportará correctamente.\nEstas limitaciones serán solucionadas en las próximas versiones de PSeInt."),_T("Exportar a código C++"),wxOK|wxICON_EXCLAMATION);
	wxFileDialog dlg (main_window, _T("Guardar Cpp"),source->GetPathForExport(),source->GetNameForExport()+_T(".cpp"), _T("Archivo C++|*.cpp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK) return false;
	config->last_dir=wxFileName(dlg.GetPath()).GetPath();
	wxString command;
	command<<config->psexport_command<<_T(" \"")<<source->GetTempFilenamePSD()<<_T("\" \"")<<dlg.GetPath()<<_T("\"");
	if (config->lang.base_zero_arrays) command<<_T(" --basezeroarrays");
	_LOG("mxProcess::ExportCpp "<<command);
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

wxString mxProcess::GetProfileArgs() {
	wxString command;
	if (config->lang.force_semicolon)
		command<<_T(" --forcesemicolon");
	if (!config->lang.enable_string_functions)
		command<<_T(" --hidestringfunctions");
	if (!config->lang.enable_user_functions)
		command<<_T(" --disableuserfunctions");
	if (config->lang.overload_equal)
		command<<_T(" --overloadequal");
	if (!config->lang.lazy_syntax)
		command<<_T(" --nolazysyntax");
	if (config->lang.coloquial_conditions)
		command<<_T(" --coloquialconditions");
	if (!config->lang.word_operators)
		command<<_T(" --nowordoperators");
	if (config->lang.allow_concatenation)
		command<<_T(" --allowconcatenation");
	if (config->lang.allow_dinamyc_dimensions)
		command<<_T(" --allowddims");
	if (config->lang.force_define_vars)
		command<<_T(" --forcedefinevars");
	if (config->lang.force_init_vars)
		command<<_T(" --forceinitvars");
	if (config->lang.base_zero_arrays)
		command<<_T(" --basezeroarrays");
	return command;
}

wxString mxProcess::GetInputArgs() {
	if (!source->input || !source->input->HasInput()) return "";
	wxString args(source->input->IsPartial()?"":"--noinput ");
	wxString values=source->input->GetInput();
	if (!values.Len()) return args;
	values.Replace("\r","");
	if (values.EndsWith("\n")) values.RemoveLast();
	values.Replace("\\","\\\\");
	values.Replace("\"","\\\"");
	values.Replace("\n","\" \"--input=");
	args<<"\"--input="<<values<<"\"";
	return args;
}

void mxProcess::SetSourceDeleted ( ) {
	source=NULL;
}

