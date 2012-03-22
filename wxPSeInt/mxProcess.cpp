#include "mxProcess.h"
#include "ConfigManager.h"
#include "mxUtils.h"

#include <wx/arrstr.h>
#include "mxMainWindow.h"
#include <wx/textfile.h>
#include "DebugManager.h"
#include <wx/filename.h>
#include "mxInputDialog.h"
using namespace std;

mxProcess *proc_list;

enum { mxPW_NULL=0, mxPW_RUN, mxPW_DEBUG, mxPW_CHECK, mxPW_CHECK_AND_RUN, mxPW_CHECK_AND_DEBUG, mxPW_DRAW, mxPW_CHECK_AND_DRAW, mxPW_DRAWEDIT, mxPW_CHECK_AND_DRAWEDIT, mxPW_EXPORT, mxPW_CHECK_AND_EXPORT, mxPW_CHECK_AND_SAVEDRAW, mxPW_SAVEDRAW };
	
mxProcess *proc_for_killing=NULL;

int mxProcess::cont=0;

mxProcess::mxProcess(mxSource *src, wxString aname) {
	prev=NULL;
	next=proc_list;
	proc_list=this;
	if (next)
		next->prev=this;
	filename = aname;
	cont++;
	source = src;
	if (proc_for_killing) {
		delete proc_for_killing;
		proc_for_killing=NULL;
	}
	what = mxPW_NULL;
	input = NULL;
	output = NULL;
}

mxProcess::~mxProcess() {
	if (prev)
		prev->next=next;
	else
		proc_list=next;
	if (next)
		next->prev=prev;
}

void mxProcess::OnTerminate(int pid, int status) {
	if (this==debug->process) {
		debug->debugging=false;
		main_window->SetDebugState(DS_STOPPED);
		source->SetReadOnly(was_readonly);
		source->SetDebugLine();
		debug->process=NULL;
	}
	if (what==mxPW_DRAWEDIT) {
		source->EditFlow(NULL);
	}
	if (what==mxPW_RUN) {
		ReadOut();
	}
	if (proc_for_killing) {
		delete proc_for_killing;
		proc_for_killing=NULL;
	}
	proc_for_killing=this;
}

bool mxProcess::CheckSyntax(wxString file, wxString parsed, int id) {
	
	if (what==mxPW_NULL) what=mxPW_CHECK;
	
	wxString command;	
	command<<config->pseint_command<<_T(" --nouser --norun \"")<<file<<_T("\"");
	
	command<<GetProfileArgs();
	
	if (parsed!=wxEmptyString)
		command<<_(" --draw \"")<<parsed<<_T("\"");
	
	wxArrayString output;
	wxExecute(command,output,wxEXEC_SYNC);
	
	main_window->results_tree->DeleteChildren(main_window->results_root);
	for (unsigned int i=0;i<output.GetCount();i++)
		main_window->results_tree->AppendItem(main_window->results_root,output[i],1);
	
	main_window->last_source = source;
	
	if (output.GetCount()) {
		main_window->ShowResults();
		if (output.GetCount()==1)
			main_window->results_tree->SetItemText(main_window->results_root,filename+_T(": Sintaxis Incorrecta: un error."));
		else
			main_window->results_tree->SetItemText(main_window->results_root,filename+wxString(_T(": Sintaxis Incorrecta: "))<<output.GetCount()<<_T(" errores."));
		main_window->SelectFirstError();
		wxTreeItemIdValue v;
		wxTreeItemId item(main_window->results_tree->GetFirstChild(main_window->results_root,v));
		wxTreeEvent evt(0,main_window->results_tree,item);
		main_window->OnSelectError(evt);
		main_window->Raise();
	} else
		main_window->results_tree->SetItemText(main_window->results_root,filename+_T(": Sintaxis Correcta"));
		
	if (!output.GetCount()) {
		if (what==mxPW_CHECK_AND_RUN)
			return Run(file,false);
		else if (what==mxPW_CHECK_AND_DEBUG)
			return Debug(file,false);
		else if (what==mxPW_CHECK_AND_DRAW)
			return Draw(file,false);
		else if (what==mxPW_CHECK_AND_DRAWEDIT)
			return DrawAndEdit(file,id,false);
		else if (what==mxPW_CHECK_AND_SAVEDRAW)
			return SaveDraw(file,false);
		else if (what==mxPW_CHECK_AND_EXPORT)
			return ExportCpp(file,false);
		else
			main_window->ShowResults();
	} else
		proc_for_killing = this;
	return output.GetCount()==0;
}

bool mxProcess::Run(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_RUN:mxPW_RUN;
	if (check_first) return CheckSyntax(file);
	wxString command;
	if (config->tty_command.Len()) {
		command<<config->tty_command<<_T(" ");
		command.Replace(_T("$name"),_T("Ejecucion"));
	}
	temp = config->temp_out;
	temp<<_T(".")<<cont;
	command<<config->pseint_command<<_T(" --nocheck \"")<<file<<_T("\" \"")<<temp<<_T("\"");
	if (config->use_colors)
		command<<_T(" --color");
	command<<GetProfileArgs()<<" "<<GetInputArgs();
//	cerr<<command<<endl;
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

bool mxProcess::Debug(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_DEBUG:mxPW_DEBUG;
	if (check_first) 
		return CheckSyntax(file);
	wxString command;
	if (config->tty_command.Len()) {
		command<<config->tty_command<<_T(" ");
		command.Replace(_T("$name"),_T("Ejecucion"));
	}
	temp = config->temp_out;
	temp<<_T(".")<<cont;
	int port=config->debug_port;
	int delay=250+750*(2-config->stepstep_speed);
	main_window->debug_speed->SetThumbPosition(debug->GetSpeed(delay));
	command<<config->pseint_command<<_T(" --port=")<<port<<_T(" --delay=")<<delay<<_T(" --nocheck \"")<<file<<_T("\" \"")<<temp<<_T("\"");
	if (config->use_colors)
		command<<_T(" --color");
	command<<GetProfileArgs()<<" "<<GetInputArgs();
	debug->Start(this,source,port);
	was_readonly = source->GetReadOnly();
	if (pid) source->SetReadOnly(true);
	pid = wxExecute(command, wxEXEC_ASYNC, this);
	return pid!=0;
}

bool mxProcess::Draw(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_DRAW:mxPW_DRAW;
	if (check_first) return CheckSyntax(file,config->temp_draw);
	wxString command;
	command<<config->psdraw2_command<<_T(" --noedit \"")<<config->temp_draw<<_T("\"");
//	if (config->high_res_flows) command<<_T(" +");
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

bool mxProcess::DrawAndEdit(wxString file, int id, bool check_first) {
	what = check_first?mxPW_CHECK_AND_DRAWEDIT:mxPW_DRAWEDIT;
	if (check_first) return CheckSyntax(file,config->temp_draw,id);
	wxString command;
	command<<config->psdraw2_command;
	command<<" --port="<<config->flow_port<<" --id="<<id;
	if (source->GetReadOnly()) command<<" --noedit";
	command<<_T(" \"")<<config->temp_draw<<_T("\"");
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

bool mxProcess::SaveDraw(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_SAVEDRAW:mxPW_SAVEDRAW;
	if (check_first) return CheckSyntax(file,config->temp_draw);
	wxFileDialog dlg (main_window, _T("Guardar Dibujo"),_T(""),_T(""), _T("Imagen jpg|*.jpg|Imagen bmp|*.bmp|Imagen png|*.png"),  wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK) return false;
	wxString command;
	wxString ext=_T("jpg");
	if (dlg.GetFilterIndex()==1) ext=_T("bmp");
	else if (dlg.GetFilterIndex()==2) ext=_T("png");
	wxString fname = dlg.GetPath();
	if (fname.Len()<4 || fname.Right(4).MakeLower()!=wxString(_T("."))+ext)
		fname<<_T(".")<<ext;
	command<<config->psdraw_command<<_T(" \"")<<config->temp_draw<<_T("\" ");
	if (config->high_res_flows) command<<_T(" +");
	command<<ext<<_T(" \"")<<fname<<_T("\"");
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

bool mxProcess::ExportCpp(wxString file, bool check_first) {
	what = check_first?mxPW_CHECK_AND_EXPORT:mxPW_EXPORT;
	if (check_first) return CheckSyntax(file,config->temp_draw);
	wxFileDialog dlg (main_window, _T("Guardar Cpp"),config->last_dir,_T(""), _T("Archivo C++|*.cpp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK) return false;
	config->last_dir=wxFileName(dlg.GetPath()).GetPath();
	wxString command;
	command<<config->psexport_command<<_T(" \"")<<config->temp_draw<<_T("\" \"")<<dlg.GetPath()<<_T("\"");
	if (config->lang.base_zero_arrays) command<<_T(" --basezeroarrays");
	return wxExecute(command, wxEXEC_ASYNC, this)!=0;
}

void mxProcess::ReadOut() {
	main_window->results_tree->DeleteChildren(main_window->results_root);	
	wxTextFile fil(temp);
	bool happy_ending = false;
	if (fil.Exists()) {
		fil.Open();
		for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
			if (str[0]=='*') {
//				if (str.Contains("Interrumpida"))
//					main_window->results_tree->SetItemText(main_window->results_root,filename+_T(": Ejecucion Interrumpida"));
//				else 
				if (str.Contains(_T("Finalizada"))) {
					happy_ending=true;
					main_window->results_tree->SetItemText(main_window->results_root,filename+_T(": Ejecucion Finalizada"));
				}
			} else {
				if (str.Len())
					main_window->results_tree->AppendItem(main_window->results_root,str,1);
			}
		}
		fil.Close();
		wxRemoveFile(temp);
		if (!happy_ending) {
			main_window->results_tree->SetItemText(main_window->results_root,filename+_T(": Ejecucion Interrumpida"));
			main_window->SelectFirstError();
			wxTreeItemIdValue v;
			wxTreeItemId item(main_window->results_tree->GetFirstChild(main_window->results_root,v));
			wxTreeEvent evt(0,main_window->results_tree,item);
			main_window->OnSelectError(evt);
		} else {
			if (main_window->notebook->GetPageCount()) main_window->notebook->GetPage(main_window->notebook->GetSelection())->SetFocus();
		}
	}
	main_window->ShowResults(!happy_ending);
}

wxString mxProcess::GetProfileArgs() {
	wxString command;
	if (config->lang.force_dot_and_comma)
		command<<_T(" --forcedotandcomma");
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
