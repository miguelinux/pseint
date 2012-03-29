#include <wx/statusbr.h>
#include <wx/menu.h>
#include <wx/aui/auibook.h>
#include <wx/toolbar.h>
#include <wx/filename.h>
#include <wx/html/htmlwin.h>
#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/textfile.h>
#include <wx/cmndata.h>
#include "mxUtils.h"
#include "mxMainWindow.h"
#include "ids.h"
#include "ConfigManager.h"
#include "mxProcess.h"
#include "version.h"
#include "HelpManager.h"
#include "mxBitmapButton.h"
#include "mxFindDialog.h"
#include "mxAboutWindow.h"
#include "mxHelpWindow.h"
#include "mxDropTarget.h"
#include "DebugManager.h"
#include "mxDesktopTest.h"
#include "mxDesktopVarsEditor.h"
#include "mxEvaluateDialog.h"
#include "mxUpdatesChecker.h"
#include "mxConfig.h"
#include "mxProfile.h"
#include "mxPrintOut.h"
//#include "icon16.xpm"
//#include "icon32.xpm"
//#include "icon48.xpm"
//#include "icon64.xpm"
//#include "icon128.xpm"

#define IF_THERE_IS_SOURCE if (notebook->GetPageCount()>0)
#define CURRENT_SOURCE ((mxSource*)notebook->GetPage(notebook->GetSelection()))
#include "mxInputDialog.h"
#include "FlowEditionManager.h"
#include <iostream>
using namespace std;

mxMainWindow *main_window;

#define _debug_speed_h 80
#define _debug_speed_m 55
#define _debug_speed_l 20

BEGIN_EVENT_TABLE(mxMainWindow, wxFrame)
	EVT_CLOSE(mxMainWindow::OnClose)
	EVT_MENU(mxID_FILE_NEW, mxMainWindow::OnFileNew)
	EVT_MENU(mxID_FILE_OPEN, mxMainWindow::OnFileOpen)
	EVT_MENU(mxID_FILE_SAVE, mxMainWindow::OnFileSave)
	EVT_MENU(mxID_FILE_EDIT_FLOW, mxMainWindow::OnFileEditFlow)
	EVT_MENU(mxID_FILE_EXPORT_CPP, mxMainWindow::OnFileExportCpp)
	EVT_MENU(mxID_FILE_CLOSE, mxMainWindow::OnFileClose)
	EVT_MENU(mxID_FILE_SAVE_AS, mxMainWindow::OnFileSaveAs)
	EVT_MENU(mxID_FILE_PRINT, mxMainWindow::OnFilePrint)
	EVT_MENU(mxID_FILE_EXIT, mxMainWindow::OnFileExit)
	EVT_MENU(mxID_EDIT_UNDO, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_REDO, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_CUT, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_COPY, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_PASTE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_COMMENT, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_UNCOMMENT, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_DUPLICATE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_DELETE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_FIND, mxMainWindow::OnEditFind)
	EVT_MENU(mxID_EDIT_REPLACE, mxMainWindow::OnEditReplace)
	EVT_MENU(mxID_EDIT_FIND_NEXT, mxMainWindow::OnEditFindNext)
	EVT_MENU(mxID_EDIT_FIND_PREV, mxMainWindow::OnEditFindPrev)
	EVT_MENU(mxID_EDIT_SELECT_ALL, mxMainWindow::OnEdit)
	EVT_MENU(mxID_RUN_RUN, mxMainWindow::OnRunRun)
	EVT_MENU(mxID_RUN_STEP_STEP, mxMainWindow::OnRunStepStep)
	EVT_MENU(mxID_RUN_CHECK, mxMainWindow::OnRunCheck)
	EVT_MENU(mxID_RUN_DRAW_FLOW, mxMainWindow::OnRunDrawFlow)
	EVT_MENU(mxID_RUN_SAVE_FLOW, mxMainWindow::OnRunSaveFlow)
	EVT_MENU(mxID_RUN_SET_INPUT, mxMainWindow::OnRunSetInput)
	EVT_MENU(mxID_EDIT_TOGGLE_LINES_UP, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_TOGGLE_LINES_DOWN, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_INDENT_SELECTION, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_BEAUTIFY_CODE, mxMainWindow::OnEdit)
	
	EVT_MENU(mxID_DO_THAT, mxMainWindow::OnDoThat)
	EVT_MENU(mxID_CONFIG_LANGUAGE, mxMainWindow::OnConfigLanguage)
	EVT_MENU(mxID_CONFIG_USE_COLORS, mxMainWindow::OnConfigUseColors)
	EVT_MENU(mxID_CONFIG_SHOW_TOOLBAR, mxMainWindow::OnConfigShowToolbar)
	EVT_MENU(mxID_CONFIG_SHOW_COMMANDS, mxMainWindow::OnConfigShowCommands)
	EVT_MENU(mxID_CONFIG_AUTOCOMP, mxMainWindow::OnConfigAutoComp)
	EVT_MENU(mxID_CONFIG_CALLTIP_HELPS, mxMainWindow::OnConfigCalltipHelps)
	EVT_MENU(mxID_CONFIG_SHOW_QUICKHELP, mxMainWindow::OnConfigShowQuickHelp)
	EVT_MENU(mxID_CONFIG_SHOW_DEBUG_PANEL, mxMainWindow::OnConfigShowDebugPanel)
	EVT_MENU(mxID_CONFIG_REALLY_SHOW_DEBUG_PANEL, mxMainWindow::OnToolbarShowDebugPanel)
//	EVT_MENU(mxID_CONFIG_SHOW_RESULTS, mxMainWindow::OnConfigShowResults)
//	EVT_MENU(mxID_CONFIG_COLOUR_SINTAX, mxMainWindow::OnConfigColourSintax)
	EVT_MENU(mxID_CONFIG_SMART_INDENT, mxMainWindow::OnConfigSmartIndent)
	EVT_MENU(mxID_CONFIG_STEPSTEP_L, mxMainWindow::OnConfigStepStepL)
	EVT_MENU(mxID_CONFIG_STEPSTEP_M, mxMainWindow::OnConfigStepStepM)
	EVT_MENU(mxID_CONFIG_STEPSTEP_H, mxMainWindow::OnConfigStepStepH)
//	EVT_MENU(mxID_CONFIG_HIGHRES, mxMainWindow::OnConfigHighRes)

	EVT_BUTTON(mxID_CMD_ASIGNAR, mxMainWindow::OnCmdAsignar)
	EVT_BUTTON(mxID_CMD_LEER, mxMainWindow::OnCmdLeer)
	EVT_BUTTON(mxID_CMD_ESCRIBIR, mxMainWindow::OnCmdEscribir)
	EVT_BUTTON(mxID_CMD_MIENTRAS, mxMainWindow::OnCmdMientras)
	EVT_BUTTON(mxID_CMD_REPETIR, mxMainWindow::OnCmdRepetir)
	EVT_BUTTON(mxID_CMD_PARA, mxMainWindow::OnCmdPara)
	EVT_BUTTON(mxID_CMD_SI, mxMainWindow::OnCmdSi)
	EVT_BUTTON(mxID_CMD_SEGUN, mxMainWindow::OnCmdSegun)
	EVT_BUTTON(mxID_DEBUG_BUTTON, mxMainWindow::OnDebugButton)
	EVT_BUTTON(mxID_DEBUG_PAUSE, mxMainWindow::OnDebugPause)
	EVT_BUTTON(mxID_DEBUG_STEP, mxMainWindow::OnDebugStep)
	EVT_BUTTON(mxID_DEBUG_HELP, mxMainWindow::OnDebugHelp)
	EVT_BUTTON(mxID_DEBUG_EVALUATE, mxMainWindow::OnDebugEvaluate)
	EVT_BUTTON(mxID_DEBUG_DESKTOP_VARS, mxMainWindow::OnDebugDesktopVars)
//	EVT_MENU(mxID_CONFIG_, mxMainWindow::OnConfig)
//	EVT_MENU(mxID_HELP_, mxMainWindow::OnHelp)
	EVT_MENU_RANGE(mxID_FILE_SOURCE_HISTORY_0, mxID_FILE_SOURCE_HISTORY_0+5,mxMainWindow::OnFileSourceHistory)
	EVT_MENU(mxID_HELP_INDEX, mxMainWindow::OnHelpIndex)
	EVT_MENU(mxID_HELP_QUICKHELP, mxMainWindow::OnHelpQuickHelp)
	EVT_MENU(mxID_HELP_ABOUT, mxMainWindow::OnHelpAbout)
	EVT_MENU(mxID_HELP_EXAMPLES, mxMainWindow::OnHelpExamples)
	EVT_MENU(mxID_HELP_UPDATES, mxMainWindow::OnHelpUpdates)
	
	EVT_MENU(mxID_VIEW_NOTEBOOK_PREV, mxMainWindow::OnViewNotebookPrev)
	EVT_MENU(mxID_VIEW_NOTEBOOK_NEXT, mxMainWindow::OnViewNotebookNext)

	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mxMainWindow::OnSelectError)
	EVT_TREE_SEL_CHANGED(wxID_ANY, mxMainWindow::OnSelectError)
	EVT_AUI_PANE_CLOSE(mxMainWindow::OnPaneClose)
	EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, mxMainWindow::OnNotebookPageClose)
	EVT_SOCKET(wxID_ANY,mxMainWindow::OnSocketEvent)
	EVT_COMMAND_SCROLL(wxID_ANY, mxMainWindow::OnScrollDegugSpeed)
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxMainWindow::OnLink)
END_EVENT_TABLE()

mxMainWindow::mxMainWindow(wxPoint pos, wxSize size) : wxFrame(NULL, wxID_ANY, _T("PSeInt"), pos, size, wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	wxTheColourDatabase->AddColour(_T("Z LIGHT GRAY"),wxColour(240,240,240));
	wxTheColourDatabase->AddColour(_T("Z LIGHT BLUE"),wxColour(240,240,255));
	wxTheColourDatabase->AddColour(_T("Z LIGHT RED"),wxColour(255,220,220));
	wxTheColourDatabase->AddColour(_T("DARK BLUE"),wxColour(0,0,128));
	wxTheColourDatabase->AddColour(_T("DARK RED"),wxColour(128,0,0));
	wxTheColourDatabase->AddColour(_T("DARK GRAY"),wxColour(128,128,128));
	wxTheColourDatabase->AddColour(_T("Z DARK GRAY"),wxColour(150,150,150));
	
	aui_manager.SetManagedWindow(this);
	
	for (int i=0;i<5;i++)
		file_history[i]=NULL;
	
//	SetIcon(wxIcon(icon_xpm));
	wxIconBundle bundle;
	wxIcon icon16; icon16.CopyFromBitmap(wxBitmap(_T("imgs/icon16.png"),wxBITMAP_TYPE_PNG));
	bundle.AddIcon(wxIcon(icon16));
	wxIcon icon32; icon32.CopyFromBitmap(wxBitmap(_T("imgs/icon32.png"),wxBITMAP_TYPE_PNG));
	bundle.AddIcon(wxIcon(icon32));
	wxIcon icon48; icon48.CopyFromBitmap(wxBitmap(_T("imgs/icon48.png"),wxBITMAP_TYPE_PNG));
	bundle.AddIcon(wxIcon(icon48));
	wxIcon icon64; icon64.CopyFromBitmap(wxBitmap(_T("imgs/icon64.png"),wxBITMAP_TYPE_PNG));
	bundle.AddIcon(wxIcon(icon64));
	wxIcon icon128; icon128.CopyFromBitmap(wxBitmap(_T("imgs/icon128.png"),wxBITMAP_TYPE_PNG));
	bundle.AddIcon(wxIcon(icon128));
	SetIcons(bundle);
	
	find_replace_dialog = new mxFindDialog(this,wxID_ANY);
	
	last_source = NULL;
	
	CreateMenus();
	CreateToolbars();
	CreateCommandsPanel();
	CreateDebugControlsPanel();
	CreateDesktopTestGrid();
	CreateNotebook();
	CreateResultsTree();
	CreateQuickHelp();
	CreateStatusBar(2,0);
	
	aui_manager.SetFlags(aui_manager.GetFlags() | wxAUI_MGR_TRANSPARENT_DRAG|wxAUI_MGR_LIVE_RESIZE);
	aui_manager.Update();

	SetDropTarget(new mxDropTarget());
	SetAccelerators();
	
	evaluate_window = new mxEvaluateDialog(this);
	debug = new DebugManager(desktop_test_grid,evaluate_window);
	
}

void mxMainWindow::OnFileSourceHistory (wxCommandEvent &event) {
	OpenProgram(config->last_files[event.GetId()-mxID_FILE_SOURCE_HISTORY_0]);
}

mxMainWindow::~mxMainWindow() {
	aui_manager.UnInit();
}

void mxMainWindow::CreateMenus() {
	wxMenuBar *menu = new wxMenuBar;
	
	wxMenu *file = new wxMenu;
	utils->AddItemToMenu(file,mxID_FILE_NEW, _T("&Nuevo\tCtrl+N"),_T(""),_T("nuevo.png"));
	utils->AddItemToMenu(file,mxID_FILE_OPEN, _T("&Abrir...\tCtrl+O"),_T(""),_T("abrir.png"));
	utils->AddItemToMenu(file,mxID_FILE_SAVE, _T("&Guardar\tCtrl+S"),_T(""),_T("guardar.png"));
	utils->AddItemToMenu(file,mxID_FILE_SAVE_AS, _T("Guardar &Como...\tCtrl+Shift+S"),_T(""),_T("guardar_como.png"));
	utils->AddItemToMenu(file,mxID_FILE_EDIT_FLOW, _T("Editar Diagrama de Flujo..."),_T(""),_T("edit_flow.png"));
	utils->AddItemToMenu(file,mxID_FILE_PRINT, _T("Imprimir..."),_T(""),_T("imprimir.png"));
	utils->AddItemToMenu(file,mxID_FILE_EXPORT_CPP, _T("Exportar a Cpp..."),_T(""),_T("cpp.png"));
	utils->AddItemToMenu(file,mxID_FILE_CLOSE, _T("&Cerrar...\tCtrl+W"),_T(""),_T("cerrar.png"));
	file->AppendSeparator();
	utils->AddItemToMenu(file,mxID_FILE_EXIT, _T("&Salir\tAlt+F4"),_T(""),_T("salir.png"));
	menu->Append(file, _T("&Archivo"));
	file_menu=file;
	RegenFileMenu(_T(""));
	
	wxMenu *edit = new wxMenu;
	utils->AddItemToMenu(edit,mxID_EDIT_UNDO, _T("&Deshacer\tCtrl+Z"),_T(""),_T("deshacer.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_REDO, _T("&Rehacer\tCtrl+Shift+Z"),_T(""),_T("rehacer.png"));
	edit->AppendSeparator();
	utils->AddItemToMenu(edit,mxID_EDIT_CUT, _T("C&ortar\tCtrl+X"),_T(""),_T("cortar.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_COPY, _T("&Copiar\tCtrl+C"),_T(""),_T("copiar.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_PASTE, _T("&Pegar\tCtrl+V"),_T(""),_T("pegar.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_TOGGLE_LINES_UP, _T("Mover Hacia Arriba\tCtrl+T"),_T("Mueve la o las lineas seleccionadas hacia arriba"),_T("toggleLinesUp.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_TOGGLE_LINES_DOWN, _T("Mover Hacia Abajo\tCtrl+Shift+T"),_T("Mueve la o las lineas seleccionadas hacia abajo"),_T("toggleLinesDown.png"));
	edit->AppendSeparator();
	utils->AddItemToMenu(edit,mxID_EDIT_FIND, _T("Buscar...\tCtrl+F"),_T(""),_T("buscar.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_FIND_PREV, _T("Buscar Anterior\tShift+F3"),_T(""),_T("anterior.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_FIND_NEXT, _T("Buscar Siguiente\tF3"),_T(""),_T("siguiente.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_REPLACE, _T("&Reemplazar...\tCtrl+R"),_T(""),_T("reemplazar.png"));
	edit->AppendSeparator();
	utils->AddItemToMenu(edit,mxID_EDIT_SELECT_ALL, _T("Seleccionar Todo\tCtrl+A"),_T(""),_T("seleccionar.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_DUPLICATE, _T("Duplicar Lineas\tCtrl+L"),_T(""),_T("duplicar.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_DELETE, _T("Eliminar Lineas\tCtrl+Shift+L"),_T(""),_T("eliminar.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_COMMENT, _T("Comentar Lineas\tCtrl+D"),_T(""),_T("comentar.png"));
	utils->AddItemToMenu(edit,mxID_EDIT_UNCOMMENT, _T("Descomentar Lineas\tCtrl+Shift+D"),_T(""),_T("descomentar.png"));
	edit->AppendSeparator();
	utils->AddItemToMenu(edit,mxID_EDIT_INDENT_SELECTION,_T("Corregir Indentado\tCtrl+I"),_T(""),_T("indentar.png"));
//	utils->AddItemToMenu(edit,mxID_EDIT_BEAUTIFY_CODE,_T("Emprolijar Algoritmo\tCtrl+Shift+I"),_T(""),_T("acomodar.png"));
	menu->Append(edit, _T("&Editar"));
	
	wxMenu *cfg = new wxMenu;
	mi_autocomp = utils->AddCheckToMenu(cfg,mxID_CONFIG_AUTOCOMP, _T("Utilizar Autocompletado"),_T(""),config->autocomp);
	mi_calltip_helps = utils->AddCheckToMenu(cfg,mxID_CONFIG_CALLTIP_HELPS, _T("Utilizar Ayudas Emergentes"),_T(""),config->calltip_helps);
	mi_smart_indent = utils->AddCheckToMenu(cfg,mxID_CONFIG_SMART_INDENT, _T("Utilizar Indentado Inteligente"),_T(""),config->smart_indent);
	
	mi_quickhelp = utils->AddCheckToMenu(cfg,mxID_CONFIG_SHOW_QUICKHELP, _T("Mostrar Ayuda Rapida"),_T(""),config->auto_quickhelp);
//	mi_results = utils->AddCheckToMenu(cfg,mxID_CONFIG_SHOW_RESULTS, _T("Mostrar Errores"),_T(""),false);
	mi_toolbar = utils->AddCheckToMenu(cfg,mxID_CONFIG_SHOW_TOOLBAR, _T("Mostrar Barra de Herramientas"),_T(""),config->show_toolbar);
//	mi_colour = utils->AddCheckToMenu(cfg,mxID_CONFIG_COLOUR_SINTAX, _T("Colorear Sintaxis"),_T(""),config->colour_sintax);
	mi_commands = utils->AddCheckToMenu(cfg,mxID_CONFIG_SHOW_COMMANDS, _T("Mostrar Panel de Comandos"),_T(""),config->show_commands);
	mi_debug_panel = utils->AddCheckToMenu(cfg,mxID_CONFIG_SHOW_DEBUG_PANEL, _T("Mostrar Panel de Ejecucion Paso a Paso"),_T(""),config->show_debug_panel);
	cfg->AppendSeparator();
//	mi_high_res = utils->AddCheckToMenu(cfg,mxID_CONFIG_HIGHRES, _T("Diagramas de Flujo de Alta Resolucion"),_T(""),config->high_res_flows);
	mi_use_colors = utils->AddCheckToMenu(cfg,mxID_CONFIG_USE_COLORS, _T("Utilizar colores al interpretar"),_T(""),config->use_colors);
	cfg->AppendSeparator();
	utils->AddItemToMenu(cfg,mxID_CONFIG_LANGUAGE, _T("Opciones del Lenguaje..."),_T(""),_T("lenguaje.png"));
	cfg->AppendSeparator();
	mi_stepstep_l = utils->AddCheckToMenu(cfg,mxID_CONFIG_STEPSTEP_L, _T("Velocidad del paso a paso: Baja"),_T(""),config->stepstep_speed==0);
	mi_stepstep_m = utils->AddCheckToMenu(cfg,mxID_CONFIG_STEPSTEP_M, _T("Velocidad del paso a paso: Media"),_T(""),config->stepstep_speed==1);
	mi_stepstep_h = utils->AddCheckToMenu(cfg,mxID_CONFIG_STEPSTEP_H, _T("Velocidad del paso a paso: Alta"),_T(""),config->stepstep_speed==2);
	menu->Append(cfg, _T("&Configurar"));
	
	wxMenu *run = new wxMenu;
	utils->AddItemToMenu(run,mxID_RUN_RUN, _T("Ejecutar\tF9"),_T(""),_T("ejecutar.png"));
	utils->AddItemToMenu(run,mxID_RUN_STEP_STEP, _T("Ejecutar Paso a Paso\tF5"),_T(""),_T("pasos.png"));
	utils->AddItemToMenu(run,mxID_RUN_CHECK, _T("Verificar Sintaxis\tShift+F9"),_T(""),_T("verificar.png"));
	utils->AddItemToMenu(run,mxID_RUN_DRAW_FLOW, _T("Dibujar Diagrama de Flujo\tF7"),_T(""),_T("flujo.png"));
	utils->AddItemToMenu(run,mxID_RUN_SAVE_FLOW, _T("Guardar Diagrama de Flujo\tShift+F7"),_T(""),_T("guardar.png"));
	utils->AddItemToMenu(run,mxID_RUN_SET_INPUT, _T("Predefinir Entrada...\tCtrl+F9"),_T(""),_T("input.png"));
	menu->Append(run, _T("E&jecutar"));
	
//	wxMenu *config = new wxMenu;
//	utils->AddItemToMenu(config,mxID_CONFIG_, _T(""),_T(""),_T(".png"));
//	menu->Append(config, _T("&Configuar"));
//	
	wxMenu *help = new wxMenu;
	utils->AddItemToMenu(help,mxID_HELP_INDEX, _T("Indice...\tF1"),_T(""),_T("ayuda.png"));
//	utils->AddItemToMenu(help,mxID_HELP_REFERENCE, _T("Referencia..."),_T(""),_T("referencia.png"));
	utils->AddItemToMenu(help,mxID_HELP_QUICKHELP, _T("Ayuda Rapida\tShift+F1"),_T(""),_T("referencia.png"));
	utils->AddItemToMenu(help,mxID_HELP_EXAMPLES, _T("&Ejemplos..."),_T(""),_T("abrir.png"));
	help->AppendSeparator();
	utils->AddItemToMenu(help,mxID_HELP_UPDATES, _T("&Buscar actualizaciones...\t"),_T("Comprueba a traves de Internet si hay versiones mas recientes de PSeInt disponibles..."),_T("updates.png"));
	utils->AddItemToMenu(help,mxID_HELP_ABOUT, _T("Acerca de..."),_T(""),_T("acerca_de.png"));
	menu->Append(help, _T("A&yuda"));
	
	SetMenuBar(menu);
	
}

void mxMainWindow::CreateToolbars() {
	
	toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	toolbar->SetToolBitmapSize(wxSize(24,24));
	utils->AddTool(toolbar,mxID_FILE_NEW,_T("Nuevo"),_T("nuevo.png"),_T(""));
	utils->AddTool(toolbar,mxID_FILE_OPEN,_T("Abrir..."),_T("abrir.png"),_T(""));
	utils->AddTool(toolbar,mxID_FILE_SAVE,_T("Guardar"),_T("guardar.png"),_T(""));
	utils->AddTool(toolbar,mxID_FILE_SAVE_AS,_T("Guardar Como..."),_T("guardar_como.png"),_T(""));
	toolbar->AddSeparator();
	utils->AddTool(toolbar,mxID_EDIT_UNDO,_T("Deshacer"),_T("deshacer.png"),_T(""));
	utils->AddTool(toolbar,mxID_EDIT_REDO,_T("Rehacer"),_T("rehacer.png"),_T(""));
	utils->AddTool(toolbar,mxID_EDIT_CUT,_T("Cortar"),_T("cortar.png"),_T(""));
	utils->AddTool(toolbar,mxID_EDIT_COPY,_T("Copiar"),_T("copiar.png"),_T(""));
	utils->AddTool(toolbar,mxID_EDIT_PASTE,_T("Pegar"),_T("pegar.png"),_T(""));
	utils->AddTool(toolbar,mxID_EDIT_INDENT_SELECTION,_T("Corregir Indentado"),_T("indentar.png"),_T(""));
	toolbar->AddSeparator();
	utils->AddTool(toolbar,mxID_EDIT_FIND,_T("Buscar"),_T("buscar.png"),_T(""));
	utils->AddTool(toolbar,mxID_EDIT_FIND_PREV,_T("Buscar Anterior"),_T("anterior.png"),_T(""));
	utils->AddTool(toolbar,mxID_EDIT_FIND_NEXT,_T("Buscar Siguiente"),_T("siguiente.png"),_T(""));
	utils->AddTool(toolbar,mxID_EDIT_REPLACE,_T("Reemplazar"),_T("reemplazar.png"),_T(""));
	toolbar->AddSeparator();
	utils->AddTool(toolbar,mxID_RUN_RUN,_T("Ejecutar..."),_T("ejecutar.png"),_T(""));
	utils->AddTool(toolbar,mxID_RUN_STEP_STEP,_T("Ejecutar paso a paso..."),_T("pasos.png"),_T(""));
	utils->AddTool(toolbar,mxID_FILE_EDIT_FLOW,_T("Dibujar Diagrama de Flujo..."),_T("flujo.png"),_T(""));
	toolbar->AddSeparator();
	utils->AddTool(toolbar,mxID_HELP_INDEX,_T("Ayuda..."),_T("ayuda.png"),_T(""));
//	utils->AddTool(toolbar,mxID_FILE_EXIT,_T("Salir"),_T("salir.png"),_T(""));
	toolbar->Realize();
	if (config->show_toolbar)
		aui_manager.AddPane(toolbar, wxAuiPaneInfo().Name(_T("toolbar")).Caption(_T("Toolbar")).ToolbarPane().Top().Position(0).LeftDockable(false).RightDockable(false).Show());
	else
		aui_manager.AddPane(toolbar, wxAuiPaneInfo().Name(_T("toolbar")).Caption(_T("Toolbar")).ToolbarPane().Top().Position(0).LeftDockable(false).RightDockable(false).Hide());
	
}

void mxMainWindow::CreateCommandsPanel() {
	wxPanel *panel = commands = new wxPanel(this);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	utils->AddImgButton(sizer,panel,mxID_CMD_ESCRIBIR,_T("escribir.png"),_T("Escribir"));
	utils->AddImgButton(sizer,panel,mxID_CMD_LEER,_T("leer.png"),_T("Leer"));
	utils->AddImgButton(sizer,panel,mxID_CMD_ASIGNAR,_T("asignar.png"),_T("Asignar"));
	utils->AddImgButton(sizer,panel,mxID_CMD_SI,_T("si.png"),_T("Si-Entonces"));
	utils->AddImgButton(sizer,panel,mxID_CMD_SEGUN,_T("segun.png"),_T("Segun"));
	utils->AddImgButton(sizer,panel,mxID_CMD_MIENTRAS,_T("mientras.png"),_T("Mientras"));
	utils->AddImgButton(sizer,panel,mxID_CMD_REPETIR,_T("repetir.png"),_T("Repetir"));
	utils->AddImgButton(sizer,panel,mxID_CMD_PARA,_T("para.png"),_T("Para"));

	panel->SetSizerAndFit(sizer);
	if (config->show_commands)
		aui_manager.AddPane(panel, wxAuiPaneInfo().Name(_T("commands")).Caption(_T("Comandos")).Right().Show().Row(2));
	else
		aui_manager.AddPane(panel, wxAuiPaneInfo().Name(_T("commands")).Caption(_T("Comandos")).Right().Hide().Row(2));
}

void mxMainWindow::CreateDebugControlsPanel() {
	wxPanel *panel = debug_panel = new wxPanel(this);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(new wxStaticText(panel,wxID_ANY,_T(" Estado:")),wxSizerFlags().Proportion(0).Expand().Border(wxTOP,10));
	debug_status = new wxStaticText(panel,wxID_ANY,_T("No iniciada"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
	sizer->Add(debug_status,wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(dp_button_run = new wxButton(panel,mxID_DEBUG_BUTTON,_T("Comenzar")),wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(dp_button_pause = new wxButton(panel,mxID_DEBUG_PAUSE,_T("Pausar/Continuar")),wxSizerFlags().Proportion(0).Expand());
	sizer->Add(dp_button_step = new wxButton(panel,mxID_DEBUG_STEP,_T("Primer Paso")),wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(dp_button_desktop_vars = new wxButton(panel,mxID_DEBUG_DESKTOP_VARS,_T("Prueba de Esc...")),wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(dp_button_evaluate = new wxButton(panel,mxID_DEBUG_EVALUATE,_T("Evaluar...")),wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(new wxStaticText(panel,wxID_ANY,_T(" Velocidad:")),wxSizerFlags().Proportion(0).Expand().Border(wxTOP,10));
	debug_speed=new wxScrollBar(panel,mxID_DEBUG_SLIDER);
	debug_speed->SetScrollbar(0,1,100,10);
	sizer->Add(debug_speed,wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(new wxButton(panel,mxID_DEBUG_HELP,_T("Ayuda...")),wxSizerFlags().Proportion(0).Expand().Border(wxTOP,10));
	dp_button_pause->Disable();
	dp_button_evaluate->Disable();

	panel->SetSizerAndFit(sizer);
	if (config->show_debug_panel)
		aui_manager.AddPane(panel, wxAuiPaneInfo().Name(_T("debug_panel")).Caption(_T("Paso a paso")).Right().Show());
	else
		aui_manager.AddPane(panel, wxAuiPaneInfo().Name(_T("debug_panel")).Caption(_T("Paso a paso")).Right().Hide());
}

void mxMainWindow::CreateNotebook() {
	wxSize client_size = GetClientSize();
	notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(client_size.x, client_size.y), wxSize(430,200), wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER | wxAUI_NB_WINDOWLIST_BUTTON);
//	wxBitmap page_bmp = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16));
	aui_manager.AddPane(notebook, wxAuiPaneInfo().Name(_T("notebook_sources")).CenterPane().PaneBorder(false));
}

wxStatusBar* mxMainWindow::OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name) {
	status_bar = new wxStatusBar(this, id, style, name);
	int sz[2]={-75,100};
	status_bar->SetFieldsCount(number,sz);
	
	wxTextFile fil(_T("version"));
	if (fil.Exists()) {
		fil.Open();
		status_bar->SetStatusText(wxString(_T(" v"))<<fil.GetFirstLine(),1);
		fil.Close();
	} else {
		status_bar->SetStatusText(_T("version desconocida"),1);
	}
	return status_bar;
}

mxSource *mxMainWindow::NewProgram() {
	mxSource *source = new mxSource(notebook);
	notebook->AddPage(source,_T("<sin_titulo>"),true);
	source->SetText(_T("Proceso sin_titulo\n\t\nFinProceso\n"));
	source->SetIndicator(1,8,18);
	source->SetSelection(20,20);
	source->SetModify(false);
	source->SetFocus();
	return source;
}


void mxMainWindow::OnFileNew(wxCommandEvent &evt) {
	NewProgram();
}

void mxMainWindow::OnFileExportCpp(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		bool mod = source->GetModify();
		source->SaveFile(config->temp_file);
		source->SetModify(mod);
		if (debug->debugging)
			debug->Stop();
		else
			(new mxProcess(source,notebook->GetPageText(notebook->GetSelection())))->ExportCpp(config->temp_file,true);
	}	
}

void mxMainWindow::OnFileClose(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		if (source->GetModify()) {
			int res=wxMessageBox(_T("Hay cambios sin guardar. Desea guardarlos antes de cerrar?"), source->filename, wxYES_NO|wxCANCEL,this);
			if (res==wxCANCEL)
				return;
			else if (res==wxYES) {
				if (source->sin_titulo) {
					OnFileSaveAs(evt);
					if (source->GetModify())
						return;
				} else
					source->SaveFile(source->filename);
			}
		}
		notebook->DeletePage(notebook->GetSelection());
	}
}


void mxMainWindow::OnFileOpen(wxCommandEvent &evt) {
	wxFileDialog dlg (this, _T("Abrir Archivo"), config->last_dir, _T(" "), _T("Any file (*)|*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	dlg.SetWildcard(_T("Todos los archivos|*|Algoritmos en pseudocodigo|*.psc;*.PSC|Archivos de texto|*.txt;*.TXT"));
	if (dlg.ShowModal() == wxID_OK) {
		config->last_dir=dlg.GetDirectory();
		wxArrayString paths;
		dlg.GetPaths(paths);
		for (unsigned int i=0;i<paths.GetCount();i++)
			OpenProgram(paths[i]);
	}
}

void mxMainWindow::OnHelpExamples(wxCommandEvent &evt) {
	if (!helpw) helpw = new mxHelpWindow();
	helpw->ShowHelp(_T("ejemplos.html"));
//	wxFileDialog dlg (this, _T("Abrir Archivo"), DIR_PLUS_FILE(wxGetCwd(),	config->examples_dir), _T(""), _T("Ejemplos|*.psc"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
//	if (dlg.ShowModal() == wxID_OK) {
//		wxArrayString paths;
//		dlg.GetPaths(paths);
//		for (unsigned int i=0;i<paths.GetCount();i++)
//			OpenProgram(paths[i],false)->SetReadOnly(true);
//	}
}

void mxMainWindow::RegenFileMenu(wxString path) {
	int i=0, c = config->last_files.GetCount();
	if (path.Len()) {
		if (c) {
			for (i=0;i<c;i++)
				if (config->last_files[i]==path)
					break;
			if (i==c) {
				if (c==5) {
					for (int i=4;i>0;i--)
						config->last_files[i]=config->last_files[i-1];
				} else {
					config->last_files.Add(_T(""));
					for (int i=c;i>0;i--)
						config->last_files[i]=config->last_files[i-1];
					c++;
				}
			} else if (i) {
				if (i!=4) for (int j=i;j>0;j--)
					config->last_files[j]=config->last_files[j-1];
			}
			config->last_files[0]=path;
		} else {
			config->last_files.Add(path);
			c++;
		}
	}
		
	if (!file_history[0] && c)
		file_menu->AppendSeparator();
	
	for (i=0;i<c;i++) {
		if (file_history[i])
			file_menu->Remove(file_history[i]);
		if (config->last_files[i][0])
			file_history[i] = utils->AddItemToMenu(file_menu, mxID_FILE_SOURCE_HISTORY_0+i,config->last_files[i],config->last_files[i],wxString(_T("recent"))<<i+1<<_T(".png"));
	}
}

mxSource *mxMainWindow::OpenProgram(wxString path, bool history) {
	
	if (!wxFileName::FileExists(path)) {
		wxMessageBox(wxString(_T("No se pudo abrir el archivo "))<<path,_T("Error"));
		return NULL;
	}
	
	if (history) 
		RegenFileMenu(path);
	
	mxSource *source = new mxSource(notebook,path);
	notebook->AddPage(source,wxFileName(path).GetFullName(),true);
	source->LoadFile(path);
	return source;
}

void mxMainWindow::OnFileSave(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		if (source->sin_titulo)
			OnFileSaveAs(evt);
		else
			source->SaveFile(source->filename);
	}
}

void mxMainWindow::OnFileSaveAs(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		wxFileDialog dlg (this, _T("Guardar"),source->sin_titulo?config->last_dir:wxFileName(source->filename).GetPath(),source->sin_titulo?wxString(wxEmptyString):wxFileName(source->filename).GetFullName(), _T("Any file (*)|*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		dlg.SetWildcard(_T("Todos los archivos|*|Algoritmos en pseudocodigo|*.psc;*.PSC|Archivos de texto|*.txt;*.TXT"));
		if (dlg.ShowModal() == wxID_OK) {
			wxFileName file(dlg.GetPath());
			if (file.GetExt().Len()==0) file.SetExt(_T("psc"));
			source->UnExample();
			config->last_dir=dlg.GetDirectory();
			source->SaveFile(file.GetFullPath());
			source->SetFileName(file.GetFullPath());
			notebook->SetPageText(notebook->GetSelection(),file.GetFullName());
			RegenFileMenu(file.GetFullPath());
		}
	}	
}

void mxMainWindow::OnFileExit(wxCommandEvent &evt) {
	Close(true);
}

void mxMainWindow::OnEdit(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE 
		CURRENT_SOURCE->ProcessEvent(evt);
}

void mxMainWindow::OnRunRun(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		bool mod = source->GetModify();
		source->SaveFile(config->temp_file);
		source->SetModify(mod);
		if (debug->debugging)
			debug->Stop();
		else
			(new mxProcess(source,notebook->GetPageText(notebook->GetSelection())))->Run(config->temp_file, true);
	}
}

void mxMainWindow::OnRunStepStep(wxCommandEvent &evt) {
	if (debug->debugging)
		debug->Stop();
	else IF_THERE_IS_SOURCE
		StartDebugging(CURRENT_SOURCE,false);
}

void mxMainWindow::OnDebugButton(wxCommandEvent &evt) {
	OnRunStepStep(evt);
}

void mxMainWindow::OnDebugPause(wxCommandEvent &evt) {
	if (ds_state==DS_STEP) return;
	debug->Pause();
//	if (debug->Pause())
//		SetDebugState(DS_PAUSED);
//	else
//		SetDebugState(DS_RESUMED);
}

void mxMainWindow::OnDebugStep(wxCommandEvent &evt) {
	if (ds_state==DS_STEP) return;
	if (debug->debugging)
		debug->Step();
	else IF_THERE_IS_SOURCE
		StartDebugging(CURRENT_SOURCE,true);
}

void mxMainWindow::StartDebugging(mxSource *source, bool paused) {
	bool mod = source->GetModify();
	source->SaveFile(config->temp_file);
	source->SetModify(mod);
	debug->should_pause = paused;
	if ( (new mxProcess(source,notebook->GetPageText(notebook->GetSelection())))->Debug(config->temp_file, true) )
		SetDebugState(DS_STARTING);
}

void mxMainWindow::OnRunCheck(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		bool mod = source->GetModify();
		source->SaveFile(config->temp_file);
		source->SetModify(mod);
		if (debug->debugging)
			debug->Stop();
		else
			(new mxProcess(source,notebook->GetPageText(notebook->GetSelection())))->CheckSyntax(config->temp_file);
	}
}

void mxMainWindow::OnRunDrawFlow(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		bool mod = source->GetModify();
		source->SaveFile(config->temp_file);
		source->SetModify(mod);
		if (debug->debugging)
			debug->Stop();
		else
			(new mxProcess(source,notebook->GetPageText(notebook->GetSelection())))->Draw(config->temp_file,true);
	}
}

void mxMainWindow::OnRunSaveFlow(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		bool mod = source->GetModify();
		source->SaveFile(config->temp_file);
		source->SetModify(mod);
		if (debug->debugging)
			debug->Stop();
		else
			(new mxProcess(source,notebook->GetPageText(notebook->GetSelection())))->SaveDraw(config->temp_file,true);
	}		
}

void mxMainWindow::OnClose(wxCloseEvent &evt) {
	
	IF_THERE_IS_SOURCE {
		for (int i=notebook->GetPageCount()-1;i>=0;i--) {
			mxSource *source = (mxSource*)(notebook->GetPage(i));
			if (source->GetModify()) {
				notebook->SetSelection(i);
				int res=wxMessageBox(_T("Hay cambios sin guardar. Desea guardarlos antes de salir?"), source->filename, wxYES_NO|wxCANCEL,this);
				if (res&wxYES) {
					if (!source->sin_titulo)
						source->SaveFile(source->filename);
					else {
						wxCommandEvent evt;
						OnFileSaveAs(evt);
						if (source->GetModify())
							return;
					}
				}
				if (res&wxCANCEL) {
					return;
				}
			}
		}
	}
	
	/** @todo: ver porque esto rompe al x (pid incorrecto?) **/
	if (proc_for_killing) delete proc_for_killing;
	proc_for_killing=NULL;
	mxProcess *proc=proc_list;
	while (proc) {
		mxProcess *nproc=proc->next;
		if (proc_list->pid) 
			proc->Kill(proc_list->pid,wxSIGKILL);
//		proc->Destroy();
		proc=nproc;
	}
	if (proc_for_killing) delete proc_for_killing;
	
	config->show_debug_panel = aui_manager.GetPane(debug_panel).IsShown();
	config->show_commands = aui_manager.GetPane(commands).IsShown();
	config->show_toolbar = aui_manager.GetPane(toolbar).IsShown();
	if (IsMaximized()) {
		config->maximized=true;
	} else if (!IsIconized()) { 
		config->pos_x=GetPosition().x;
		config->pos_y=GetPosition().y;
		config->size_x=GetSize().GetWidth();
		config->size_y=GetSize().GetHeight();
		config->maximized=false;
	}	
	config->Save();
	delete find_replace_dialog;
	delete help;
	delete debug;
	delete config;
	wxExit();
}

void mxMainWindow::CreateResultsTree() {

	results_tree = new wxTreeCtrl(this, wxID_ANY, wxPoint(0,0), wxSize(160,250), wxTR_DEFAULT_STYLE | wxNO_BORDER );
	wxImageList* imglist = new wxImageList(16, 16, true, 2);
	imglist->Add(wxBitmap(DIR_PLUS_FILE(config->images_path,_T("tree.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(DIR_PLUS_FILE(config->images_path,_T("error.png")),wxBITMAP_TYPE_PNG));
	results_tree->AssignImageList(imglist);
	results_root = results_tree->AddRoot(wxString(_T("PSeInt "))<<VERSION, 0);
	aui_manager.AddPane(results_tree, wxAuiPaneInfo().Name(_T("symbols_tree")).Caption(_T("Resultados")).Bottom().CloseButton(true).MaximizeButton(true).Hide().Position(0));	
	
}

void mxMainWindow::ShowResults(bool error) {
	if (!aui_manager.GetPane(results_tree).IsShown()) {
		if (!error && aui_manager.GetPane(quick_html).IsShown())
			aui_manager.GetPane(quick_html).Hide();
		aui_manager.GetPane(results_tree).Hide();
		aui_manager.GetPane(results_tree).Show();
//		compiler_tree.menuItem->Check(true);
		aui_manager.Update();
	} else if (!error && aui_manager.GetPane(quick_html).IsShown()) {
		aui_manager.GetPane(quick_html).Hide();
		aui_manager.Update();
	}
	results_tree->Expand(results_root);
}

void mxMainWindow::OnHelpQuickHelp(wxCommandEvent &evt) {
	
	if (!aui_manager.GetPane(quick_html).IsShown()) {
		aui_manager.GetPane(quick_html).Show();
		aui_manager.Update();
	} 
	
	bool ask = true; // preguntar si no hay fuente abierto o palabra seleccionada
	wxString key;
	IF_THERE_IS_SOURCE { // si hay fuente abierto
		mxSource *source=CURRENT_SOURCE; 
		int pos=source->GetCurrentPos(); // buscar la palabra sobre el cursor
		int s=source->WordStartPosition(pos,true);
		int e=source->WordEndPosition(pos,true);
		key = source->GetTextRange(s,e);
		if (key.Len()!=0) { // puede ser una directiva de preprocesador
			ask=false;
		}
	}
	if (ask) // si no hay clave, preguntar
		key = wxGetTextFromUser(_T("Palabra a buscar:"), _T("Ayuda Rapida") , _(""), this);
	
	quick_html->LoadPage(help->GetQuickHelp(key));
}

void mxMainWindow::ShowQuickHelp(wxString text) {
	if (!aui_manager.GetPane(quick_html).IsShown()) {
		quick_html->SetPage(text);
		aui_manager.GetPane(quick_html).Show();
		aui_manager.Update();
	} else
		quick_html->SetPage(text);
}

void mxMainWindow::HideQuickHelp() {
	if (aui_manager.GetPane(quick_html).IsShown()) {
		aui_manager.GetPane(quick_html).Hide();
		aui_manager.Update();
	} 
}

void mxMainWindow::CreateQuickHelp() {
	quick_html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(400,300));
	quick_html->SetPage(wxString(_T("PSeInt "))<<VERSION);
	aui_manager.AddPane(quick_html, wxAuiPaneInfo().Name(_T("quick_html")).Caption(_T("Ayuda Rapida")).Bottom().CloseButton(true).MaximizeButton(true).Hide().Position(1));	
}

void mxMainWindow::OnSelectError(wxTreeEvent &evt) {
	int index = notebook->GetPageIndex(last_source);
	if (index==wxNOT_FOUND) return;
	notebook->SetSelection(index);
	wxString text = results_tree->GetItemText(evt.GetItem());
	if (text.SubString(0,5)==_T("Linea ")) {
		long l;
		text.AfterFirst(' ').BeforeFirst(':').ToLong(&l);
		l--;
		last_source->SetSelection(last_source->GetLineIndentPosition(l),last_source->GetLineEndPosition(l));
		last_source->SetFocus();
		text = text.AfterFirst(':');
		if (text.SubString(0,6)==_T(" ERROR ")) {
			long e=0;
			text.Mid(7).ToLong(&e);
			if (config->auto_quickhelp) 
				ShowQuickHelp(help->GetErrorText(text,e));
		}
	}
}

void mxMainWindow::OnCmdAsignar(wxCommandEvent &evt) {
	if (config->auto_quickhelp) 
		ShowQuickHelp(help->GetCommandText(_T("ASIGNAR")));
	wxArrayString toins;
	toins.Add(_T("{variable}<-{expresion};"));
	InsertCode(toins);
}

void mxMainWindow::OnCmdLeer(wxCommandEvent &evt) {
	if (config->auto_quickhelp) 
		ShowQuickHelp(help->GetCommandText(_T("LEER")));
	wxArrayString toins;
	toins.Add(_T("Leer {lista_de_variables};"));
	InsertCode(toins);
}

void mxMainWindow::OnCmdEscribir(wxCommandEvent &evt) {
	if (config->auto_quickhelp) 
		ShowQuickHelp(help->GetCommandText(_T("ESCRIBIR")));
	wxArrayString toins;
	toins.Add(_T("Escribir {lista_de_expresiones};"));
	InsertCode(toins);
}

void mxMainWindow::OnCmdMientras(wxCommandEvent &evt) {
	if (config->auto_quickhelp) 
		ShowQuickHelp(help->GetCommandText(_T("MIENTRAS")));
	wxArrayString toins;
	toins.Add(_T("Mientras {expresion_logica} Hacer"));
	toins.Add(_T("\t{secuencia_de_acciones}"));
	toins.Add(_T("FinMientras"));
	InsertCode(toins);
}

void mxMainWindow::OnCmdRepetir(wxCommandEvent &evt) {
	if (config->auto_quickhelp) 
		ShowQuickHelp(help->GetCommandText(_T("REPETIR")));
	wxArrayString toins;
	toins.Add(_T("Repetir"));
	toins.Add(_T("\t{secuencia_de_acciones}"));
	toins.Add(_T("Hasta Que {expresion_logica}"));
	InsertCode(toins);
}

void mxMainWindow::OnCmdPara(wxCommandEvent &evt) {
	if (config->auto_quickhelp) 
		ShowQuickHelp(help->GetCommandText(_T("PARA")));
	wxArrayString toins;
	toins.Add(_T("Para {variable_numerica}<-{valor_inicial} Hasta {valor_final} Con Paso {paso} Hacer"));
	toins.Add(_T("\t{secuencia_de_acciones}"));
	toins.Add(_T("FinPara"));
	InsertCode(toins);
}

void mxMainWindow::OnCmdSi(wxCommandEvent &evt) {
	if (config->auto_quickhelp) 
		ShowQuickHelp(help->GetCommandText(_T("SI")));
	wxArrayString toins;
	toins.Add(_T("Si {expresion_logica} Entonces"));
	toins.Add(_T("\t{acciones_por_verdadero}"));
	toins.Add(_T("Sino"));
	toins.Add(_T("\t{acciones_por_falso}"));
	toins.Add(_T("FinSi"));
	InsertCode(toins);
}

void mxMainWindow::OnCmdSegun(wxCommandEvent &evt) {
	if (config->auto_quickhelp) 
		ShowQuickHelp(help->GetCommandText(_T("SEGUN")));
	wxArrayString toins;
	toins.Add(_T("Segun {variable_numerica} Hacer"));
	toins.Add(_T("\t{opcion_1}:"));
	toins.Add(_T("\t\t{secuencia_de_acciones_1}"));
	toins.Add(_T("\t{opcion_2}:"));
	toins.Add(_T("\t\t{secuencia_de_acciones_2}"));
	toins.Add(_T("\t{opcion_3}:"));
	toins.Add(_T("\t\t{secuencia_de_acciones_3}"));
	toins.Add(_T("\tDe Otro Modo:"));
	toins.Add(_T("\t\t{secuencia_de_acciones_dom}"));
	toins.Add(_T("FinSegun"));
	InsertCode(toins);
}	

void mxMainWindow::InsertCode(wxArrayString &toins) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		if (source->GetReadOnly()) return source->MessageReadOnly();
		int ss = source->GetSelectionStart(), se = source->GetSelectionEnd();
		if (ss!=se) {
			source->SetTargetStart(ss);
			source->SetTargetEnd(se);
			source->ReplaceTarget(_T(""));
		}
		int line = source->LineFromPosition(ss);
		int indent = source->GetLineIndentation(line), oline=line;
		for (unsigned int i=0;i<toins.GetCount();i++) {
			if (i) 
				source->InsertText(source->PositionFromLine(line),_T("\n"));
			source->SetLineIndentation(line,indent);
			int pos = source->GetLineIndentPosition(line);
			wxString toindic = toins[i];
			for (int j=toindic.size()-1;j>=0;j--) {
				if (toindic[j]=='{' || toindic[j]=='}')
					toins[i] = toins[i].SubString(0,j-1)+toins[i].Mid(j+1);
			}
			source->InsertText(pos,toins[i]);
			int p1=-1,p2=-2, des=0;
			for (unsigned int j=0;j<toindic.size();j++) {
				if (toindic[j]=='{' && p1==-1) {
					p1 = j+pos-des;
					des++;
				} else if (toindic[j]=='}' && p1!=-1) {
					p2 = j+pos-des;
					des++;
					source->SetIndicator(1,p1,p2);
					p1 = -1;
				}
			}
			line++;
		}
		source->SetSelectionStart(source->GetLineIndentPosition(oline));
		source->SetSelectionEnd(source->GetLineEndPosition(line-1));
		source->SetFocus();
	}
}
void mxMainWindow::OnEditFind (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		find_replace_dialog->ShowFind(CURRENT_SOURCE);
	} else
		find_replace_dialog->ShowFind(NULL);
	return;
}

void mxMainWindow::OnEditFindNext (wxCommandEvent &event) {
	if (find_replace_dialog->last_search.Len()) {
		if (!find_replace_dialog->FindNext())
			wxMessageBox(wxString(_T("La cadena \""))<<find_replace_dialog->last_search<<_T("\" no se encontro."), _T("Buscar"));
	} else {
		OnEditFind(event);
	}
}

void mxMainWindow::OnEditFindPrev (wxCommandEvent &event) {
	if (find_replace_dialog->last_search.Len()) {
		if (!find_replace_dialog->FindPrev())
			wxMessageBox(wxString(_T("La cadena \""))<<find_replace_dialog->last_search<<_T("\" no se encontro."), _T("Buscar"));
	} else {
		OnEditFind(event);
	}
}

void mxMainWindow::OnEditReplace (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		find_replace_dialog->ShowReplace(CURRENT_SOURCE);
	} else
		find_replace_dialog->ShowReplace(NULL);
	return;
}

void mxMainWindow::OnHelpAbout(wxCommandEvent &evt) {
	// cppcheck-suppress unusedScopedObject
	mxAboutWindow(this,wxID_ANY);
}

void mxMainWindow::OnHelpIndex(wxCommandEvent &evt) {
	if (helpw) {
		helpw->ShowIndex();
		if (helpw->IsIconized())
			helpw->Maximize(false);
		else
			helpw->Raise();
	} else
		helpw = new mxHelpWindow();
}

void mxMainWindow::OnConfigShowToolbar(wxCommandEvent &evt) {
	if (!mi_toolbar->IsChecked()) {
		mi_toolbar->Check(false);
		aui_manager.GetPane(toolbar).Hide();
	} else {
		mi_toolbar->Check(true);
		aui_manager.GetPane(toolbar).Show();
	}
	aui_manager.Update();	
}
	
void mxMainWindow::OnConfigShowCommands(wxCommandEvent &evt) {
	if (!mi_commands->IsChecked()) {
		mi_commands->Check(false);
		aui_manager.GetPane(commands).Hide();
	} else {
		mi_commands->Check(true);
		aui_manager.GetPane(commands).Show();
	}
	aui_manager.Update();	
}
	
void mxMainWindow::OnConfigShowDebugPanel(wxCommandEvent &evt) {
	if (!mi_debug_panel->IsChecked()) {
		mi_debug_panel->Check(false);
		aui_manager.GetPane(debug_panel).Hide();
	} else {
		mi_debug_panel->Check(true);
		aui_manager.GetPane(debug_panel).Show();
	}
	aui_manager.Update();	
}
	
void mxMainWindow::OnConfigShowQuickHelp(wxCommandEvent &evt) {
	if (!mi_quickhelp->IsChecked()) {
		mi_quickhelp->Check(false);
		config->auto_quickhelp = false;
	} else {
		mi_quickhelp->Check(true);
		config->auto_quickhelp = true;
	}
}
	
//void mxMainWindow::OnConfigShowResults(wxCommandEvent &evt) {
//	if (!mi_results->IsChecked()) {
//		mi_results->Check(false);
//		aui_manager.GetPane(toolbar).Hide();
//	} else {
//		mi_results->Check(true);
//		aui_manager.GetPane(toolbar).Show();
//	}
//	aui_manager.Update();	
// }
	
//void mxMainWindow::OnConfigColoutSintax(wxCommandEvent &evt) {
//	if (!mi_toolbar->IsChecked()) {
//		mi_toolbar->Check(false);
//		config->colour_sintax=false;
//	} else {
//		mi_toolbar->Check(true);
//		config->colour_sintax=true;
//	}
// }

void mxMainWindow::OnConfigSmartIndent(wxCommandEvent &evt) {
	if (!mi_smart_indent->IsChecked()) {
		mi_smart_indent->Check(false);
		config->smart_indent=false;
	} else {
		mi_smart_indent->Check(true);
		config->smart_indent=true;
	}
}

void mxMainWindow::OnConfigCalltipHelps(wxCommandEvent &evt) {
	if (!mi_calltip_helps->IsChecked()) {
		mi_calltip_helps->Check(false);
		config->calltip_helps=false;
	} else {
		mi_calltip_helps->Check(true);
		config->calltip_helps=true;
	}
}

void mxMainWindow::OnConfigAutoComp(wxCommandEvent &evt) {
	if (!mi_autocomp->IsChecked()) {
		mi_autocomp->Check(false);
		config->autocomp=false;
	} else {
		mi_autocomp->Check(true);
		config->autocomp=true;
	}
}

void mxMainWindow::OnConfigUseColors(wxCommandEvent &evt) {
	if (!mi_use_colors->IsChecked()) {
		mi_use_colors->Check(false);
		config->use_colors=false;
	} else {
		mi_use_colors->Check(true);
		config->use_colors=true;
	}
}

//void mxMainWindow::OnConfigHighRes(wxCommandEvent &evt) {
//	if (!mi_high_res->IsChecked()) {
//		mi_high_res->Check(false);
//		config->high_res_flows=false;
//	} else {
//		mi_high_res->Check(true);
//		config->high_res_flows=true;
//	}
// }

void mxMainWindow::OnConfigStepStepL(wxCommandEvent &evt) {
	mi_stepstep_l->Check(true);
	mi_stepstep_h->Check(false);
	mi_stepstep_m->Check(false);
	debug_speed->SetThumbPosition(config->stepstep_speed=_debug_speed_l);
	debug->SetSpeed(config->stepstep_speed);
}

void mxMainWindow::OnConfigStepStepH(wxCommandEvent &evt) {
	mi_stepstep_h->Check(true);
	mi_stepstep_l->Check(false);
	mi_stepstep_m->Check(false);
	debug_speed->SetThumbPosition(config->stepstep_speed=_debug_speed_h);
	debug->SetSpeed(config->stepstep_speed);
}
void mxMainWindow::OnConfigStepStepM(wxCommandEvent &evt) {
	mi_stepstep_m->Check(true);
	mi_stepstep_l->Check(false);
	mi_stepstep_h->Check(false);
	debug_speed->SetThumbPosition(config->stepstep_speed=_debug_speed_m);
	debug->SetSpeed(config->stepstep_speed);
}

void mxMainWindow::OnPaneClose(wxAuiManagerEvent& event) {
	if (event.pane->name == _T("toolbar")) mi_toolbar->Check(config->show_toolbar=false);
	else if (event.pane->name == _T("commands")) mi_commands->Check(config->show_commands=false);
	else if (event.pane->name == _T("debug_panel")) mi_debug_panel->Check(config->show_debug_panel=false);
}

void mxMainWindow::OnNotebookPageClose(wxAuiNotebookEvent& event)  {
	mxSource *source = CURRENT_SOURCE;
	if (source->GetModify()) {
		int res=wxMessageBox(_T("Hay cambios sin guardar. Desea guardarlos antes de cerrar el archivo?"), source->filename, wxYES_NO|wxCANCEL,this);
		if (res==wxCANCEL) {
			event.Veto();
			return;
		}
		if (res==wxYES) {
			if (source->sin_titulo) {
				wxCommandEvent evt;
				OnFileSaveAs(evt);
				if (source->GetModify()) {
					event.Veto();
					return;
				}
			} else
				source->SaveFile(source->filename);
		}
	}
}

void mxMainWindow::OnSocketEvent(wxSocketEvent &event){
	if (debug && debug->HasSocket(event.GetEventObject()))
		debug->SocketEvent(&event);
	else if (flow_editor && flow_editor->HasSocket(event.GetEventObject()) )
		flow_editor->SocketEvent(&event);
}

void mxMainWindow::OnScrollDegugSpeed(wxScrollEvent &evt) {
	int speed=evt.GetInt();
	mi_stepstep_l->Check(speed==0);
	mi_stepstep_m->Check(speed==1);
	mi_stepstep_h->Check(speed==2);
	debug->SetSpeed(speed);
}

void mxMainWindow::OnDebugDesktopVars(wxCommandEvent &evt) {
	new mxDesktopVarsEditor(this);
}

void mxMainWindow::CreateDesktopTestGrid() {
	desktop_test_grid = new mxDesktopTest(this,wxID_ANY);
	aui_manager.AddPane(desktop_test_grid, wxAuiPaneInfo().Name(_T("desktop_test")).Caption(_T("Prueba de Escritorio")).CloseButton(true).MaximizeButton(true).Bottom().Hide());
}

const wxArrayString &mxMainWindow::GetDesktopVars() {
	return desktop_test_grid->GetDesktopVars();
}

void mxMainWindow::SetDesktopVars(bool do_dt, const wxArrayString &vars) {
	debug->SetDoDesktopTest(do_dt);
	desktop_test_grid->SetDesktopVars(vars);
	if (do_dt) 
		ShowDesktopTestGrid();
	else
		HideDesktopTestGrid();
}

void mxMainWindow::ShowDesktopTestGrid() {
	aui_manager.GetPane(desktop_test_grid).Show();	
	aui_manager.Update();
}

void mxMainWindow::HideDesktopTestGrid() {
	aui_manager.GetPane(desktop_test_grid).Hide();	
	aui_manager.Update();
}


void mxMainWindow::OnToolbarShowDebugPanel(wxCommandEvent &evt) {
	if (mi_debug_panel->IsChecked()) {
		mi_debug_panel->Check(true);
		aui_manager.GetPane(debug_panel).Show();
		aui_manager.Update();	
	} else
		OnRunStepStep(evt);
}

void mxMainWindow::OnDebugEvaluate(wxCommandEvent &evt) {
	evaluate_window->Show();
}

void mxMainWindow::OnDebugHelp(wxCommandEvent &evt) {
	if (!helpw) helpw = new mxHelpWindow();
	helpw->ShowHelp(_T("debug.html"));
}

void mxMainWindow::SelectLine(mxSource *src, int l) {
	int index = notebook->GetPageIndex(last_source);
	if (index==wxNOT_FOUND) return;
	notebook->SetSelection(index);
	src->SetSelection(src->GetLineIndentPosition(l-1),src->GetLineEndPosition(l-1));
}

void mxMainWindow::SetDebugState(ds_enum state) {
	ds_state = state;
	switch (state) {
	case DS_STARTING:
		dp_button_run->SetLabel(_T("Finalizar"));
		dp_button_step->Disable();
		debug_status->SetLabel(_T("Iniciando"));
		dp_button_step->SetLabel(_T("Avanzar un Paso"));
		dp_button_desktop_vars->Enable(false);
		break;
	case DS_STOPPED:
		dp_button_run->SetLabel(_T("Comenzar"));
		dp_button_step->SetLabel(_T("Primer Paso"));
		dp_button_step->Enable();
		dp_button_desktop_vars->Enable();
		dp_button_evaluate->Disable();
		dp_button_step->Enable();
		dp_button_pause->Disable();
		debug_status->SetLabel(_T("No Iniciada"));
		break;
	case DS_PAUSED:
		dp_button_step->Enable(true);
		dp_button_pause->Enable(true);
		dp_button_pause->SetFocus();
		dp_button_step->SetFocus();
		dp_button_pause->SetLabel(_T("Continuar"));
		dp_button_evaluate->Enable();
		debug_status->SetLabel(_T("Pausado"));
		break;
	case DS_RESUMED:
		dp_button_step->Enable(false);
		dp_button_pause->Enable(true);
		dp_button_pause->SetLabel(_T("Pausar"));
		dp_button_evaluate->Disable();
		debug_status->SetLabel(_T("Ejecutando"));
		break;
	case DS_STEP:
//		dp_button_step->Enable(false);
		dp_button_pause->Disable();
//		dp_button_pause->SetLabel("Pausar");
		dp_button_evaluate->Disable();
		debug_status->SetLabel(_T("Ejecutando"));
		break;
	case DS_NONE:
		debug_status->SetLabel(_T("Desconocido"));
	}
}

bool mxMainWindow::SelectFirstError() {
	wxTreeItemIdValue v;
	wxTreeItemId item(results_tree->GetFirstChild(results_root,v));
	if (item.IsOk())
		results_tree->SelectItem(item);
	else
		return false;
	return true;
	
}

void mxMainWindow::OnHelpUpdates(wxCommandEvent &evt) {
	new mxUpdatesChecker(true);
}

void mxMainWindow::OnConfigLanguage(wxCommandEvent &evt) {
	new mxProfile(this);
}

void mxMainWindow::SetAccelerators() {
	wxAcceleratorEntry entries[5];
	entries[0].Set(wxACCEL_CTRL|wxACCEL_SHIFT, WXK_TAB, mxID_VIEW_NOTEBOOK_PREV);
	entries[1].Set(wxACCEL_CTRL, WXK_TAB, mxID_VIEW_NOTEBOOK_NEXT);
	entries[2].Set(wxACCEL_CTRL, WXK_PAGEUP, mxID_VIEW_NOTEBOOK_PREV);
	entries[3].Set(wxACCEL_CTRL, WXK_PAGEDOWN, mxID_VIEW_NOTEBOOK_NEXT);
	entries[4].Set(wxACCEL_CTRL|wxACCEL_SHIFT, WXK_F5, mxID_DO_THAT);
	wxAcceleratorTable accel(5, entries);
	SetAcceleratorTable(accel);
}

void mxMainWindow::OnViewNotebookNext(wxCommandEvent &evt){
	unsigned int i=notebook->GetSelection();
	i++; 
	if (i>=notebook->GetPageCount())
		i=0;
	notebook->SetSelection(i);
}

void mxMainWindow::OnViewNotebookPrev(wxCommandEvent &evt){
	int i=notebook->GetSelection();
	i--; 
	if (i<0)
		i=notebook->GetPageCount()-1;
	notebook->SetSelection(i);
}

void mxMainWindow::OnDoThat (wxCommandEvent &event) {
	LangSettings old=config->lang;
	new mxConfig(this);
	if (config->lang!=old) {
		config->profile=_T("<personalizado>");
		SetWordsForSources();
	}
}

void mxMainWindow::OnFilePrint (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		if (!printDialogData) printDialogData=new wxPrintDialogData;
		mxSource *src=CURRENT_SOURCE;
		wxPrinter printer(printDialogData);
		mxPrintOut printout(src,notebook->GetPageText(notebook->GetSelection()));
		src->SetPrintMagnification(-2);
		src->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_NONE);
		if (!printer.Print(this, &printout, true)) {
			if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
				wxMessageBox(_T("Ha ocurrido un error al intentar imprimir"),_T("Error"));
		}
		src->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_START|wxSTC_WRAPVISUALFLAG_END);
	}
}

void mxMainWindow::OnLink (wxHtmlLinkEvent &event) {
	if (event.GetLinkInfo().GetHref().StartsWith(_T("example:"))) {
		main_window->OpenProgram(DIR_PLUS_FILE(config->examples_dir,event.GetLinkInfo().GetHref().Mid(8)),false)->SetExample();
		if (IsMaximized()) Maximize(false);
		main_window->Raise();
	} else {
		if (!helpw) helpw = new mxHelpWindow();
		helpw->ShowHelp(event.GetLinkInfo().GetHref());
	}
}

void mxMainWindow::SetWordsForSources() {
	for (unsigned int i=0;i<notebook->GetPageCount();i++) {
		((mxSource*)(notebook->GetPage(i)))->SetWords();
		((mxSource*)(notebook->GetPage(i)))->Colourise(0,((mxSource*)(notebook->GetPage(i)))->GetLength());
	}
}

void mxMainWindow::OnRunSetInput (wxCommandEvent & evt) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		if (!src->input) src->input=new mxInputDialog(this);
		src->input->Show();
	}
}

void mxMainWindow::OnFileEditFlow (wxCommandEvent & evt) {
	IF_THERE_IS_SOURCE {
		if (!flow_editor) new FlowEditionManager();
		mxSource *source = CURRENT_SOURCE;
		if (source->GetFlowSocket()) { 
			source->GetFlowSocket()->Write("raise",5); 
			return;
		} else if (!source->GetReadOnly() && source->HaveComments()) {
			wxMessageBox(_T("Su algoritmo contiene comentarios. Si edita el diagrama y guarda los cambios perderá los comentarios!"),_T("Advertencia"),wxOK|wxICON_EXCLAMATION,this);
		}
		bool mod = source->GetModify();
		source->SaveFile(config->temp_file);
		source->SetModify(mod);
		if (debug->debugging)
			debug->Stop();
		else {
			mxProcess *flow=new mxProcess(source,notebook->GetPageText(notebook->GetSelection()));
			int id=flow_editor->GetNextId();
			if (flow->DrawAndEdit(config->temp_file,id,true)) source->EditFlow(flow,id); else delete flow;
		}
	}	
}

mxSource * mxMainWindow::FindFlowId (int id) {
	if (id<=0) return NULL;
	for(unsigned int i=0;i<notebook->GetPageCount();i++)
		if (((mxSource*)(notebook->GetPage(i)))->GetFlowId()==id) 
			return ((mxSource*)(notebook->GetPage(i)));
	return NULL;
}

mxSource * mxMainWindow::FindFlowSocket (wxObject *s) {
	if (!s) return NULL;
	for(unsigned int i=0;i<notebook->GetPageCount();i++)
		if (((mxSource*)(notebook->GetPage(i)))->GetFlowSocket()==s) 
			return ((mxSource*)(notebook->GetPage(i)));
	return NULL;
}

void mxMainWindow::SelectSource (mxSource * s) {
	for(unsigned int i=0;i<notebook->GetPageCount();i++)
		if (notebook->GetPage(i)==s) 
			notebook->SetSelection(i);
}

