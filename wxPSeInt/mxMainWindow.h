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
class mxDesktopTestPanel;

class mxMainWindow : public wxFrame {
private:
	friend class DebugManager;
	mxFindDialog *find_replace_dialog;
	friend class mxSource; // para el page_text
	friend class mxProcess;
	friend class mxFindDialog;
	wxHtmlWindow *quick_html;
	int last_proc;
	wxAuiManager aui_manager;
	wxToolBar *toolbar;
	wxPanel *commands;
	void CreateDesktopTestPanel();
	void CreateVarsPanel();
	void CreateOpersPanel();
	void CreateMenus();
	void CreateToolbars();
	void CreateNotebook();
	void CreateResultsTree();
	void CreateQuickHelp();
	void CreateCommandsPanel();
	void CreateDebugControlsPanel();
	void CreateStatusBar();
	wxAuiNotebook *notebook; 
	wxButton *button_subproc;
public:
	
	mxSource *NewProgram();
	mxSource *OpenProgram(wxString file, bool is_example=false);
	
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
	void OnFileExportHtml(wxCommandEvent &evt);
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
	void OnRunSubtitles(wxCommandEvent &evt);
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
	
//	void OnConfigHighRes(wxCommandEvent &evt);
//	void OnConfigShowToolbar(wxCommandEvent &evt);
	void OnConfigCalltipHelps(wxCommandEvent &evt);
	void OnConfigAutoComp(wxCommandEvent &evt);
	void OnConfigHighlightBlocks(wxCommandEvent &evt);
	void OnConfigAutoClose(wxCommandEvent &evt);
//	void OnConfigShowCommands(wxCommandEvent &evt);
//	void OnConfigShowVars(wxCommandEvent &evt);
//	void OnToolbarShowDebugPanel(wxCommandEvent &evt);
//	void OnConfigShowDebugPanel(wxCommandEvent &evt);
	void OnConfigShowQuickHelp(wxCommandEvent &evt);
	void OnConfigAnimateGui(wxCommandEvent &evt);
	void OnConfigReorganizeForDebug(wxCommandEvent &evt);
	void OnConfigUseColors(wxCommandEvent &evt);
	void OnConfigUsePSTerm(wxCommandEvent &evt);
	void OnConfigNassiScheiderman(wxCommandEvent &evt);
	void OnConfigLanguage(wxCommandEvent &evt);
	void OnConfigRealTimeSyntax(wxCommandEvent &evt);
	void OnConfigSmartIndent(wxCommandEvent &evt);
	
	void OnDoThat(wxCommandEvent &evt);
	void OnDebugShortcut(wxCommandEvent &evt);
	
	void OnSocketEvent(wxSocketEvent &event);
	void OnScrollDegugSpeed(wxScrollEvent &evt);
	
	wxMenuItem *mi_autocomp, *mi_autoclose, *mi_highlight_blocks, *mi_quickhelp, *mi_smart_indent,
		//*mi_debug_panel, *mi_toolbar, *mi_commands, *mi_vars_panel
		*mi_calltip_helps, *mi_rt_syntax, *mi_nassi_schne, *mi_use_colors, *mi_reorganize_for_debug, *mi_use_psterm, *mi_animate_gui;
	
	wxMenu *file_menu;
	wxMenuItem *file_history[5];
	
	void OnClose(wxCloseEvent &evt);
	void OnSelectError(wxTreeEvent &evt);
	void SelectError(wxString text);
	void HideQuickHelp();

	void OnCmdEscribir(wxCommandEvent &evt);
	void OnCmdLeer(wxCommandEvent &evt);
	void OnCmdAsignar(wxCommandEvent &evt);
	void OnCmdPara(wxCommandEvent &evt);
	void OnCmdMientras(wxCommandEvent &evt);
	void OnCmdRepetir(wxCommandEvent &evt);
	void OnCmdSi(wxCommandEvent &evt);
	void OnCmdSegun(wxCommandEvent &evt);
	void OnCmdSubProceso(wxCommandEvent &evt);
	void InsertCode(wxString toins);
	void InsertCode(wxArrayString &toins);

	void OnPaneClose(wxAuiManagerEvent& event);
	void OnNotebookPageChange(wxAuiNotebookEvent& event);
	void OnNotebookPageClose(wxAuiNotebookEvent& event);
	
//	const wxArrayString &GetDesktopVars();
//	void SetDesktopVars(bool do_dt, const wxArrayString &vars);

	void SelectLine(mxSource *src, int line);
	
	bool SelectFirstError();
	
	void SetAccelerators();
	
	void OnLink (wxHtmlLinkEvent &event);
		
	mxSource *FindSourceById(int id);
	mxSource *FindFlowSocket(wxObject *s);
	void SelectSource(mxSource *s);
	
	void ProfileChanged(); // lo llama mxConfig para que actualice el menu configurar y reinicie el rtsyntax
	void UpdateRealTimeSyntax(); // lo llama rtsyntax despues de reiniciarse
	void OnRTSyntaxAuxTimer(wxTimerEvent &event);
	
	mxSource *GetCurrentSource();
	
	void OnHelperOpers(wxCommandEvent &evt);
	void OnHelperVars(wxCommandEvent &evt);
	void OnHelperDebug(wxCommandEvent &evt);
	void OnHelperCommands(wxCommandEvent &evt);
	
	void CheckIfNeedsRTS();
	
	void ShowOpersPanel(bool show, bool anim=false);
	void ShowVarsPanel(bool show, bool anim=false);
	void ShowDebugPanel(bool show, bool anim=false);
	void ShowCommandsPanel(bool show, bool anim=false);
	void ShowResults(bool show, bool no_error);
	void ShowQuickHelp(bool show, wxString text="", bool load=false);
	void ShowSubtitles(bool show, bool anim=false);
	void ShowDesktopTestPanel(bool show, bool anim=false);
	
	void ParseResults(mxSource *source); // analiza el archivo de salida que genera una ejecucion, que contiene los errores de la misma
	
	void RunCurrent(bool raise, bool from_psdraw=false); // ejecuta o re-ejecuta el source actual (si ya estaba corriendo en un psterm, raise indica si debe pasar al frente esa ventana)
	
	void ReorganizeForDebugging();
	
	bool IsQuickHelpVisible();
	
	// para los paneles laterales
	void ShowPanel(wxString helper, wxWindow *panel, bool anim=true);
	void HidePanel(wxString helper, wxWindow *panel, bool anim=true);
	// para los paneles inferiores
	void ShowPanel(wxWindow *panel, bool anim=true);
	void HidePanel(wxWindow *panel, bool anim=true);
	
private:
	bool result_tree_done;
	wxTreeItemId results_root, results_last;
	wxTreeCtrl *results_tree_ctrl; // si no hay rt_syntax mejor no usar el result_tree y poner todo en el...
	wxString results_tree_text; // ...si no se usa el resulttree, aca va acumulando el equivalente para mostrarlo en el quick_html
	wxArrayString results_tree_errors; // lista de errores para obtener el texto completo al hacer click en los mismos como links en el quick_html
	int result_tree_text_level; // al ir armando por partes con el RTreeAdd el texto del html, se dejan listas e items incompletos (<OL>,<LI>), esto indica qué
	wxString RTreeAdd_auxHtml(wxString str); // pasa a html un mesaje de error, reemplazando caracteres especiales y convirtiendolo en hyperlink si corresponde
public:
	void RTreeReset();
	void RTreeAdd(wxString text, int type, mxSource *source=NULL);
	void RTreeDone(bool show, bool error);
	
	DECLARE_EVENT_TABLE();
};

extern mxMainWindow *main_window;

#endif

