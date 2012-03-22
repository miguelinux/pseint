#ifndef MXMAINWINDOW_H
#define MXMAINWINDOW_H
#include <wx/frame.h>
#include <wx/aui/aui.h>
#include "mxSource.h"
#include <wx/treebase.h>
#include <wx/treectrl.h>
#include <wx/socket.h>
#include <wx/html/htmlwin.h>

class wxHtmlWindow;
class mxFindDialog;
class wxStaticText;
class wxScrollBar;
class mxDesktopTest;
class mxEvaluateDialog;

enum ds_enum {DS_NONE,DS_STEP,DS_STARTING,DS_PAUSED,DS_RESUMED,DS_STOPPED};

class mxMainWindow : public wxFrame {
private:
	ds_enum ds_state;
	mxEvaluateDialog *evaluate_window;
	friend class DebugManager;
	mxFindDialog *find_replace_dialog;
	friend class mxProcess;
	friend class mxFindDialog;
	friend class mxDebugManager;
	wxHtmlWindow *quick_html;
	wxTreeCtrl *results_tree;
	wxTreeItemId results_root;
	int last_proc;
	wxStaticText *debug_status;
	wxScrollBar *debug_speed;
	wxStatusBar *status_bar;
	wxAuiManager aui_manager;
	wxToolBar *toolbar;
	wxPanel *commands;
	wxPanel *debug_panel;
	wxButton *dp_button_run, *dp_button_step, *dp_button_pause, *dp_button_desktop_vars, *dp_button_evaluate;
	mxDesktopTest *desktop_test_grid;
	void CreateDesktopTestGrid();
	void CreateMenus();
	void CreateToolbars();
	void CreateNotebook();
	void CreateResultsTree();
	void CreateQuickHelp();
	void CreateCommandsPanel();
	void CreateDebugControlsPanel();
	wxStatusBar *OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name);
	wxAuiNotebook *notebook; 
public:
	
	mxSource *NewProgram();
	mxSource *OpenProgram(wxString file, bool history=true);
	
	mxMainWindow(wxPoint pos, wxSize size);
	mxSource *last_source;
	~mxMainWindow();
	
	void RegenFileMenu(wxString path);
	
	void OnFileNew(wxCommandEvent &evt);
	void OnFileOpen(wxCommandEvent &evt);
	void OnFileSave(wxCommandEvent &evt);
	void OnFileSaveAs(wxCommandEvent &evt);
	void OnFilePrint(wxCommandEvent &event);
	void OnFileEditFlow(wxCommandEvent &evt);
	void OnFileExportCpp(wxCommandEvent &evt);
	void OnFileClose(wxCommandEvent &evt);
	void OnFileExit(wxCommandEvent &evt);
	void OnFileSourceHistory (wxCommandEvent &event);
	void OnEdit(wxCommandEvent &evt);
	void OnEditFind(wxCommandEvent &evt);
	void OnEditFindNext(wxCommandEvent &evt);
	void OnEditFindPrev(wxCommandEvent &evt);
	void OnEditReplace(wxCommandEvent &evt);
	void OnRunRun(wxCommandEvent &evt);
	void OnRunStepStep(wxCommandEvent &evt);
	void OnRunCheck(wxCommandEvent &evt);
	void OnRunDrawFlow(wxCommandEvent &evt);
	void OnRunSaveFlow(wxCommandEvent &evt);
	void OnRunSetInput(wxCommandEvent &evt);
	void OnHelpAbout(wxCommandEvent &evt);
	void OnHelpIndex(wxCommandEvent &evt);
	void OnHelpQuickHelp(wxCommandEvent &evt);
	void OnHelpExamples(wxCommandEvent &evt);
	void OnHelpUpdates (wxCommandEvent &event);

	void OnViewNotebookNext (wxCommandEvent &event);
	void OnViewNotebookPrev (wxCommandEvent &event);
	
	void OnConfigHighRes(wxCommandEvent &evt);
	void OnConfigShowToolbar(wxCommandEvent &evt);
	void OnConfigCalltipHelps(wxCommandEvent &evt);
	void OnConfigAutoComp(wxCommandEvent &evt);
	void OnConfigShowCommands(wxCommandEvent &evt);
	void OnToolbarShowDebugPanel(wxCommandEvent &evt);
	void OnConfigShowDebugPanel(wxCommandEvent &evt);
	void OnConfigShowQuickHelp(wxCommandEvent &evt);
	void OnConfigUseColors(wxCommandEvent &evt);
	void OnConfigLanguage(wxCommandEvent &evt);
//	void OnConfigShowResults(wxCommandEvent &evt);
//	void OnConfigColourSintax(wxCommandEvent &evt);
	void OnConfigSmartIndent(wxCommandEvent &evt);
	void OnConfigStepStepL(wxCommandEvent &evt);
	void OnConfigStepStepM(wxCommandEvent &evt);
	void OnConfigStepStepH(wxCommandEvent &evt);
	
	void OnDoThat(wxCommandEvent &evt);
	
	void OnSocketEvent(wxSocketEvent &event);
	void OnScrollDegugSpeed(wxScrollEvent &evt);
	
	wxMenuItem *mi_toolbar, *mi_commands, *mi_autocomp, *mi_quickhelp, /* *mi_results, *mi_sintax,*/ *mi_smart_indent, *mi_debug_panel, *mi_calltip_helps,
		*mi_init_vars, *mi_dot_and_comma,  *mi_stepstep_h, *mi_stepstep_l, *mi_stepstep_m, *mi_word_operators, *mi_use_colors, *mi_high_res;
	
	wxMenu *file_menu;
	wxMenuItem *file_history[5];
	
	void OnClose(wxCloseEvent &evt);
	void OnSelectError(wxTreeEvent &evt);
	void ShowResults(bool error=false);
	void ShowQuickHelp(wxString text);

	void OnCmdEscribir(wxCommandEvent &evt);
	void OnDebugButton(wxCommandEvent &evt);
	void OnDebugPause(wxCommandEvent &evt);
	void OnDebugStep(wxCommandEvent &evt);
	void OnDebugDesktopVars(wxCommandEvent &evt);
	void OnDebugEvaluate(wxCommandEvent &evt);
	void OnDebugHelp(wxCommandEvent &evt);
	void OnCmdLeer(wxCommandEvent &evt);
	void OnCmdAsignar(wxCommandEvent &evt);
	void OnCmdPara(wxCommandEvent &evt);
	void OnCmdMientras(wxCommandEvent &evt);
	void OnCmdRepetir(wxCommandEvent &evt);
	void OnCmdSi(wxCommandEvent &evt);
	void OnCmdSegun(wxCommandEvent &evt);
	void InsertCode(wxArrayString &toins);

	void OnPaneClose(wxAuiManagerEvent& event);
	void OnNotebookPageClose(wxAuiNotebookEvent& event);
	
	const wxArrayString &GetDesktopVars();
	void SetDesktopVars(bool do_dt, const wxArrayString &vars);

	void ShowDesktopTestGrid();
	void HideDesktopTestGrid();
	void SelectLine(mxSource *src, int line);
	
	void StartDebugging(mxSource *source, bool paused);
	void SetDebugState(ds_enum state);
	
	bool SelectFirstError();
	
	void SetAccelerators();
	
	void OnLink (wxHtmlLinkEvent &event);
		
	void SetWordsForSources();
	
	mxSource *FindFlowId(int id);
	mxSource *FindFlowSocket(wxObject *s);
	void SelectSource(mxSource *s);
	
	DECLARE_EVENT_TABLE();
};

extern mxMainWindow *main_window;

#endif

