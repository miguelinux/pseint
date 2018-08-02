#include <iostream>
#include <vector>
#include <algorithm>
#include <wx/clipbrd.h>
#include <wx/process.h>
#include <wx/socket.h>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/choicdlg.h>
#include <wx/ffile.h>
#include "HelpManager.h"
#include "Logger.h"
#include "string_conversions.h"
#include "mxVarWindow.h"
#include "mxDesktopTestGrid.h"
#include "error_recovery.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "mxStatusBar.h"
#include "ConfigManager.h"
#include "ids.h"
#include "mxProcess.h"
#include "mxDropTarget.h"
#include "DebugManager.h"
#include "mxMainWindow.h"
#include "RTSyntaxManager.h"

#define RT_DELAY 1000
#define RELOAD_DELAY 3500

#ifdef _AUTOINDENT
#	warning _AUTOINDENT no se lleva bien con Undo/Redo
#endif
#include "CommonParsingFunctions.h"
#include <wx/textdlg.h>

#ifdef UNICODE
bool EsLetra(const wxUniCharRef &k, bool incluir_nros=true) { return EsLetra(_C(k),incluir_nros); }
bool EsLetra(const wxUniChar &k, bool incluir_nros=true) { return EsLetra(_C(k),incluir_nros); }
#endif

int mxSource::last_id=0;

//const wxChar* mxSourceWords3 = 
//	"hacer entonces para ";

enum {MARKER_BLOCK_HIGHLIGHT=0,MARKER_DEBUG_RUNNING_ARROW,MARKER_DEBUG_RUNNING_BACK,MARKER_DEBUG_PAUSE_ARROW,MARKER_DEBUG_PAUSE_BACK,MARKER_ERROR_LINE};

enum {INDIC_FIELD=0, INDIC_ERROR_1, INDIC_ERROR_2};
#ifdef WX3
static int indic_to_mask[] = { 1, 2, 4, 8, 16 };
const int ANNOTATION_STYLE = wxSTC_STYLE_LASTPREDEFINED + 1;
#else
static int indic_to_mask[] = { wxSTC_INDIC0_MASK, wxSTC_INDIC1_MASK, wxSTC_INDIC2_MASK };
#endif

BEGIN_EVENT_TABLE (mxSource, wxStyledTextCtrl)
	EVT_LEFT_DOWN(mxSource::OnClick)
	EVT_STC_CHANGE(wxID_ANY,mxSource::OnChange)
#ifdef _AUTOINDENT
	EVT_STC_MODIFIED(wxID_ANY,mxSource::OnModified)
#endif
	EVT_STC_UPDATEUI (wxID_ANY, mxSource::OnUpdateUI)
	EVT_STC_CHARADDED (wxID_ANY, mxSource::OnCharAdded)
	EVT_KEY_DOWN(mxSource::OnKeyDown)
	EVT_STC_USERLISTSELECTION (wxID_ANY, mxSource::OnUserListSelection)
	EVT_STC_ROMODIFYATTEMPT (wxID_ANY, mxSource::OnModifyOnRO)
	EVT_STC_DWELLSTART (wxID_ANY, mxSource::OnToolTipTime)
	EVT_STC_DWELLEND (wxID_ANY, mxSource::OnToolTipTimeOut)
	EVT_MENU (mxID_EDIT_CUT, mxSource::OnEditCut)
	EVT_MENU (mxID_EDIT_COPY, mxSource::OnEditCopy)
	EVT_MENU (mxID_EDIT_PASTE, mxSource::OnEditPaste)
	EVT_MENU (mxID_EDIT_REDO, mxSource::OnEditRedo)
	EVT_MENU (mxID_EDIT_UNDO, mxSource::OnEditUndo)
	EVT_MENU (mxID_EDIT_COMMENT, mxSource::OnEditComment)
	EVT_MENU (mxID_EDIT_UNCOMMENT, mxSource::OnEditUnComment)
	EVT_MENU (mxID_EDIT_DUPLICATE, mxSource::OnEditDuplicate)
	EVT_MENU (mxID_EDIT_DELETE, mxSource::OnEditDelete)
	EVT_MENU (mxID_EDIT_TOGGLE_LINES_DOWN, mxSource::OnEditToggleLinesDown)
	EVT_MENU (mxID_EDIT_TOGGLE_LINES_UP, mxSource::OnEditToggleLinesUp)
	EVT_MENU (mxID_EDIT_SELECT_ALL, mxSource::OnEditSelectAll)
	EVT_MENU (mxID_EDIT_INDENT_SELECTION, mxSource::OnEditIndentSelection)
	EVT_MENU (mxID_VARS_DEFINE, mxSource::OnDefineVar)
	EVT_MENU (mxID_VARS_RENAME, mxSource::OnRenameVar)
	EVT_MENU (mxID_VARS_ADD_ONE_TO_DESKTOP_TEST, mxSource::AddOneToDesktopTest)
//	EVT_MENU (mxID_VARS_ADD_ALL_TO_DESKTOP_TEST, mxSource::AddAllToDesktopTest)
	EVT_STC_SAVEPOINTREACHED(wxID_ANY, mxSource::OnSavePointReached)
	EVT_STC_SAVEPOINTLEFT(wxID_ANY, mxSource::OnSavePointLeft)
	EVT_STC_MARGINCLICK (wxID_ANY, mxSource::OnMarginClick)
#ifdef WX3
#	define Z_EVT_STC_CALLTIP_CLICK EVT_STC_CALLTIP_CLICK
#else
#	define Z_EVT_STC_CALLTIP_CLICK(id, fn)     DECLARE_EVENT_TABLE_ENTRY( wxEVT_STC_CALLTIP_CLICK,         id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxStyledTextEventFunction, & fn ), (wxObject *) NULL ),
#endif
	// la siguiente linea va sin el prefijo "Z_", pero genera un error, hay que parchear wx/stc/stc.h, quitando un paréntesis izquierdo que sobra en la definicion de la macro EVT_STC_CALLTIP_CLICK (justo despues de los argumentos)
	Z_EVT_STC_CALLTIP_CLICK(wxID_ANY, mxSource::OnCalltipClick)
	EVT_SET_FOCUS (mxSource::OnSetFocus)
	EVT_MOUSEWHEEL(mxSource::OnMouseWheel)
	EVT_RIGHT_DOWN(mxSource::OnPopupMenu)
	EVT_STC_PAINTED(wxID_ANY, mxSource::OnPainted)
END_EVENT_TABLE()

// para el autocompletado de palabras clave
struct comp_list_item {
	wxString label;
	wxString text;
	wxString instruction; // este elemento solo debe mostrarse en el contexto de esta instruccion ("*" significa cualquier contexto, pero no como nombre de instruccion)
	comp_list_item(){}
	comp_list_item(wxString  _label, wxString _text, wxString _instruction):label(_label),text(_text),instruction(_instruction.Lower()){}
	operator wxString() { return label; }
	bool operator<(const comp_list_item &o) const { return label<o.label; }
};
static vector<comp_list_item> comp_list;

// para mostrar las ayudas emergentes (palabras que las disparan y textos de ayuda)
struct calltip_text { 
	wxString key, text; 
	bool only_if_not_first;
	calltip_text() {}
	calltip_text(const wxString &k, const wxString &t, bool f=false) 
		: key(k), text(t), only_if_not_first(f) {}
};
static vector<calltip_text> calltips_functions;
static vector<calltip_text> calltips_instructions;


#define STYLE_IS_CONSTANT(s) (s==wxSTC_C_STRING || s==wxSTC_C_STRINGEOL || s==wxSTC_C_CHARACTER || s==wxSTC_C_REGEX || s==wxSTC_C_NUMBER)
#define STYLE_IS_COMMENT(s) (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR)

mxSource::mxSource (wxWindow *parent, wxString ptext, wxString afilename) 
	: wxStyledTextCtrl (parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxVSCROLL) 
{

	_LOG("mxSource::mxSource "<<this);

	id=++last_id;
	temp_filename_prefix=DIR_PLUS_FILE(config->temp_dir,wxString("temp_")<<id);	
  
	SetModEventMask(wxSTC_MOD_INSERTTEXT|wxSTC_MOD_DELETETEXT|wxSTC_PERFORMED_USER|wxSTC_PERFORMED_UNDO|wxSTC_PERFORMED_REDO|wxSTC_LASTSTEPINUNDOREDO);
	
	mask_timers=false;
	rt_running=false;
	flow_socket=NULL;
	run_socket=NULL;
	input=NULL;
	
	page_text=ptext;
	
	brace_1=brace_2=-1;
	last_s1=last_s2=0;
	is_example=false;
	just_created=true;
	
	filename = afilename;
	sin_titulo = filename==wxEmptyString;
	SetStyling();
	SetTabWidth(config->tabw);
	SetUseTabs (true);
	
	wxFont font (config->wx_font_size, wxMODERN, wxNORMAL, wxNORMAL);
	StyleSetFont (wxSTC_STYLE_DEFAULT, font);
	
	AutoCompSetSeparator('|');
	AutoCompSetIgnoreCase(true);
	SetBackSpaceUnIndents (true);
	SetTabIndents (true);
	SetIndent (4);
	SetIndentationGuides(true);
	if (comp_list.empty()) SetAutocompletion();
	if (calltips_instructions.empty()) SetCalltips();
	
	SetMarginType (0, wxSTC_MARGIN_NUMBER);
	SetMarginWidth (0, TextWidth (wxSTC_STYLE_LINENUMBER, " XXX"));
	SetMarginSensitive (1, true);
//	StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour ("DARK GRAY"));
//	StyleSetBackground (wxSTC_STYLE_LINENUMBER, *wxWHITE);

	debug_line=-1;
	
	SetDropTarget(new mxDropTarget(this));
	
#ifdef _AUTOINDENT
	to_indent_first_line = 99999999;
	indent_timer = new wxTimer(GetEventHandler());
#endif
	rt_timer = new wxTimer(GetEventHandler());
	flow_timer = new wxTimer(GetEventHandler());
	reload_timer = new wxTimer(GetEventHandler());
	Connect(wxEVT_TIMER,wxTimerEventHandler(mxSource::OnTimer),NULL,this);
	
	status = STATUS_NEW_SOURCE;
//	SetStatus(STATUS_NEW_SOURCE);
	
	SetMouseDwellTime(500);
	
	er_register_source(this);
}

static void mxRemoveFile(const wxString &file) {
	if (wxFileName::FileExists(file)) wxRemoveFile(file);
}

mxSource::~mxSource() {
	_LOG("mxSource::~mxSource "<<this);
	debug->InvalidateLambda(this);
	er_unregister_source(this);
	mxRemoveFile(GetTempFilenameOUT());
	mxRemoveFile(GetTempFilenamePSC());
	mxRemoveFile(GetTempFilenamePSD());
	flow_timer->Stop();
	reload_timer->Stop();
	rt_timer->Stop();
	RTSyntaxManager::OnSourceClose(this);
	if (debug) debug->Close(this);
	main_window->OnSourceClose(this);
	if (flow_socket) {
		flow_socket->Write("quit\n",5);
		flow_socket=NULL;
	}
	KillRunningTerminal();
	mxProcess *proc=proc_list;
	while (proc) {
		if (proc->source==this) proc->SetSourceDeleted();
		proc=proc->next;
	}
}

void mxSource::SetStyle(int idx, const char *foreground, const char *background, int fontStyle){
	wxFont font (config->wx_font_size- ((fontStyle&mxSOURCE_SMALLER)?1:0),
				 wxMODERN, wxNORMAL, wxNORMAL, false, config->wx_font_name);
	StyleSetFont (idx, font);
	if (foreground) StyleSetForeground (idx, wxColour (foreground));
	if (background)  StyleSetBackground (idx, wxColour (background));
	StyleSetBold (idx, (fontStyle & mxSOURCE_BOLD) > 0);
	StyleSetItalic (idx, (fontStyle & mxSOURCE_ITALIC) > 0);
	StyleSetUnderline (idx, (fontStyle & mxSOURCE_UNDERL) > 0);
	StyleSetVisible (idx, (fontStyle & mxSOURCE_HIDDEN) == 0);

}

void mxSource::SetStyling(bool colour) {
	
	const char *CL_BLOCK_BACK = config->use_dark_theme ? "#222211" : "#FFFFAA" ;
	const char *CL_DEBUG_PAUSE_BACK = config->use_dark_theme ? "#444422" : "#FFFFAA" ;
	const char *CL_DEBUG_RUN_BACK = config->use_dark_theme ? "#224422" : "#C8FFC8" ;
	
	IndicatorSetStyle(INDIC_FIELD,wxSTC_INDIC_BOX);
	IndicatorSetStyle(INDIC_ERROR_1,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetStyle(INDIC_ERROR_2,wxSTC_INDIC_SQUIGGLE);
	//	IndicatorSetStyle(2,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetForeground (INDIC_FIELD, 0x005555);
	IndicatorSetForeground (INDIC_ERROR_1, 0x0000FF);
	IndicatorSetForeground (INDIC_ERROR_2, 0x004499);
	
	MarkerDefine(MARKER_ERROR_LINE,wxSTC_MARK_PLUS, "WHITE", "RED");
	MarkerDefine(MARKER_DEBUG_RUNNING_ARROW,wxSTC_MARK_SHORTARROW, "BLACK", "GREEN");
	MarkerDefine(MARKER_DEBUG_RUNNING_BACK,wxSTC_MARK_BACKGROUND, CL_DEBUG_RUN_BACK, CL_DEBUG_RUN_BACK);
	MarkerDefine(MARKER_DEBUG_PAUSE_ARROW,wxSTC_MARK_SHORTARROW, "BLACK", "YELLOW");
	MarkerDefine(MARKER_DEBUG_PAUSE_BACK,wxSTC_MARK_BACKGROUND, CL_DEBUG_PAUSE_BACK, CL_DEBUG_PAUSE_BACK);
	MarkerDefine(MARKER_BLOCK_HIGHLIGHT,wxSTC_MARK_BACKGROUND, CL_BLOCK_BACK, CL_BLOCK_BACK);

	
	const char *CL_REG_BACK  = config->use_dark_theme ? "#333333" : "#FFFFFF" ;
	const char *CL_REG_FORE  = config->use_dark_theme ? "#FAFAFA" : "#000000" ;
	const char *CL_KEYWORD   = config->use_dark_theme ? "#9999FA" : "#000080" ;
	const char *CL_STRING    = config->use_dark_theme ? "#FA9999" : "#FF0000" ;
	const char *CL_NUMBER    = config->use_dark_theme ? "#FAFA99" : "#A0522D" ;
	const char *CL_COMMENT_1 = config->use_dark_theme ? "#999999" : "#969696" ;
	const char *CL_COMMENT_2 = config->use_dark_theme ? "#33FA33" : "#5050FF" ;
	const char *CL_ALT_FORE  = config->use_dark_theme ? "#999999" : "#969696" ;
	const char *CL_ALT_BACK  = config->use_dark_theme ? "#606060" : "#D3D3D3" ;
	const char *CL_HLG_BACK  = config->use_dark_theme ? "#335050" : "#AACCFF" ;
#ifdef WX3
	const char *CL_ANOT_FORE = config->use_dark_theme ? "#FA9999" : "#800000" ;
	const char *CL_ANOT_BACK = config->use_dark_theme ? "#454545" : "#FAFAD7" ;
#endif
	
	StyleSetForeground (wxSTC_STYLE_LINENUMBER, CL_ALT_FORE);
	StyleSetBackground (wxSTC_STYLE_LINENUMBER, CL_REG_BACK);
	SetCaretForeground (CL_REG_FORE);
	SetSelBackground(true,CL_ALT_BACK);
	
	SetLexer(wxSTC_LEX_CPPNOCASE); // setear el lexer antes de las keywords!!! sino en wx 3 no tiene efecto
	SetWords();
	SetStyle(wxSTC_STYLE_DEFAULT,            CL_REG_FORE,       CL_REG_BACK,        0);               // default
	SetStyle(wxSTC_C_DEFAULT,                CL_REG_FORE,       CL_REG_BACK,        0);               // default
	SetStyle(wxSTC_C_COMMENT,                CL_COMMENT_1,      CL_REG_BACK,        0);               // comment
	SetStyle(wxSTC_C_COMMENTLINE,            CL_COMMENT_1,      CL_REG_BACK,        mxSOURCE_ITALIC); // comment line
	SetStyle(wxSTC_C_COMMENTDOC,             CL_COMMENT_2,      CL_REG_BACK,        mxSOURCE_ITALIC); // comment doc
	SetStyle(wxSTC_C_COMMENTLINEDOC,         CL_COMMENT_2,      CL_REG_BACK,        0);               // special comment 
	SetStyle(wxSTC_C_NUMBER,                 CL_NUMBER,         CL_REG_BACK,        0);               // number
	SetStyle(wxSTC_C_WORD,                   CL_KEYWORD,        CL_REG_BACK,        mxSOURCE_BOLD);   // keywords
	SetStyle(wxSTC_C_IDENTIFIER,             CL_REG_FORE,       CL_REG_BACK,        0);               // identifier 
	SetStyle(wxSTC_C_CHARACTER,              CL_STRING,         CL_REG_BACK,        0);               // character
	SetStyle(wxSTC_C_STRINGEOL,              CL_STRING,         CL_ALT_BACK,        0);               // string eol
	SetStyle(wxSTC_C_STRING,                 CL_STRING,         CL_REG_BACK,        0);               // character
	SetStyle(wxSTC_C_OPERATOR,               CL_REG_FORE,       CL_REG_BACK,        mxSOURCE_BOLD);   // operator 
	SetStyle(wxSTC_C_VERBATIM,               CL_REG_FORE,       CL_REG_BACK,        0);               // default verbatim
	SetStyle(wxSTC_C_WORD2,                  CL_KEYWORD,        CL_REG_BACK,        0);               // extra words
	SetStyle(wxSTC_C_GLOBALCLASS,            CL_REG_FORE,       CL_HLG_BACK,        0);               // keywords errors
	SetStyle(wxSTC_STYLE_BRACELIGHT,         CL_REG_FORE,       CL_ALT_BACK,        mxSOURCE_BOLD); 
	SetStyle(wxSTC_STYLE_BRACEBAD,           CL_STRING,         CL_ALT_BACK,        mxSOURCE_BOLD); 
#ifdef WX3
	SetStyle(ANNOTATION_STYLE,               CL_ANOT_FORE,      CL_ANOT_BACK,       mxSOURCE_ITALIC|mxSOURCE_SMALLER); 
#endif
//		SetStyle(wxSTC_C_UUID,                   "ORCHID",          "WHITE",        0);               // uuid
//		SetStyle(wxSTC_C_PREPROCESSOR,           "FOREST GREEN",    "WHITE",        0);               // preprocessor
//		SetStyle(wxSTC_C_REGEX,                  "ORCHID",          "WHITE",        0);               // regexp  
//		SetStyle(wxSTC_C_COMMENTDOCKEYWORD,      "CORNFLOWER BLUE", "WHITE",        0);               // doxy keywords
//		SetStyle(wxSTC_C_COMMENTDOCKEYWORDERROR, "RED",             "WHITE",        0);               // keywords errors
//		SetStyle(wxSTC_STYLE_BRACELIGHT,         "RED",             "Z LIGHT BLUE", mxSOURCE_BOLD); 
#ifdef WX3
	AnnotationSetVisible(wxSTC_ANNOTATION_INDENTED);
#endif
}

void mxSource::OnEditCut(wxCommandEvent &evt) {
	if (GetReadOnly()) return MessageReadOnly();
	if (GetSelectionEnd()-GetSelectionStart() <= 0) return;
	Cut();
}

void mxSource::OnEditCopy(wxCommandEvent &evt) {
	if (GetSelectionEnd()-GetSelectionStart() <= 0) return;
	Copy();
}

void mxSource::OnEditPaste(wxCommandEvent &evt) {
	
	if (CallTipActive())
		HideCalltip();
	else if (AutoCompActive())
		AutoCompCancel();
	
	if (GetReadOnly()) return MessageReadOnly();
	if (!wxTheClipboard->Open()) return;
	wxTextDataObject data;
	if (!wxTheClipboard->GetData(data)) {
		wxTheClipboard->Close();
		return;
	}
	wxString str = data.GetText();
	BeginUndoAction();
	// borrar la seleccion previa
	if (GetSelectionEnd()-GetSelectionStart()!=0)
		DeleteBack();
	// insertar el nuevo texto
	int cp = GetCurrentPos();
	InsertText(cp,str);
	// indentar el nuevo texto
	int l1 = LineFromPosition(cp);
	int l2 = LineFromPosition(cp+str.Len());
	int pos = PositionFromLine(l1);
	int p=cp-1;
	while (p>=pos && (GetCharAt(p)==' ' || GetCharAt(p)=='\t'))
		p--;
	if (p!=pos && p>=pos) 
		l1++;
	cp+=str.Len();
	if (l2>=l1) {
		char c=LineFromPosition(cp);
		pos=cp-GetLineIndentPosition(c);
		if (pos<0) pos=0;
		Indent(l1,l2);
		pos+=GetLineIndentPosition(c);
		SetSelection(pos,pos);
	} else
		SetSelection(cp,cp);
	wxTheClipboard->Close();
	EndUndoAction();
}

void mxSource::OnEditUndo(wxCommandEvent &evt) {
	if (!CanUndo()) return;
	Undo();	
}

void mxSource::OnEditRedo(wxCommandEvent &evt) {
	if (!CanRedo()) return;
	Redo();
}

void mxSource::SetFileName(wxString afilename) {
	sin_titulo = false;
	filename = afilename;
}


void mxSource::OnEditComment(wxCommandEvent &evt) {
	if (GetReadOnly()) return MessageReadOnly();
	int ss = GetSelectionStart(), se = GetSelectionEnd();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (min>max) { int aux=min; min=max; max=aux; }
	if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
	BeginUndoAction();
	for (int i=min;i<=max;i++) {
		//if (GetLine(i).Left(2)!="//") {
		SetTargetStart(PositionFromLine(i));
		SetTargetEnd(PositionFromLine(i));
		ReplaceTarget("//");
	}	
	EndUndoAction();
}

void mxSource::OnEditUnComment(wxCommandEvent &evt) {
	if (GetReadOnly()) return MessageReadOnly();
	int ss = GetSelectionStart();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(GetSelectionEnd());
	if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
	BeginUndoAction();
	for (int i=min;i<=max;i++) {
		int aux;
		if (GetLine(i).Left(2)=="//") {
			SetTargetStart(PositionFromLine(i));
			SetTargetEnd(PositionFromLine(i)+2);
			ReplaceTarget("");
		} else if (GetLine(i).Left((aux=GetLineIndentPosition(i))-PositionFromLine(i)+2).Right(2)=="//") {
			SetTargetStart(aux);
			SetTargetEnd(aux+2);
			ReplaceTarget("");
		}
	}
	Indent(min,max);
	EndUndoAction();
}

void mxSource::OnEditDelete(wxCommandEvent &evt) {
	if (GetReadOnly()) return MessageReadOnly();
	int ss,se;
	int min=LineFromPosition(ss=GetSelectionStart());
	int max=LineFromPosition(se=GetSelectionEnd());
	if (max==min) {
		LineDelete();
		if (PositionFromLine(ss)!=min)
			GotoPos(GetLineEndPosition(min));
		else
			GotoPos(ss);
	} else {
		if (min>max) { int aux=min; min=max; max=aux; aux=ss; ss=se; se=aux;}
		if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
		GotoPos(ss);
		BeginUndoAction();
		for (int i=min;i<=max;i++)
			LineDelete();
		if (LineFromPosition(ss)!=min) 
			GotoPos(PositionFromLine(min+1)-1);
		else 
			GotoPos(ss);
		EndUndoAction();
	}
}

void mxSource::OnEditDuplicate(wxCommandEvent &evt) {
	if (GetReadOnly()) return MessageReadOnly();
	int ss,se;
	int min=LineFromPosition(ss=GetSelectionStart());
	int max=LineFromPosition(se=GetSelectionEnd());
	BeginUndoAction();
	if (max==min) {
		LineDuplicate();
	} else {
		if (min>max) { 
			int aux=min; 
			min=max; 
			max=aux;
		}
		if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
		wxString text;
		for (int i=min;i<=max;i++)
			text+=GetLine(i);
		InsertText(PositionFromLine(max+1),text);
		SetSelection(ss,se);
	}
	EndUndoAction();
}

void mxSource::OnEditSelectAll (wxCommandEvent &event) {
	SetSelection (0, GetTextLength ());
}

void mxSource::MakeCompletionFromKeywords(wxArrayString &output, int start_pos, const wxString &typed) {
	int l = typed.Len();
	wxString instruccion = GetInstruction(start_pos);
	for (size_t j,i=0;i<comp_list.size();i++) {
		if (comp_list[i].instruction=="*") {
			if (instruccion=="") continue;
		} else {
			if (comp_list[i].instruction!=instruccion) continue;
		}
		for (j=0;j<l;j++)
			if (typed[j]!=wxTolower(comp_list[i].label[j]))
				break;
		if (j==l && (comp_list[i].label[3]!=' '||comp_list[i].label[0]!='F')) {
			output.Add(comp_list[i]);
		}
	}
}

void mxSource::MakeCompletionFromIdentifiers(wxArrayString &output, int start_pos, const wxString &typed) {
	wxArrayString &vars = vars_window->all_vars;
	int l=typed.Len(), j;
	for(unsigned int i=0;i<vars.GetCount();i++) { 
		for (j=0;j<l;j++)
			if (typed[j]!=wxTolower(vars[i][j]))
				break;
		if (j==l) {
			output.Add(vars[i]);
		}
	}
}

void mxSource::OnCharAdded (wxStyledTextEvent &event) {
	char chr = event.GetKey();
	if (chr=='\n') {
		int currentLine = GetCurrentLine();
		if (!config->smart_indent) {
			int lineInd = 0;
			if (currentLine > 0)
				lineInd = GetLineIndentation(currentLine - 1);
			if (lineInd == 0) return;
			SetLineIndentation (currentLine, lineInd);
			GotoPos(GetLineIndentPosition(currentLine));
		} else {
			if (currentLine>0) IndentLine(currentLine-1);
			IndentLine(currentLine);
			GotoPos(GetLineIndentPosition(currentLine));
		}
		if (config->autoclose) TryToAutoCloseSomething(currentLine);
	} 

	if (AutoCompActive()) {
		comp_to=GetCurrentPos();
	} else if (chr==' ' && config->autocomp) {
		int p2=comp_to=GetCurrentPos(), s=GetStyleAt(p2-2);
		if (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_STRING || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRINGEOL) return;
		int p1=comp_from=WordStartPosition(p2-1,true);
		wxString st=GetTextRange(p1,p2).Lower(); st[0]=toupper(st[0]);
		wxArrayString res;
		for (size_t i=0;i<comp_list.size();i++) {
			if (comp_list[i].label.StartsWith(st))
				res.Add(comp_list[i]);
		}
		if (!res.IsEmpty()) ShowUserList(res,p1,p2);
	} else if ( EsLetra(chr,true) && config->autocomp) 
	{
		int p2=comp_to=GetCurrentPos();
		int s=GetStyleAt(p2);
		if (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_STRING || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRINGEOL) return;
		int p1=comp_from=WordStartPosition(p2,true);
		if (p2-p1>2 && EsLetra(GetCharAt(p1),false)) {
			wxString str = GetTextRange(p1,p2);
			str.MakeLower();
			wxArrayString res;
			MakeCompletionFromKeywords(res,p1,str);
			MakeCompletionFromIdentifiers(res,p1,str);
			if (!res.IsEmpty()) ShowUserList(res,p1,p2);
		}
	} else if (chr==';' && GetStyleAt(GetCurrentPos()-2)!=wxSTC_C_STRINGEOL) HideCalltip(false,true);
	if (config->calltip_helps) {
		if (chr==' ' || chr=='\n' || chr=='\t' || chr=='\r') {
			int p = GetCurrentPos()-1;
			while (p>0 && (GetCharAt(p)==' ' || GetCharAt(p)=='\t' || GetCharAt(p)=='\r' || GetCharAt(p)=='\n'))
				p--;
			int s = GetStyleAt(p);
			if (s!=wxSTC_C_CHARACTER && s!=wxSTC_C_STRING && s!=wxSTC_C_STRINGEOL && s!=wxSTC_C_COMMENTLINE) {
				int p2=p+1;	p = WordStartPosition(p,true)-1;
				wxString text = GetTextRange(p+1,p2); MakeUpper(text);
				HideCalltip();
				for(size_t i=0;i<calltips_instructions.size();i++) { 
					if (calltips_instructions[i].key == text) {
						if (calltips_instructions[i].only_if_not_first) {
							int l = LineFromPosition(p+1), paux = p;
							while (paux>0 && (GetCharAt(paux)==' ' || GetCharAt(paux)=='\t' || GetCharAt(paux)=='\r' || GetCharAt(paux)=='\n'))
								paux--;
							if (LineFromPosition(paux+1)!=l || GetCharAt(paux)==';') continue;
						}
						ShowCalltip(GetCurrentPos(),calltips_instructions[i].text);
						break;
					}
				}
			}
		} else
		if (chr=='(') {
			int p = GetCurrentPos()-1;
			while (p>0 && (GetCharAt(p)==' ' || GetCharAt(p)=='\t')) p--;
			int s = GetStyleAt(p);
			if (s!=wxSTC_C_CHARACTER && s!=wxSTC_C_STRING && s!=wxSTC_C_STRINGEOL && s!=wxSTC_C_COMMENTLINE) {
				int p0 = WordStartPosition(p,true);
				wxString text = GetTextRange(p0,p); MakeUpper(text);
				for(size_t i=0;i<calltips_functions.size();i++) { 
					if (calltips_functions[i].key == text) {
						ShowCalltip(GetCurrentPos(),calltips_functions[i].text);
						break;
					}
				}
			}
		} else
		if (chr==')'||chr==']') HideCalltip();
	}
}

void mxSource::SetModify (bool modif) {
	if (is_example) return;
	if (modif) {
		bool ro=GetReadOnly();
		if (ro) SetReadOnly(false);
		int p=GetLength()?GetSelectionStart()-1:0;
		if (GetLength()&&p<1) p=1;
		BeginUndoAction();
		SetTargetStart(p); 
		SetTargetEnd(p);
		ReplaceTarget(" ");
		SetTargetStart(p); 
		SetTargetEnd(p+1);
		ReplaceTarget("");
		EndUndoAction();
		if (ro) SetReadOnly(true);
	} else 
		SetSavePoint();
}

void mxSource::OnUserListSelection(wxStyledTextEvent &evt) {
	SetTargetStart(comp_from);
	SetTargetEnd(comp_to);
	size_t i=0;
	wxChar last_char = '\0';
	wxString what = evt.GetText();
	if (config->smart_indent) 
		while (i<comp_list.size() && comp_list[i]!=what) i++;
	if (config->smart_indent && i!=comp_list.size()) {
		wxString text(comp_list[i].text);
		if (!cfg_lang[LS_FORCE_SEMICOLON] && text.Last()==';') text.RemoveLast();
		if (comp_from>5&&text.Last()==' '&&GetTextRange(comp_from-4,comp_from).Upper()=="FIN ")
			text.Last()='\n';
		ReplaceTarget(text);
		last_char = text.Last();
		SetSelection(comp_from+text.Len(),comp_from+text.Len());
		int lfp=LineFromPosition(comp_from);
		if (text.Mid(0,3)=="Fin" || text=="Hasta Que " || text=="Mientras Que " || text.Mid(0,4)=="SiNo"||text.Last()=='\n')
			IndentLine(lfp);
		if (text.Last()=='\n') {
			IndentLine(lfp+1);
			int lip=GetLineIndentPosition(lfp+1);
			SetSelection(lip,lip);
			if (config->autoclose) TryToAutoCloseSomething(lfp+1);
		}
	} else {
		while (what.Last()=='\n') what.RemoveLast();
		ReplaceTarget(what);
		last_char = what.Last();
		SetSelection(GetTargetEnd(),GetTargetEnd());
	}
	if (last_char!='\0') {
		wxStyledTextEvent evt2;
		evt2.SetKey(last_char);
		OnCharAdded(evt2);
	}
}

void mxSource::SetFieldIndicator(int p1, int p2, bool select) {
	SetIndics(p1,p2-p1,INDIC_FIELD,true);
	if (select) { GotoPos(p1); SetSelection(p1,p2); }
}

void mxSource::OnUpdateUI (wxStyledTextEvent &event) {
	int p = GetCurrentPos();
	int indics = _if_wx3_else(IndicatorAllOnFor(p),GetStyleAt(p));
	if (indics&indic_to_mask[INDIC_FIELD]) {
		int p2=p;
		while (GetStyleAt(p2)&indic_to_mask[INDIC_FIELD])
			p2++;
		while (GetStyleAt(p)&indic_to_mask[INDIC_FIELD])
			p--;
		int s1=GetAnchor(), s2=GetCurrentPos();
		if (s1==s2) {
			if (s1==p+1 && last_s1==p+1 && last_s2==p2) {
				SetSelection(p,p);
			} else {
				SetAnchor(p+1);
				SetCurrentPos(p2);
			}
		} else if (s1>s2) {
			if (s1<p2) SetAnchor(p2);
			if (s2>p+1) SetCurrentPos(p+1);
		} else {
			if (s2<p2) SetCurrentPos(p2);
			if (s1>p+1) SetAnchor(p+1);
		}
		last_s1=GetSelectionStart(); last_s2=GetSelectionEnd();
	} 
	else if (indics&(indic_to_mask[INDIC_ERROR_1]|indic_to_mask[INDIC_ERROR_2])) { // si estoy sobre un error del rt_syntax muestra el calltip con el mensaje
		unsigned int l=GetCurrentLine();
		if (rt_errors.size()>l && rt_errors[l].is) ShowRealTimeError(p,rt_errors[l].s);
	} else if (!AutoCompActive()) { // para que un error por no haber terminado de escribir detectado por rt_syntax no oculte el autocompletado
		if (p) p--; indics = GetStyleAt(p);
		if (indics&(indic_to_mask[INDIC_ERROR_1]|indic_to_mask[INDIC_ERROR_2])) { // si estoy justo despues de un error del rt_syntax tambien muestra el calltip con el mensaje
			unsigned int l=GetCurrentLine();
			if (rt_errors.size()>l && rt_errors[l].is) ShowRealTimeError(p,rt_errors[l].s);
		} else { // si no estoy sobre ningun error, oculta el calltip si es que habia
			HideCalltip(true,false);
		}
	}
	if (blocks_markers.GetCount()) UnHighLightBlock(); 
	if (config->highlight_blocks && !rt_timer->IsRunning()) HighLightBlock();
}

void mxSource::UnHighLightBlock() {
	if (blocks_markers.GetCount()) {
		for(unsigned int i=0;i<blocks_markers.GetCount();i++) MarkerDeleteHandle(blocks_markers[i]);
		blocks_markers.Clear();
	}
}

void mxSource::HighLightBlock() {
	int l=GetCurrentLine(), nl=GetLineCount();
	if (int(blocks.GetCount())>l && blocks[l]!=-1) {
		for(int i=l;i<=blocks[l];i++)
			if (i>=0 && i<nl) 
				blocks_markers.Add(MarkerAdd(i,MARKER_BLOCK_HIGHLIGHT));
	} else if (int(blocks_reverse.GetCount())>l && blocks_reverse[l]!=-1) {
		for(int i=blocks_reverse[l];i<=l;i++)
			if (i>=0 && i<nl) 
				blocks_markers.Add(MarkerAdd(i,MARKER_BLOCK_HIGHLIGHT));
	}
}

void mxSource::OnEditToggleLinesUp (wxCommandEvent &event) {
	int ss = GetSelectionStart(), se = GetSelectionEnd();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (min>max) { int aux=min; min=max; max=aux; }
	if (min<max && PositionFromLine(max)==GetSelectionEnd()) max--;
	if (min>0) {
		BeginUndoAction();
		wxString line = GetLine(min-1);
		if (max==GetLineCount()-1)
			AppendText("\n");
		SetTargetStart(PositionFromLine(max+1));
		SetTargetEnd(PositionFromLine(max+1));
		ReplaceTarget(line);
		SetTargetStart(PositionFromLine(min-1));
		SetTargetEnd(PositionFromLine(min));
		ReplaceTarget("");
		EndUndoAction();
		if (config->smart_indent) {
			OnEditIndentSelection(event);
			IndentLine(max);
		}
	}
}

void mxSource::OnEditToggleLinesDown (wxCommandEvent &event) {
	int ss = GetSelectionStart(), se = GetSelectionEnd();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(se);
	if (PositionFromLine(min)==ss) ss=-1;
	if (PositionFromLine(max)==se) se=-1;
	if (min>max) { int aux=min; min=max; max=aux; }
	if (min<max && PositionFromLine(max)==GetSelectionEnd()) max--;
	if (max+1<GetLineCount()) {
		BeginUndoAction();
		wxString line = GetLine(max+1);
		SetTargetStart(GetLineEndPosition(max));
		SetTargetEnd(GetLineEndPosition(max+1));
		ReplaceTarget("");
		SetTargetStart(PositionFromLine(min));
		SetTargetEnd(PositionFromLine(min));
		ReplaceTarget(line);
		if (ss==-1) SetSelectionStart(PositionFromLine(min+1));
		if (se==-1) SetSelectionStart(PositionFromLine(min+1));
		EndUndoAction();
		if (config->smart_indent) {
			IndentLine(min);
			OnEditIndentSelection(event);
		}
	}
}

void mxSource::OnModifyOnRO (wxStyledTextEvent &event) {
	MessageReadOnly();
}

void mxSource::MessageReadOnly() {
	static wxDateTime last_msg=wxDateTime((time_t)0);
	if (wxDateTime::Now().Subtract(last_msg).GetSeconds()>0) {
		if (flow_socket) {
			wxMessageBox("Cierre la ventana del editor de diagramas de flujo para este algortimo, antes de continuar editando el pseudocódigo.");
			flow_socket->Write("raise\n",6); 
		}
		else if (!is_example) wxMessageBox("No se puede modificar el pseudocódigo mientras está siendo ejecutado paso a paso.");
		else wxMessageBox("No se permite modificar los ejemplos, pero puede copiarlo y pegarlo en un nuevo archivo.");
	}
	last_msg=wxDateTime::Now();
}

void mxSource::SetExample() {
	wxString total;
	wxString proceso = cfg_lang[LS_PREFER_ALGORITMO]?"Algoritmo":"Proceso";
	wxString finproceso = cfg_lang[LS_PREFER_ALGORITMO]?"FinAlgoritmo":"FinProceso";
	wxString subproceso = cfg_lang[LS_PREFER_FUNCION]?"Funcion":(cfg_lang[LS_PREFER_ALGORITMO]?"SubAlgoritmo":"SubProceso");
	wxString finsubproceso = cfg_lang[LS_PREFER_FUNCION]?"FinFuncion":(cfg_lang[LS_PREFER_ALGORITMO]?"FinSubAlgoritmo":"FinSubProceso");
	for (int i=2;i<GetLineCount();i++) {
		wxString str=GetLine(i);
		int p0=str.Index('{');
		while (p0!=wxNOT_FOUND) {
			wxString aux=str.Mid(p0);
			int p1=aux.Index('#');
			int p2=aux.Index('}');
			if (p2==wxNOT_FOUND) {
				_LOG("mxSource::SetExample ERROR 1 parsing example: "<<page_text);
				wxMessageBox("Ha ocurrido un error al procesar el ejemplo. Puede que el pseudocódigo no sea correcto.");
				break;
			}
			if (p1==wxNOT_FOUND||p1>p2) {
				
				if (p2==2&&aux[1]==';') {
					if (cfg_lang[LS_FORCE_SEMICOLON])
						str=str.Mid(0,p0)+";"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+aux.Mid(p2+1);
				
				} else if (p2==2&&aux[1]=='&') {
					if (cfg_lang[LS_WORD_OPERATORS])
						str=str.Mid(0,p0)+"Y"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+"&"+aux.Mid(p2+1);
				
				} else if (p2==2&&aux[1]=='|') {
					if (cfg_lang[LS_WORD_OPERATORS])
						str=str.Mid(0,p0)+"O"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+"|"+aux.Mid(p2+1);
				
				} else if (p2==2&&aux[1]=='~') {
					if (cfg_lang[LS_WORD_OPERATORS])
						str=str.Mid(0,p0)+"NO"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+"~"+aux.Mid(p2+1);
				
				} else if (p2==2&&aux[1]=='%') {
					if (cfg_lang[LS_WORD_OPERATORS])
						str=str.Mid(0,p0)+"MOD"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+"%"+aux.Mid(p2+1);
					
				} else if (p2==8&&aux.Mid(1,7).Upper()=="PROCESO") {
					str=str.Mid(0,p0)+proceso+aux.Mid(p2+1);
				} else if (p2==11&&aux.Mid(1,10).Upper()=="FINPROCESO") {
					str=str.Mid(0,p0)+finproceso+aux.Mid(p2+1);
				} else if (p2==11&&aux.Mid(1,10).Upper()=="SUBPROCESO") {
					str=str.Mid(0,p0)+subproceso+aux.Mid(p2+1);
				} else if (p2==14&&aux.Mid(1,13).Upper()=="FINSUBPROCESO") {
					str=str.Mid(0,p0)+finsubproceso+aux.Mid(p2+1);
				
				} else if (p2>8 && aux.Mid(1,8).Upper()=="DEFINIR " ) {
					if (cfg_lang[LS_FORCE_DEFINE_VARS])
						str=str.Mid(0,p0)+str.Mid(p0+1,p2-1)+aux.Mid(p2+1);
					else
						str="";
				
				} else {
					_LOG("mxSource::SetExample ERROR 2 parsing example: "<<page_text);
					str=str.Mid(0,p0)+str.Mid(p0+1,p2-1)+aux.Mid(p2+1);
				}
				
			} else {
				
				p1+=p0; p2+=p0;
				if (cfg_lang[LS_BASE_ZERO_ARRAYS])
					str=str.Mid(0,p0)+str.Mid(p1+1,p2-p1-1)+str.Mid(p2+1);
				else
					str=str.Mid(0,p0)+str.Mid(p0+1,p1-p0-1)+str.Mid(p2+1);
				
			}
			p0=str.Index('{');
		}
		total+=str;
	}
	SetText(total);
	SetReadOnly(sin_titulo=is_example=true);
	SetSavePoint();
//	SetStatus(STATUS_EXAMPLE); // lo hace el main despues de cargarle el contenido para que se mantenga el status_should_change=false
}

void mxSource::OnEditIndentSelection(wxCommandEvent &evt) {
	int pb,pe;
	GetSelection(&pb,&pe);
	if (pb>pe) { int a=pb; pb=pe; pe=a; }
	int b=LineFromPosition(pb);
	int e=LineFromPosition(pe);
	Indent(b,e);
	if (pb==pe) {
		int p=GetLineIndentPosition(b);
		if (pb<p) SetSelection(p,p);
	}
}

bool mxSource::IndentLine(int l, bool goup) {
	int btype;
	int cur = GetIndentLevel(l,goup,btype);
	wxString line = GetLine(l);
	int len = line.Len();
	int ws = SkipWhite(line,0,len); // ws = word start
	if (ws<len && !(line[ws]=='/'&&line[ws+1]=='/')) {
		int we = SkipWord(line,ws,len); // we = word end
		wxString fword = line.Mid(ws,we-ws);
		MakeUpper(fword);
		if (fword=="SINO") cur-=4;
		else if (fword=="DE" && we+10<len && line.SubString(ws,we+10).Upper()=="DE OTRO MODO:") cur-=4;
		else if (fword=="HASTA" && we+4<len && line.SubString(ws,we+4).Upper()=="HASTA QUE ") cur-=4;
		else if (fword=="MIENTRAS" && we+4<len && line.SubString(ws,we+4).Upper()=="MIENTRAS QUE ") cur-=4;
		else if (fword=="FINSEGUN"||fword==_Z("FINSEGÚN")) cur-=8;
		else if (fword=="FINMIENTRAS") cur-=4;
		else if (fword=="FINPARA") cur-=4;
		else if (fword=="FIN") { 
			cur-=4;
			ws = SkipWhite(line,we,len);
			we = SkipWord(line,ws,len);
			fword = line.Mid(ws,we-ws);
			MakeUpper(fword);
			if (fword=="SEGUN"||fword==_Z("SEGÚN")) cur-=4;
		}
		else if (fword=="FINSI") cur-=4;
		else if (fword=="FINPROCESO"||fword=="FINALGORITMO") cur=0;
		else if (fword=="FINSUBPROCESO"||fword=="FINFUNCION"||fword=="FINSUBALGORITMO"||fword==_Z("FINFUNCIÓN")) cur=0;
		else {
			ws = we;
			while (ws<len) {
				if (ws+1<len && line[ws]=='/' && line[ws+1]=='/') break; // comentarios
				if (line[ws]=='\''||line[ws]=='\"') ws = SkipString(line,ws,len); // comillas
				else {
					if (line[ws]==';') break;
					else if (line[ws]==':' && line[ws+1]!='=') {cur-=4; break;}
				}
				ws++;
			}
		}
	}
	if (btype==BT_SEGUN && GetLineEndPosition(l)==GetLineIndentPosition(l)) cur-=4;
	if (cur<0) cur=0;
	if (GetLineIndentation(l)==cur) return false;
	if (GetCurrentPos()==GetLineIndentPosition(l)) {
		SetLineIndentation(l,cur);
		SetSelection(GetLineIndentPosition(l),GetLineIndentPosition(l));
	} else
		SetLineIndentation(l,cur);
	return true;
}

int mxSource::GetIndentLevel(int l, bool goup, int &e_btype, bool diff_proc_sub_func) {
	e_btype = BT_NONE;
	if (goup) while (l>=1 && !LineHasSomething(l-1)) l--;
	if (l<1) return 0;
	wxString line=GetLine(l-1);
	line<<" ";
	int cur=GetLineIndentation(l-1);
	int n=line.Len();
	bool comillas=false;
	bool first_word=true; // para saber si es la primer palabra de la instruccion
	bool ignore_next=false; // para que despues de Fin se saltee lo que sigue
	int wstart=0; // para guardar donde empezaba la palabra
	for (int i=0;i<n;i++) {
		wxChar c=line[i];
		if (c=='\'' || c=='\"') {
			comillas=!comillas;
		} else if (!comillas) {
			if (c=='/' && i+1<n && line[i+1]=='/')  return cur;
			if (c==':' && line[i+1]!='=') { cur+=4; e_btype=BT_CASO; }
			else if (!EsLetra(c,true)) {
				if (wstart+1<i) {
					if (ignore_next) {
						ignore_next=false;
					} else {
						wxString word = line.SubString(wstart,i-1);
						MakeUpper(word);
						if (word=="SI") { 
							if (cfg_lang[LS_LAZY_SYNTAX]) {
								int y=i+1; while (line[y]==' '||line[y]=='\t') y++; 
								if (toupper(line[y])!='E' || toupper(line[y+1])!='S' || (line[y+2]!=' '&&line[y+2]!='\t'))
									{ cur+=4; e_btype=BT_SI; }
							} else 	{ cur+=4; e_btype=BT_SI; }
						}
						else if (word=="SINO") { cur+=4; e_btype=BT_SINO; }
						else if (word=="PROCESO") { cur+=4; e_btype=BT_PROCESO; }
						else if (word=="ALGORITMO") { cur+=4; e_btype=diff_proc_sub_func?BT_ALGORITMO:BT_PROCESO; }
						else if (word=="FUNCION"||word==_Z("FUNCIÓN")) { cur+=4; e_btype=diff_proc_sub_func?BT_FUNCION:BT_PROCESO; }
						else if (word=="SUBPROCESO") { cur+=4; e_btype=diff_proc_sub_func?BT_SUBPROCESO:BT_PROCESO; }
						else if (word=="SUBALGORITMO") { cur+=4; e_btype=diff_proc_sub_func?BT_SUBALGORITMO:BT_PROCESO; }
						else if (word=="MIENTRAS" && !(i+4<n && line.SubString(wstart,i+4).Upper()=="MIENTRAS QUE ")) { cur+=4; e_btype=BT_MIENTRAS; }
						else if (word=="SEGUN"||word==_Z("SEGÚN")) { cur+=8; e_btype=BT_SEGUN; }
						else if (word=="PARA") { cur+=4; e_btype=BT_PARA;	}
						else if (word=="REPETIR"||(first_word && word=="HACER")) { cur+=4; e_btype=BT_REPETIR; }
						else if (word=="FIN") { ignore_next=true; e_btype=BT_NONE; }
						else if (e_btype!=BT_NONE && (word=="FINSEGUN"||word==_Z("FINSEGÚN")||word=="FINPARA"||word=="FINMIENTRAS"||word=="FINSI"||word=="MIENTRAS"||word=="FINPROCESO"||word=="FINALGORITMO"||word=="FINSUBALGORITMO"||word=="FINSUBPROCESO"||word=="FINFUNCION"||word==_Z("FINFUNCIÓN"))) {
							if (e_btype==BT_SEGUN) cur-=4;
							e_btype=BT_NONE; cur-=4;
						}
						first_word=false;
					}
				}
				wstart=i+1;
				if (c==';') first_word=true;
			}
		}
	}
	return cur;
}

void mxSource::Indent(int l1, int l2) {
	BeginUndoAction();
	bool goup = true;
	for (int i=l1;i<=l2;i++) {
		IndentLine(i,goup);
		goup = !LineHasSomething(i);
	}
	EndUndoAction();
}

void mxSource::UnExample() {
	SetReadOnly(is_example=false);
}

void mxSource::SetWords() {
	// setear palabras claves para el coloreado
	SetKeyWords (0,_Z(cfg_lang.GetKeywords().c_str()));
	SetKeyWords (1,_Z(cfg_lang.GetFunctions().c_str()));
	SetKeyWords (3, ""); // para resaltar las variables
}

void mxSource::SetCalltips() {
	calltips_functions.clear(); calltips_instructions.clear();
	
	calltips_instructions.push_back(calltip_text(_Z("FUNCIÓN"),     _Z("{variable de retorno} <- {nombre} ( {lista de argumentos separados por coma} )\n{nombre} ( {lista de argumentos, separados por coma} )")));
	calltips_instructions.push_back(calltip_text(_Z("FUNCION"),     _Z("{variable de retorno} <- {nombre} ( {lista de argumentos separados por coma} )\n{nombre} ( {lista de argumentos, separados por coma} )")));
	calltips_instructions.push_back(calltip_text(_Z("SUBPROCESO"),  _Z("{variable de retorno} <- {nombre} ( {lista de argumentos separados por coma} )\n{nombre} ( {lista de argumentos, separados por coma} )")));
	calltips_instructions.push_back(calltip_text(_Z("SUBALGORITMO"),_Z("{variable de retorno} <- {nombre} ( {lista de argumentos separados por coma} )\n{nombre} ( {lista de argumentos, separados por coma} )")));
	calltips_instructions.push_back(calltip_text(_Z("LEER"),   _Z("{una o mas variables, separadas por comas}")));
	calltips_instructions.push_back(calltip_text(_Z("DEFINIR"),_Z("{una o mas variables, separadas por comas}")));
	calltips_instructions.push_back(calltip_text(_Z("ESPERAR"),_Z("{\"Tecla\" o intervalo de tiempo}")));
	calltips_instructions.push_back(calltip_text(_Z("ESCRIBIR"),    _Z("{una o mas expresiones, separadas por comas}")));
	if (cfg_lang[LS_LAZY_SYNTAX]) {
		calltips_instructions.push_back(calltip_text(_Z("MOSTRAR"), _Z("{una o mas expresiones, separadas por comas}")));
		calltips_instructions.push_back(calltip_text(_Z("IMPRIMIR"),_Z("{una o mas expresiones, separadas por comas}")));
	}
	calltips_instructions.push_back(calltip_text(_Z("MIENTRAS"),_Z("{condición, expresion lógica}")));
	calltips_instructions.push_back(calltip_text(_Z("QUE"),("{condición, expresion lógica}")));
	calltips_instructions.push_back(calltip_text(_Z("PARA"),("{asignación inicial: variable<-valor}")));
	calltips_instructions.push_back(calltip_text(_Z("DESDE"),_Z("{valor inicial}")));
	calltips_instructions.push_back(calltip_text(_Z("HASTA"),_Z("{valor final}"),true));
	calltips_instructions.push_back(calltip_text(_Z("PASO"),_Z("{valor del paso}")));
	calltips_instructions.push_back(calltip_text(_Z("SI"),_Z("{condicion, expresión lógica}")));
	calltips_instructions.push_back(calltip_text(_Z("ENTONCES"),_Z("{acciones por verdadero}")));
	calltips_instructions.push_back(calltip_text(_Z("SINO"),_Z("{acciones por falso}")));
	calltips_instructions.push_back(calltip_text(_Z("SEGUN"),_Z(cfg_lang[LS_INTEGER_ONLY_SWITCH]?"{variable o expresión numérica entera}":"{variable o expresión de control}")));
	calltips_instructions.push_back(calltip_text(_Z("SEGÚN"),_Z(cfg_lang[LS_INTEGER_ONLY_SWITCH]?"{variable o expresión numérica entera}":"{variable o expresión de control}")));
	if (cfg_lang[LS_LAZY_SYNTAX]) {
		calltips_instructions.push_back(calltip_text(_Z("OPCION"),_Z("{posible valor para la expresión de control}")));
		calltips_instructions.push_back(calltip_text(_Z("OPCIÓN"),_Z("{posible valor para la expresión de control}")));
		calltips_instructions.push_back(calltip_text(_Z("SIES"),  _Z("{posible valor para la expresión de control}")));
		calltips_instructions.push_back(calltip_text(_Z("CASO"),  _Z("{posible valor para la expresión de control}")));
	}
	
	calltips_functions.push_back(calltip_text(_Z("ALEATORIO"),_Z("{valor mínimo}, {valor máximo}")));
	calltips_functions.push_back(calltip_text(_Z("AZAR"),_Z("{expresión numérica entera positiva (máximo valor posible +1)}")));
	calltips_functions.push_back(calltip_text(_Z("TRUNC"),_Z("{expresión numérica}")));
	calltips_functions.push_back(calltip_text(_Z("REDON"),_Z("{expresión numérica}")));
	calltips_functions.push_back(calltip_text(_Z("RC"),_Z("{expresión numérica no negativa}")));
	calltips_functions.push_back(calltip_text(_Z("RAIZ"),_Z("{expresión numérica no negativa}")));
	calltips_functions.push_back(calltip_text(_Z("ABS"),_Z("{expresión numérica}")));
	calltips_functions.push_back(calltip_text(_Z("EXP"),_Z("{expresión numérica}")));
	calltips_functions.push_back(calltip_text(_Z("LN"),_Z("{expresión numérica positiva}")));
	calltips_functions.push_back(calltip_text(_Z("COS"),_Z("{ángulo en radianes}")));
	calltips_functions.push_back(calltip_text(_Z("SIN"),_Z("{ángulo en radianes}")));
	calltips_functions.push_back(calltip_text(_Z("TAN"),_Z("{ángulo en radianes}")));
	calltips_functions.push_back(calltip_text(_Z("ACOS"),_Z("{expresión numérica (en el intervalo [-1;+1])}")));
	calltips_functions.push_back(calltip_text(_Z("ASIN"),_Z("{expresión numérica (en el intervalo [-1;+1])}")));
	calltips_functions.push_back(calltip_text(_Z("ATAN"),_Z("{expresión numérica}")));
	if (cfg_lang[LS_ENABLE_STRING_FUNCTIONS]) {
		calltips_functions.push_back(calltip_text(_Z("CONVERTIRANÚMERO"),_Z("{cadena}")));
		calltips_functions.push_back(calltip_text(_Z("CONVERTIRANUMERO"),_Z("{cadena}")));
		calltips_functions.push_back(calltip_text(_Z("MAYUSCULAS"),_Z("{cadena}")));
		calltips_functions.push_back(calltip_text(_Z("MAYÚSCULAS"),_Z("{cadena}")));
		calltips_functions.push_back(calltip_text(_Z("MINUSCULAS"),_Z("{cadena}")));
		calltips_functions.push_back(calltip_text(_Z("MINÚSCULAS"),_Z("{cadena}")));
		calltips_functions.push_back(calltip_text(_Z("CONCATENAR"),_Z("{dos cadenas}")));
		calltips_functions.push_back(calltip_text(_Z("LONGITUD"),_Z("{cadena}")));
		calltips_functions.push_back(calltip_text(_Z("SUBCADENA"),_Z("{cadena}, {posición desde}, {posición hasta}")));
		calltips_functions.push_back(calltip_text(_Z("CONVERTIRATEXTO"),_Z("{expresión numérica}")));
	}
}

void mxSource::SetAutocompletion() {
	// setear reglas para el autocompletado
	comp_list.clear();
	
	comp_list.push_back(comp_list_item("Proceso","Proceso ",""));
	if (cfg_lang[LS_ENABLE_USER_FUNCTIONS]) {
		comp_list.push_back(comp_list_item("Algoritmo","Algoritmo ",""));
		comp_list.push_back(comp_list_item("Funcion","Funcion ",""));
		comp_list.push_back(comp_list_item("SubAlgoritmo","SubAlgoritmo ",""));
		comp_list.push_back(comp_list_item("SubProceso","SubProceso ",""));
		comp_list.push_back(comp_list_item("Por Valor","Por Valor","SubAlgoritmo"));
		comp_list.push_back(comp_list_item("Por Valor","Por Valor","SubProceso"));
		comp_list.push_back(comp_list_item("Por Valor","Por Valor","Funcion"));
		comp_list.push_back(comp_list_item("Por Referencia","Por Referencia","SubAlgoritmo"));
		comp_list.push_back(comp_list_item("Por Referencia","Por Referencia","SubProceso"));
		comp_list.push_back(comp_list_item("Por Referencia","Por Referencia","Funcion"));
	}
	comp_list.push_back(comp_list_item("Fin Proceso","Fin Proceso\n",""));
	comp_list.push_back(comp_list_item("FinProceso","FinProceso\n",""));
	comp_list.push_back(comp_list_item("Fin Algoritmo","Fin Algoritmo\n",""));
	comp_list.push_back(comp_list_item("FinAlgoritmo","FinAlgoritmo\n",""));
	if (cfg_lang[LS_ENABLE_USER_FUNCTIONS]) {
		comp_list.push_back(comp_list_item("Fin SubProceso","Fin SubProceso\n",""));
		comp_list.push_back(comp_list_item("FinSubProceso","FinSubProceso\n",""));
		comp_list.push_back(comp_list_item("FinFuncion","FinFuncion\n",""));
		comp_list.push_back(comp_list_item("FinSubAlgoritmo","FinSubAlgoritmo\n",""));
	}
	
	comp_list.push_back(comp_list_item("Escribir","Escribir ",""));
	if (cfg_lang[LS_LAZY_SYNTAX]) {
		comp_list.push_back(comp_list_item("Imprimir","Imprimir ",""));
		comp_list.push_back(comp_list_item("Mostrar","Mostrar ",""));
	}
	comp_list.push_back(comp_list_item("Sin Saltar","Sin Saltar","Escribir"));
	comp_list.push_back(comp_list_item("Sin Saltar","Sin Saltar","Mostrar"));
	comp_list.push_back(comp_list_item("Sin Saltar","Sin Saltar","Imprimir"));
	comp_list.push_back(comp_list_item("Leer","Leer ",""));
	
	comp_list.push_back(comp_list_item("Esperar","Esperar ",""));
	comp_list.push_back(comp_list_item("Segundos","Segundos;","Esperar"));
	comp_list.push_back(comp_list_item("Milisegundos","Milisegundos;","Esperar"));
	comp_list.push_back(comp_list_item("Tecla","Tecla;","Esperar"));
	comp_list.push_back(comp_list_item("Esperar Tecla","Esperar Tecla;",""));
	comp_list.push_back(comp_list_item("Borrar Pantalla","Borrar Pantalla;",""));
	comp_list.push_back(comp_list_item("Limpiar Pantalla","Limpiar Pantalla;",""));

	
	comp_list.push_back(comp_list_item("Dimension","Dimension ",""));
	comp_list.push_back(comp_list_item("Definir","Definir ",""));
	comp_list.push_back(comp_list_item("Como Real","Como Real;","Definir"));
	comp_list.push_back(comp_list_item("Como Caracter","Como Caracter;","Definir"));
	comp_list.push_back(comp_list_item("Como Entero","Como Entero;","Definir"));
	comp_list.push_back(comp_list_item("Como Logico","Como Logico;","Definir"));
	
	comp_list.push_back(comp_list_item("Entonces","Entonces\n",""));
	comp_list.push_back(comp_list_item("Entonces","Entonces\n","Si"));
	comp_list.push_back(comp_list_item("SiNo","SiNo\n",""));
	comp_list.push_back(comp_list_item("Fin Si","Fin Si\n",""));
	comp_list.push_back(comp_list_item("FinSi","FinSi\n",""));
	
	comp_list.push_back(comp_list_item("Mientras","Mientras ",""));
	comp_list.push_back(comp_list_item("Hacer","Hacer\n","Mientras"));
	comp_list.push_back(comp_list_item("Fin Mientras","Fin Mientras\n",""));
	comp_list.push_back(comp_list_item("FinMientras","FinMientras\n",""));
	
	comp_list.push_back(comp_list_item("Para","Para ",""));
	if (cfg_lang[LS_LAZY_SYNTAX]) {
		comp_list.push_back(comp_list_item("Para Cada","Para Cada ",""));
		comp_list.push_back(comp_list_item("Desde","Desde ","Para"));
		comp_list.push_back(comp_list_item("Hasta","Hasta ","Para"));
	}
	comp_list.push_back(comp_list_item("Con Paso","Con Paso ","Para"));
	comp_list.push_back(comp_list_item("Hacer","Hacer\n","Para"));
	if (cfg_lang[LS_LAZY_SYNTAX]) comp_list.push_back(comp_list_item("Cada ","Cada ","Para"));
	comp_list.push_back(comp_list_item("Fin Para","Fin Para\n",""));
	comp_list.push_back(comp_list_item("FinPara","FinPara\n",""));
	
	comp_list.push_back(comp_list_item("Repetir","Repetir\n",""));
	comp_list.push_back(comp_list_item("Hacer","Hacer\n",""));
	comp_list.push_back(comp_list_item("Hasta Que","Hasta Que ",""));
	if (cfg_lang[LS_LAZY_SYNTAX]) comp_list.push_back(comp_list_item("Mientras Que","Mientras Que ",""));
	
	comp_list.push_back(comp_list_item("Segun","Segun ",""));
	if (cfg_lang[LS_LAZY_SYNTAX]) {
		comp_list.push_back(comp_list_item("Opcion","Opcion ",""));
		comp_list.push_back(comp_list_item("Caso","Caso ",""));
	}
	comp_list.push_back(comp_list_item("De Otro Modo:","De Otro Modo:\n",""));
	comp_list.push_back(comp_list_item("FinSegun","FinSegun\n",""));
	comp_list.push_back(comp_list_item("Fin Segun","Fin Segun\n",""));

	comp_list.push_back(comp_list_item("Aleatorio","Aleatorio(","*"));
	if (cfg_lang[LS_ENABLE_STRING_FUNCTIONS]) {
		comp_list.push_back(comp_list_item("ConvertirATexto","ConvertirATexto(","*"));
		comp_list.push_back(comp_list_item("ConvertirANumero","ConvertirANumero(","*"));
		comp_list.push_back(comp_list_item("Concatenar","Concatenar(","*"));
		comp_list.push_back(comp_list_item("Longitud","Longitud(","*"));
		comp_list.push_back(comp_list_item("Mayusculas","Mayusculas(","*"));
		comp_list.push_back(comp_list_item("Minusculas","Minusculas(","*"));
		comp_list.push_back(comp_list_item("Subcadena","Subcadena(","*"));
	}
	
	comp_list.push_back(comp_list_item("Verdadero","Verdadero","*"));
	comp_list.push_back(comp_list_item("Falso","Falso","*"));
	
	if (cfg_lang[LS_COLOQUIAL_CONDITIONS]) {
		comp_list.push_back(comp_list_item("Es Cero","Es Cero","*"));
		comp_list.push_back(comp_list_item("Es Distinto De","Es Distinto De ","*"));
		comp_list.push_back(comp_list_item("Es Divisible Por","Es Divisible Por ","*"));
		comp_list.push_back(comp_list_item("Es Entero","Es Entero","*"));
		comp_list.push_back(comp_list_item("Es Igual A","Es Igual A ","*"));
		comp_list.push_back(comp_list_item("Es Impar","Es Impar","*"));
		comp_list.push_back(comp_list_item("Es Mayor O Igual A","Es Mayor O Igual A ","*"));
		comp_list.push_back(comp_list_item("Es Mayor Que","Es Mayor Que ","*"));
		comp_list.push_back(comp_list_item("Es Menor O Igual A","Es Menor O Igual A ","*"));
		comp_list.push_back(comp_list_item("Es Menor Que","Es Menor Que ","*"));
		comp_list.push_back(comp_list_item("Es Multiplo De","Es Multiplo De ","*"));
		comp_list.push_back(comp_list_item("Es Negativo","Es Negativo","*"));
		comp_list.push_back(comp_list_item("Es Par","Es Par","*"));
		comp_list.push_back(comp_list_item("Es Positivo","Es Positivo","*"));
		
	}
	if (cfg_lang[LS_LAZY_SYNTAX]) {
		comp_list.push_back(comp_list_item("Es Real","Es Real;","Es"));
		comp_list.push_back(comp_list_item("Es Caracter","Es Caracter;","Es"));
		if (!cfg_lang[LS_COLOQUIAL_CONDITIONS]) comp_list.push_back(comp_list_item("Es Entero","Es Entero;","Es"));
		comp_list.push_back(comp_list_item("Es Logico","Es Logico;","Es"));
		comp_list.push_back(comp_list_item("Son Reales","Son Reales;","Son"));
		comp_list.push_back(comp_list_item("Son Caracteres","Son Caracteres;","Son"));
		comp_list.push_back(comp_list_item("Son Enteros","Son Enteros;","Son"));
		comp_list.push_back(comp_list_item("Son Logicos","Son Logicos;","Son"));
	}
	
	sort(comp_list.begin(),comp_list.end());
}

void mxSource::ReloadFromTempPSD (bool check_syntax) {
	_LOG("mxSource::ReloadFromTempPSD "<<this);
	mask_timers=true;
	int cl=GetCurrentLine(), cp=GetCurrentPos(); cp-=PositionFromLine(cl);
	wxString file=GetTempFilenamePSD();
	bool isro=GetReadOnly();
	if (isro) SetReadOnly(false);
	LoadFile(file);
	// convertir en campos lo que esté incompleto
	for (int i=0;i<GetLineCount();i++) {
		wxString line=GetLine(i); 
		int l=line.Len(), j0, l0=PositionFromLine(i);
		bool comillas=false, campo=false;
		for(int j=0;j<l;j++) { 
			if (line[j]=='\''||line[j]=='\"') comillas=!comillas;
			else if (!comillas) {
				if (campo) {
					if (line[j]=='}') {
						SetTargetStart(l0+j0);
						SetTargetEnd(l0+j+1);
						ReplaceTarget(GetTextRange(l0+j0+1,l0+j));
						SetFieldIndicator(l0+j0,l0+j-1,false);
						l0-=2; // para compenzar el desfazaje entre line y la linea real
						campo=false;
					}
				} else {
					if (line[j]=='{') {
						j0=j;
						campo=true;
					}
				}
			}
		}
	}
	
	// reestablecer la posición del cursor en el nuevo código
	int lc=GetLineCount(); if (cl>=lc) cl=lc-1;
	int pl=PositionFromLine(cl);
	int le=GetLineEndPosition(cl)-pl; if (cp>=le) cp=le-1;
	SetSelection(pl+cp,pl+cp);
	
	SetModify(true);
	if (isro) SetReadOnly(true);
	if (run_socket) UpdateRunningTerminal();
	if (check_syntax) DoRTSyntaxChecking();
	mask_timers=false;
}

wxSocketBase * mxSource::GetFlowSocket ( ) {
	return flow_socket;
}

void mxSource::SetFlowSocket ( wxSocketBase *s ) {
	if ((flow_socket=s)) {
		SetReadOnly(true);
		SetStatus(STATUS_FLOW);
	} else {
		if (!is_example) SetReadOnly(false);
		status_should_change=true; 
		SetStatus();
	}
}

void mxSource::SetRunSocket ( wxSocketBase *s ) {
	run_socket=s;
}

void mxSource::SetDebugPause() {
	if (debug_line!=-1) {
		MarkerDeleteHandle(debug_line_handler_1);
		MarkerDeleteHandle(debug_line_handler_2);
		debug_line_handler_1=MarkerAdd(debug_line,MARKER_DEBUG_PAUSE_ARROW);
		debug_line_handler_2=MarkerAdd(debug_line,MARKER_DEBUG_PAUSE_BACK);
	}
}

void mxSource::SetDebugLine(int l, int i) {
	if (debug_line!=-1) {
		MarkerDeleteHandle(debug_line_handler_1);
		MarkerDeleteHandle(debug_line_handler_2);
	}
	if ((debug_line=l)!=-1) {
		debug_line_handler_1=MarkerAdd(l,MARKER_DEBUG_RUNNING_ARROW);
		debug_line_handler_2=MarkerAdd(l,MARKER_DEBUG_RUNNING_BACK);
		EnsureVisibleEnforcePolicy(l);
		if (i!=-1) SelectInstruccion(l,i);
	}
	if (flow_socket) {
		wxString s; s<<"step "<<l+1<<":"<<i+1<<"\n";
		flow_socket->Write(s.c_str(),s.Len());
	}
}

bool mxSource::HaveComments() {
	for (int j,i=0;i<GetLength();i++) {
		j=GetStyleAt(i);
		if (j==wxSTC_C_COMMENT||j==wxSTC_C_COMMENTDOC||j==wxSTC_C_COMMENTLINE||j==wxSTC_C_COMMENTLINEDOC) return true;
	}
	return false;
}

bool mxSource::LineHasSomething ( int l ) {
	int i1=PositionFromLine(l);
	int i2=GetLineEndPosition(l);
	for (int i=i1;i<=i2;i++) {
		char c=GetCharAt(i); int s=GetStyleAt(i);
		if (c!='\n' && c!=' ' && c!='\r' && c!='\t' && s!=wxSTC_C_COMMENT && s!=wxSTC_C_COMMENTDOC && s!=wxSTC_C_COMMENTLINE && s!=wxSTC_C_COMMENTLINEDOC && s!=wxSTC_C_COMMENTDOCKEYWORD && s!=wxSTC_C_COMMENTDOCKEYWORDERROR) return true;
	}
	return false;
}

void mxSource::SetPageText (wxString ptext) {
	main_window->notebook->SetPageText(main_window->notebook->GetPageIndex(this),(page_text=ptext)+(GetModify()?"*":""));
}

wxString mxSource::GetPageText ( ) {
	return page_text;
}

void mxSource::OnSavePointReached (wxStyledTextEvent & evt) {
	main_window->notebook->SetPageText(main_window->notebook->GetPageIndex(this),page_text);	
}

void mxSource::OnSavePointLeft (wxStyledTextEvent & evt) {
	main_window->notebook->SetPageText(main_window->notebook->GetPageIndex(this),page_text+"*");	
}

vector<int> &mxSource::FillAuxInstr(int _l) {
	static vector<int> v; v.clear();
	wxString s=GetLine(_l);
	int i=0,len=s.Len(),last_ns=1;
	bool starting=true,comillas=false;
	while (i<len) {
		if (s[i]=='\''||s[i]=='\"') comillas=!comillas;
		else if (!comillas && i+1<len && s[i]=='/' && s[i+1]=='/') break;
		if (s[i]!=' '&&s[i]!='\t') {
			if (!comillas) {
				if (starting) { v.push_back(i); starting=false; }
				if (s[i]==';'||s[i]==':'||s[i]=='\n') { v.push_back(last_ns); starting=true; }
				else if (wxTolower(s[i])=='e' && i+8<len && s.Mid(i,8).Upper()=="ENTONCES" && !EsLetra(_C(s[i+8]),true)) {
					if (v.back()!=i) { v.push_back(last_ns); v.push_back(i); } v.push_back(i+8); 
					i+=7; starting=true;
				}
				else if (wxTolower(s[i])=='h' && i+5<len && s.Mid(i,5).Upper()=="HACER" && !EsLetra(s[i+5],true)) {
					if (v.back()!=i) { v.push_back(last_ns); v.push_back(i); } v.push_back(i+5); 
					i+=4; starting=true;
				}
				else if (wxTolower(s[i])=='s' && i+4<len && s.Mid(i,4).Upper()=="SINO" && !EsLetra(s[i+4],true)) {
					if (v.back()!=i) { v.push_back(last_ns); v.push_back(i); } v.push_back(i+4); 
					i+=3; starting=true;
				}
			}
			last_ns=i+1;
		}
		i++;
	}
	if (comillas) last_ns=len;
	if (v.empty()) v.push_back(last_ns);
	v.push_back(last_ns);
	return v;
}

void mxSource::SelectInstruccion (int _l, int _i) {
	vector<int> &v=FillAuxInstr(_l);
	_l=PositionFromLine(_l);
	if (2*_i>int(v.size())) SetSelection(_l+v[0],_l+v[v.size()-1]);
	else SetSelection(_l+v[2*_i+1],_l+v[2*_i]);
}

void mxSource::DoRealTimeSyntax (RTSyntaxManager::Info *args) {
	RTSyntaxManager::Process(this,args);
//	SetStatus(); // si lo hago aca setea el estado antes de terminar de analizar todo, mejor que lo haga el rt_syntax
}

void mxSource::ClearErrorData() {
	if (flow_socket) flow_socket->Write("errors reset\n",13);
	rt_errors.clear();
	SetIndics(0,GetLength(),INDIC_ERROR_1,false);
	SetIndics(0,GetLength(),INDIC_ERROR_2,false);
#ifdef WX3
	AnnotationClearAll();
#endif
}

void mxSource::MarkError(wxString line) {
	long l=-1,i=-1,n;
	line.AfterFirst(':').BeforeFirst(':').AfterLast(' ').ToLong(&n);
	line.AfterFirst(' ').BeforeFirst(' ').ToLong(&l);
	line.BeforeFirst(':').AfterLast(' ').BeforeLast(')').ToLong(&i);
	line=line.AfterFirst(':').AfterFirst(':').Mid(1);
	MarkError(l-1,i-1,n,line,line.StartsWith("Falta cerrar "));
}

/**
* @param l número de linea del error
* @param i número instrucción dentro de esa linea del error
* @param n número de error (para obtener su descripción)
* @param str texto del mensaje corto de error
* @param specil indica que va de otro color (se usa para los "falta cerrar....")
**/
void mxSource::MarkError(int l, int i, int n, wxString str, bool special) {
	if (l<0 || l>=GetLineCount()) return; // el error debe caer en una linea valida
	vector<int> &v=FillAuxInstr(l);
	int pos =PositionFromLine(l)+v[2*i], len = v[2*i+1]-v[2*i];
	// ver que no sea culpa de una plantilla sin completar
	for(int p=0;p<len;p++) { 
		int indics = _if_wx3_else(IndicatorAllOnFor(pos+p),GetStyleAt(pos+p));
		if (indics&indic_to_mask[INDIC_FIELD]) return;
	}
	// ok, entonces agregarlo como error
	while (l>=int(rt_errors.size())) rt_errors.push_back(rt_err()); // hacer lugar en el arreglo de errores por linea si no hay
	rt_errors[l].Add(i,n,str); // guardarlo en el vector de errores
	if (flow_socket) { // avisarle al diagrama de flujo
		wxString msg("errors add "); msg<<l+1<<':'<<i+1<<' '<<str<<'\n';
		flow_socket->Write(msg.c_str(),msg.Len());
	}
	
	// marcarlo en el pseudocódigo subrayando la instrucción y poniendo la cruz en el margen
	if (int(v.size())<=2*i+1) return;
	if (!(MarkerGet(l)&(1<<MARKER_ERROR_LINE))) MarkerAdd(l,MARKER_ERROR_LINE);
	// agregar el error como anotacion y subrayar la instruccion
#ifdef WX3
	if (config->rt_annotate) {
		wxString prev = AnnotationGetText(l); if (!prev.IsEmpty()) prev<<"\n";
		AnnotationSetText(l,prev<<_if_unicode(wxString(L"\u25ba ")<<)"en inst. "<<i+1<<": "<<str);
		AnnotationSetStyle(l,ANNOTATION_STYLE);
	}
#endif
	SetIndics(pos,len,special?INDIC_ERROR_2:INDIC_ERROR_1,true);
}

void mxSource::StartRTSyntaxChecking ( ) {
	rt_running=true; DoRTSyntaxChecking();
}

void mxSource::StopRTSyntaxChecking ( ) {
	rt_running=false; rt_timer->Stop(); 
	ClearErrorData(); ClearErrorMarks(); ClearBlocks(); UnHighLightBlock();
}

void mxSource::OnTimer (wxTimerEvent & te) {
//	_LOG("mxSource::OnTimer in");
	wxObject *obj = _wxEvtTimer_to_wxTimerPtr(te);
#ifdef _AUTOINDENT
	if (obj==indent_timer) {
		BeginUndoAction();
		int cl = GetCurrentLine(), p0, p1;
		for(int i = to_indent_first_line; i<int(to_indent.size()); i++) {
			if (!to_indent[i]) continue;
			if (i==cl) {
				int ip = GetLineIndentPosition(cl);
				p0 = GetSelectionStart()-ip;
				p1 = GetSelectionEnd()-ip;
			}
			if (IndentLine(i)) to_indent[i+1] = true;
			if (i==cl) {
				int ip = GetLineIndentPosition(cl);
				p0+=ip; p1+=ip;
				int pb = PositionFromLine(cl);
				if (p0<pb) p0=pb; if (p1<pb) p1=pb;
				SetSelection(p0,p1);
			}
		}
		to_indent.clear();
		to_indent_first_line = 99999999;
		EndUndoAction();
	} else 
#endif
	if (obj==flow_timer) {
		_LOG("mxSource::OnTimes(flow) "<<this);
		UpdateFromFlow();
	} else if (obj==rt_timer) {
		_LOG("mxSource::OnTimes(rt) "<<this);
		if (main_window->GetCurrentSource()!=this) {
//			_LOG("mxSource::OnTimer out");
			return; // solo si tiene el foco
		}
		if (!just_created) 
			DoRealTimeSyntax(); 
		HighLightBlock();
	} else if (obj==reload_timer) {
		_LOG("mxSource::OnTimes(reload) "<<this);
		if (run_socket && rt_errors.empty()) UpdateRunningTerminal();
	}
//	_LOG("mxSource::OnTimer out");
}

void mxSource::ShowCalltip (int pos, const wxString & l, bool is_error) {
	if (_if_wx3_else(!is_error||!config->rt_annotate,true)) {
		if (!current_calltip.is_error && is_error && CallTipActive()) return; // que un error no tape una ayuda
		// muestra el tip
		current_calltip.pos=pos;
		current_calltip.is_error=is_error;
		CallTipShow(pos,l);
	}
	// si era un error y está el panel de ayuda rápida muestra también la descripción larga
	if (!is_error || !main_window->QuickHelp().IsVisible()) return;
	int il=LineFromPosition(pos);
	if (il<0||il>int(rt_errors.size())) return;
	rt_err &e=rt_errors[il];
	if (e.is) main_window->QuickHelp().ShowRTError(e.n,e.s);
}

void mxSource::ShowRealTimeError (int pos, const wxString & l) {
	ShowCalltip(pos,l,true);
}

void mxSource::HideCalltip (bool if_is_error, bool if_is_not_error) {
	if (current_calltip.is_error && if_is_error) {
		CallTipCancel();
		main_window->QuickHelp().ShowRTResult(!rt_errors.empty());
		
	} else if (!current_calltip.is_error && if_is_not_error) CallTipCancel();
}

int mxSource::GetIndent(int line) {
	int i=PositionFromLine(line), n=0;
	while (true) {
		char c=GetCharAt(i++);
		if (c==' ') n++; 
		else if (c=='\t') n+=config->tabw;
		else return n;
	}
}

void mxSource::TryToAutoCloseSomething (int l) {
	// ver si se abre una estructura
	int btype;
	GetIndentLevel(l,false,btype,true); 
	// buscar la siguiente linea no nula
	int l2=l+1,ln=GetLineCount();
	if (btype==BT_NONE||btype==BT_SINO||btype==BT_CASO) return;
	while (l2<ln && GetLineEndPosition(l2)==GetLineIndentPosition(l2)) l2++;
	// comparar los indentados para ver si la siguiente esta dentro o fuera
	int i1=GetIndent(l-1), i2=GetIndent(l2);
	if (l2<ln && i1<i2) return; // si estaba dentro no se hace nada
	// ver que dice la siguiente para que no coincida con lo que vamos a agregar
	wxString sl2=i2<i1?"":GetLine(l2); MakeUpper(sl2); 
	int i=0, sl=sl2.Len(); 
	while (i<sl && (sl2[i]==' '||sl2[i]=='\t'))i++;
	if (i) sl2.Remove(0,i);
	// agregar FinAlgo
	if (btype==BT_PROCESO) {
		if (sl2.StartsWith("FINPROCESO") || sl2.StartsWith("FIN PROCESO")) return;
		InsertText(PositionFromLine(l+1),"FinProceso\n");
		IndentLine(l+1,true);
	} else if (btype==BT_SUBPROCESO) {
		if (sl2.StartsWith("FINSUBPROCESO") || sl2.StartsWith("FIN SUBPROCESO")) return;
		InsertText(PositionFromLine(l+1),"FinSubProceso\n");
		IndentLine(l+1,true);
	}else if (btype==BT_ALGORITMO) {
		if (sl2.StartsWith("FINALGORITMO") || sl2.StartsWith("FIN ALGORITMO")) return;
		InsertText(PositionFromLine(l+1),"FinAlgoritmo\n");
		IndentLine(l+1,true);
	} else if (btype==BT_SUBALGORITMO) {
		if (sl2.StartsWith("FINSUBALGORITMO") || sl2.StartsWith("FIN SUBALGORITMO")) return;
		InsertText(PositionFromLine(l+1),"FinSubAlgoritmo\n");
		IndentLine(l+1,true);
	} else if (btype==BT_FUNCION) {
		if (sl2.StartsWith("FINFUNCION") || sl2.StartsWith("FIN FUNCION")) return;
		InsertText(PositionFromLine(l+1),"FinFuncion\n");
		IndentLine(l+1,true);
	} else if (btype==BT_PARA) {
		if (sl2.StartsWith("FINPARA") || sl2.StartsWith("FIN PARA")) return;
		InsertText(PositionFromLine(l+1),"FinPara\n");
		IndentLine(l+1,true);
	} else if (btype==BT_SI) {
		if (sl2.StartsWith("FINSI") || sl2.StartsWith("FIN SI") || sl2.StartsWith("SINO")) return;
		InsertText(PositionFromLine(l+1),"FinSi\n");
		IndentLine(l+1,true);
	} else if (btype==BT_REPETIR) {
		if (sl2.StartsWith("HASTA QUE") || sl2.StartsWith("MIENTRAS QUE")) return;
		InsertText(PositionFromLine(l+1),"Hasta Que \n");
		IndentLine(l+1,true);
	} else if (btype==BT_MIENTRAS) {
		if (sl2.StartsWith("FINMIENTRAS") || sl2.StartsWith("FIN MIENTRAS")) return;
		InsertText(PositionFromLine(l+1),"FinMientras\n");
		IndentLine(l+1,true);
	} else if (btype==BT_SEGUN) {
		if (sl2.StartsWith("FINSEG") || sl2.StartsWith("FIN SEG")) return;
		InsertText(PositionFromLine(l+1),"FinSegun\n");
		IndentLine(l+1,true);
	}
}


void mxSource::OnToolTipTimeOut (wxStyledTextEvent &event) {
//	if (current_calltip.is_dwell) HideCalltip(true);
}

void mxSource::OnToolTipTime (wxStyledTextEvent &event) {
	
	if (main_window->GetCurrentSource()!=this) return;
	if (debug->debugging && debug->paused) {
		int p=event.GetPosition(); if (p<0) return; int s=GetStyleAt(p);
		wxString key=GetCurrentKeyword(p);
		if (key.Len()!=0 && (s==wxSTC_C_IDENTIFIER||s==wxSTC_C_GLOBALCLASS)) {
			if (GetCharAt(p+key.Len()=='('||GetCharAt(p+key.Len()=='['))) { // si es arreglo, incluir los indices
				int p2 = BraceMatch(p+key.Len());
				if (p2!=wxSTC_INVALID_POSITION) key+=GetTextRange(p+key.Len(),++p2);
			}
			_DEBUG_LAMBDA_3( lmbCalltip, mxSource,src, wxString,var, int,pos, { src->CallTipShow(pos,var+": "+ans.Mid(2)); } );
			debug->SendEvaluation(key,new lmbCalltip(this,key,p));
		}
	} 
	else if (config->rt_syntax && main_window->IsActive()) {
		int p = event.GetPosition();
		int indics = _if_wx3_else(IndicatorAllOnFor(p),GetStyleAt(p));
		if (indics&(indic_to_mask[INDIC_ERROR_1]|indic_to_mask[INDIC_ERROR_2])) {
			unsigned int l=LineFromPosition(p);
			if (rt_errors.size()>l && rt_errors[l].is) ShowRealTimeError(p,rt_errors[l].s);
		}
	}
}

void mxSource::HighLight(wxString words, int from, int to) {
	if (to!=-1) words<<" 0"<<from<<" 1"<<to; // el 0 y 1 son porque scintilla los va a ordernar alfabeticamente, para que queden simpre primero y segundo
	SetKeyWords(3,words.Lower());
	Colourise(0,GetLength());
}

void mxSource::ClearBlocks ( ) {
	for(unsigned int i=0;i<blocks.GetCount();i++) { 
		blocks[i]=-1;
	}
	for(unsigned int i=0;i<blocks_reverse.GetCount();i++) { 
		blocks_reverse[i]=-1;
	}
}

void mxSource::AddBlock (int l1, int l2) {
	if (l1<0||l2<0) return;
	while (int(blocks.GetCount())<=l1) blocks.Add(-1);
	blocks[l1]=l2;
	while (int(blocks_reverse.GetCount())<=l2) blocks_reverse.Add(-1);
	blocks_reverse[l2]=l1;
}

void mxSource::SetStatus (int cual) {
	
	if (cual!=-1) {
		status_should_change=false;
		status_bar->SetStatus(status=cual);
		return;
	}
	
//	// no pasa nada, edicion normal...
//	if (!status_should_change) { // si se habia definido un estado externo (por main window, o por debug panel por ejemplo), se mantiene hasta que alguien modifique el pseudocodigo
//		status_bar->SetStatus(status);
//		return;
//	}
	if (config->rt_syntax) { // ...con verificacion de sintaxis en tiempo real
		if (!rt_errors.empty()) status_bar->SetStatus(status=STATUS_SYNTAX_ERROR);
		else if (run_socket) status_bar->SetStatus(status=STATUS_RUNNING_CHANGED);
		else status_bar->SetStatus(status=STATUS_SYNTAX_OK);
	} else // ...sin verificacion de sintaxis en tiempo real
		status_bar->SetStatus(status=STATUS_NO_RTSYNTAX);
	
}

void mxSource::OnChange (wxStyledTextEvent & event) {
	just_created=false; status_should_change=true; event.Skip();
	if (run_socket && status!=STATUS_RUNNING_CHANGED && status!=STATUS_SYNTAX_ERROR) {
		run_socket->Write("dimm\n",5);
		SetStatus(STATUS_RUNNING_CHANGED);
	}
	if (!mask_timers) {
		DoRTSyntaxChecking();
		reload_timer->Start(RELOAD_DELAY,true);
	}
}
#ifdef _AUTOINDENT
void mxSource::OnModified (wxStyledTextEvent & event) {
	event.Skip();
	if (indent_timer) {
		if (event.GetModificationType()&(wxSTC_MOD_INSERTTEXT|wxSTC_MOD_DELETETEXT)) {
			int el1 = LineFromPosition(event.GetPosition());
			int el2 = el1+event.GetLinesAdded()+2;
			for (int el = el1; el<el2; ++el) {
				if (to_indent.size()<=el+1) to_indent.resize(el+1,false);
				to_indent[el] = to_indent[el+1] = true;
				if (el<to_indent_first_line) to_indent_first_line = el;
				indent_timer->Start(200,true);
			}
		}
	}
}
#endif

int mxSource::GetId ( ) {
	return id;
}

wxString mxSource::GetTempFilenamePSC() {
	return temp_filename_prefix+".psc";
}

wxString mxSource::GetTempFilenamePSD() {
	return temp_filename_prefix+".psd";
}

wxString mxSource::GetTempFilenameOUT() {
	return temp_filename_prefix+".out";
}


wxString mxSource::SaveTemp() {
	mask_timers=true;
	wxString fname=GetTempFilenamePSC();
	bool mod = GetModify();
	SaveFile(fname);
	SetModify(mod);
	mask_timers=false;
	return fname;
}

/**
* @param ignore_rt  Cuando el usuario incia la ejecución desde el diagrama de flujo los datos errores en tiempo real estan desactualizados (el código acaba de recibirse desde psdraw3).
**/
bool mxSource::UpdateRunningTerminal (bool raise, bool ignore_rt) {
	if (!run_socket) return false;
	if (!ignore_rt && rt_running && !rt_timer->IsRunning() && !rt_errors.empty()) return false; // el rt_timer ya dijo que estaba mal, no vale la pena intentar ejecutar
	reload_timer->Stop();
	SaveTemp();
	run_socket->Write("reload\n",7);
	if (raise) run_socket->Write("raise\n",6);
	SetStatus(STATUS_RUNNING_UPDATED);
	return true;
}

void mxSource::StopReloadTimer ( ) {
	reload_timer->Stop();
}

static bool EstiloNada(int s) {
	return s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_STRING || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRINGEOL;
}

wxString mxSource::GetInstruction (int p) {
	int i = PositionFromLine(LineFromPosition(p));
	wxString instruccion; int i0=-1; bool first=true;
	bool flag_nros=false; // para que en la primera no permita nros
	while (i<p) {
		int s=GetStyleAt(i);
		char c=GetCharAt(i);
		bool nada=EstiloNada(s);
		if ( nada || (!EsLetra(c,flag_nros)) ) {
			flag_nros=true;
			if (!nada && c==';') {
				i0=-1; first=true; instruccion.Clear();
			} else if (i0!=-1) {
				wxString palabra=GetTextRange(i0,i);
				if (first) {
					instruccion=palabra;
					first=false;
				} else {
					palabra.MakeLower();
					if (palabra=="entonces" || palabra=="hacer") {
						first=true; instruccion.Clear();
					}
				}
				i0=-1;
			}
		} else {
			if (i0==-1) { i0=i; }
		}
		i++;
	}
	return instruccion.Lower();
}

void mxSource::KillRunningTerminal ( ) {
	if (run_socket) {
		run_socket->Write("quit\n",5);
		run_socket=NULL;
	}
}

int mxSource::GetStatus ( ) {
	return status;
}

wxString mxSource::GetPathForExport() {
	if (sin_titulo) return config->last_dir;
	else return wxFileName(filename).GetPath();
}

wxString mxSource::GetNameForExport() {
	if (sin_titulo) return "sin_titulo";
	else return wxFileName(filename).GetName();
}

void mxSource::OnCalltipClick (wxStyledTextEvent & event) {
	if (!current_calltip.is_error) return;
	int l=LineFromPosition(current_calltip.pos);
	if (l<0||l>int(rt_errors.size())) return;
	rt_err &e=rt_errors[l];
	if (e.is) main_window->QuickHelp().ShowRTError(e.n,e.s,true);
}

void mxSource::ProfileChanged ( ) {
	KillRunningTerminal();
	cfg_lang.Log();
	SetWords();
	SetAutocompletion();
	SetCalltips();
	if (is_example) {
		SetReadOnly(false);
		LoadFile(filename); 
		SetExample();
	}
	Colourise(0,GetLength());
//	SetStatus(STATUS_PROFILE);
}

void mxSource::RTOuputStarts ( ) {
	if (config->rt_syntax) ClearErrorData();
	ClearBlocks();
}

void mxSource::RTOuputEnds ( ) {
	if (config->rt_syntax) ClearErrorMarks();
	main_window->QuickHelp().ShowRTResult(!rt_errors.empty());
	SetStatus(); // para que diga en la barra de estado si hay o no errores
}

void mxSource::ClearErrorMarks ( ) {
	int sl=GetLineCount(), el=rt_errors.size();
	int n=sl>el?el:sl;
	for(int l=0;l<n;l++) { 
		if (!rt_errors[l].is && (MarkerGet(l)&(1<<MARKER_ERROR_LINE))) MarkerDelete(l,MARKER_ERROR_LINE);
	}
	for(int l=el;l<sl;l++) { 
		if ((MarkerGet(l)&(1<<MARKER_ERROR_LINE))) MarkerDelete(l,MARKER_ERROR_LINE);
	}
}

void mxSource::OnMarginClick (wxStyledTextEvent & event) {
	event.Skip();
	int l = LineFromPosition(event.GetPosition());
	if (l<rt_errors.size()) {
		rt_err &e = rt_errors[l];
		if (e.is) main_window->QuickHelp().ShowRTError(e.n,e.s,true);
	}
}

void mxSource::DebugMode (bool on) {
	if (on) {
		SetReadOnly(true);
		if (flow_socket) flow_socket->Write("debug start\n",12);
	} else {
		SetReadOnly(is_example||flow_socket);
		SetDebugLine();
		if (flow_socket) flow_socket->Write("debug stop\n",11);
	}
}

void mxSource::OnSetFocus (wxFocusEvent & evt) {
	_LOG("mxSource::SetFocus "<<this);
	flow_timer->Start(100,true);
	evt.Skip();
}

void mxSource::UpdateFromFlow ( ) {
	if (flow_socket) flow_socket->Write("send update\n",12);
}

void mxSource::DoRTSyntaxChecking ( ) {
	if (rt_running) rt_timer->Start(RT_DELAY,true);
}

void mxSource::OnMouseWheel (wxMouseEvent & event) {
	if (event.ControlDown()) {
		if (event.m_wheelRotation>0) {
			ZoomIn();
		} else {
			ZoomOut();
		}
	} else
		event.Skip();
}

void mxSource::OnPopupMenu(wxMouseEvent &evt) {
	
	// mover el cursor a la posición del click (a menos que haya una selección y se clickeó dentro)
	int p1=GetSelectionStart();
	int p2=GetSelectionEnd();
	int mp = PositionFromPointClose(evt.GetX(),evt.GetY());
	if (mp!=wxSTC_INVALID_POSITION && (p1==p2 || (mp<p1 && mp<p2) || (mp>p1 && mp>p2)) )
		GotoPos(mp);
	
	wxMenu menu("");
	int p=GetCurrentPos(); int s=GetStyleAt(p);
	wxString key=GetCurrentKeyword(p);
	
	if (key.Len()!=0 && (s==wxSTC_C_IDENTIFIER||s==wxSTC_C_GLOBALCLASS)) {
		if (IsProcOrSub(GetCurrentLine())) {
//			menu.Append(mxID_VARS_ADD_ALL_TO_DESKTOP_TEST,_ZZ("Agregar todas las variables a la prueba de escritorio"));
		} else {
			menu.Append(mxID_VARS_DEFINE,_ZZ("Definir variable \"")+key+_Z("\""));
			menu.Append(mxID_VARS_RENAME,_ZZ("Renombrar variable \"")+key+_Z("\""));
			menu.Append(mxID_VARS_ADD_ONE_TO_DESKTOP_TEST,_ZZ("Agregar variable \"")+key+_Z("\" a la prueba de escritorio"));
		}
	}
	if (key.Len()!=0 && help->GetQuickHelp(key,"").Len())
		menu.Append(mxID_HELP_QUICKHELP,_ZZ("Ayuda sobre \"")+key+"\"");
	if (STYLE_IS_COMMENT(s)) menu.Append(mxID_EDIT_UNCOMMENT,_Z("Descomentar"));
	else menu.Append(mxID_EDIT_COMMENT,_Z("Comentar"));
	menu.Append(mxID_EDIT_INDENT_SELECTION,_Z("Indentar"));
	menu.Append(mxID_EDIT_SELECT_ALL,_Z("Seleccionar todo"));
	
	menu.AppendSeparator();
	menu.Append(mxID_EDIT_UNDO,_Z("Deshacer"));
	menu.Append(mxID_EDIT_REDO,_Z("Rehacer"));
	menu.AppendSeparator();
	menu.Append(mxID_EDIT_CUT,_Z("Cortar"));
	menu.Append(mxID_EDIT_COPY,_Z("Copiar"));
	menu.Append(mxID_EDIT_PASTE,_Z("Pegar"));

	
	main_window->PopupMenu(&menu);
	
}

wxString mxSource::GetCurrentKeyword (int pos) {
	if (pos==-1) pos=GetCurrentPos();
	int s=WordStartPosition(pos,true);
	if (GetCharAt(s-1)=='#') s--;
	int e=WordEndPosition(pos,true);
	return GetTextRange(s,e);
}

bool mxSource::IsEmptyLine(int line/*, bool comments*/) {
	wxString s = GetLine(line);
	int l=s.Len(), i=0;
	while (i<l && (s[i]==' '||s[i]=='\t'||s[i]=='\n'||s[i]=='\r')) {
//		if (comments && s[i]=='/' && i+1<l && s[i+1]=='/') return true;
		i++;
	}
	return i==l;
}



bool mxSource::IsDimOrDef(int line) {
	wxString fword = GetFirstWord(GetLine(line));
	if (fword=="DEFINIR ") return true;
	if (fword=="DIMENSION ") return true;
	if (fword=="DIMENSIÓN ") return true;
	return false;
}

bool mxSource::IsProcOrSub(int line) {
	wxString fword = GetFirstWord(GetLine(line));
	if (fword=="PROCESO") return true;
	if (fword=="ALGORITMO") return true;
	if (cfg_lang[LS_ENABLE_USER_FUNCTIONS]) {
		if (fword=="SUBPROCESO") return true;
		if (fword=="FUNCION") return true;
		if (fword=="FUNCIÓN") return true;
		if (fword=="SUBALGORITMO") return true;
//		while (i<l) {
//			if (i+12<l && s.Mid(i,12).Upper()==" SUBPROCESO ") return true;
//			if (i+9<l && s.Mid(i,9).Upper()==" FUNCION ") return true;
//			if (i+9<l && s.Mid(i,9).Upper()==" FUNCIÓN ") return true;
//			if (i+9<l && s.Mid(i,14).Upper()==" SUBALGORITMO ") return true;
//			i++;
//		}
	}
	return false;
}

/**
* @brief Agrega una instrucción a un proceso para definir explícitamente una variable
*
* @param where    linea donde comienza el proceso (la de Proceso...., después de esta agregará la definición)
* @param var_name nombre de la variable a definir
* @param type     tipo de la variable, si es -1 lo consulta en el panel de variables
**/

void mxSource::DefineVar(int where, wxString var_name, int line_from, int type) {
	int n = GetLineCount(), empty_lines=0;
	
	// ver si se conoce la variable, y donde empieza el proceso
	if (type==-1||line_from==-1) {
		type = vars_window->GetVarType(where, var_name);
	} else where = line_from;
	if (type==-1) type = 0; 
	else if (type&LV_DEFINIDA) return;
	
	// ver si hay lineas en blanco al principio del proceso
	while (where+1<n && IsEmptyLine(where+1)) { where++; empty_lines++; } 
	
	bool add_line = empty_lines && where+1<n && !IsDimOrDef(where+1);
	
	if (var_name.Contains("[")) var_name=var_name.BeforeFirst('['); // cortar las dimensiones si fuera un arreglo
	
	wxString var_type;
	
	if (type==LV_LOGICA) var_type="Logica";
	else if (type==LV_NUMERICA) var_type="Numerica";
	else if (type==LV_CARACTER) var_type="Caracter";
	else { // si el tipo es ambiguo o desconocido, preguntar
		wxArrayString types, real_types;
		if (type==0 || (type&LV_LOGICA)) { types.Add("Logica"); real_types.Add("Logica"); }
		if (type==0 || (type&LV_NUMERICA)) { types.Add("Numérica (real)"); real_types.Add("Numerica"); }
		if (type==0 || (type&LV_NUMERICA)) { types.Add("Numérica Entera"); real_types.Add("Entera"); }
		if (type==0 || (type&LV_CARACTER)) { types.Add("Caracter/Cadena"); real_types.Add("Caracter"); }
		var_type=wxGetSingleChoice("Tipo de variable:",var_name,types);
		if (!var_type.Len()) return;
		else var_type = real_types[types.Index(var_type)];
	}
	// agregar la definicion
	int x=GetLineEndPosition(where);
	SetTargetStart(x); SetTargetEnd(x);
	ReplaceTarget(wxString("\n")<<"\tDefinir "<<var_name<<" Como "<<var_type<<(cfg_lang[LS_FORCE_SEMICOLON]?";":"")<<(add_line?"\n\t":""));
}

void mxSource::OnDefineVar (wxCommandEvent & evt) {
	if (config->show_vars) {
		DefineVar(GetCurrentLine(),GetCurrentKeyword());
	} else {
		RTSyntaxManager::Info info;
		info.SetForVarDef(GetCurrentLine(),GetCurrentKeyword());
		DoRealTimeSyntax(&info);
	}
}

void mxSource::OnRenameVar (wxCommandEvent & evt) {
	if (config->show_vars) {
		RenameVar(GetCurrentLine(),GetCurrentKeyword());
	} else {
		RTSyntaxManager::Info info;
		info.SetForVarRename(GetCurrentLine(),GetCurrentKeyword());
		DoRealTimeSyntax(&info);
	}
}


void mxSource::RenameVar(int where, wxString var_name, int line_from, int line_to) {
	
	if (var_name.Contains("[")) var_name=var_name.BeforeFirst('['); // cortar las dimensiones si fuera un arreglo
	if (var_name.IsEmpty()) return;
	
	if (line_from==-1||line_to==-1) {
		if (!vars_window->GetVarScope(where,var_name, line_from,line_to)) return;
	}
	
	wxString new_name = wxGetTextFromUser(_Z("Nuevo identificador:"),var_name,var_name,this);
	if (new_name.IsEmpty()) return;
	
	int p0 = PositionFromLine(line_from);
	int p = FindText(p0,GetLength(),var_name,wxSTC_FIND_WHOLEWORD);
	HighLight(new_name,line_from,line_to);
	while (p!=wxSTC_INVALID_POSITION && LineFromPosition(p)<=line_to) {
		if (GetStyleAt(p)==wxSTC_C_IDENTIFIER) {
			SetTargetStart(p);
			SetTargetEnd(p+var_name.Len());
			ReplaceTarget(new_name);
			p0 = p+new_name.Len();
		} else {
			p0 = p+var_name.Len();
		}
		p = FindText(p0,GetLength(),var_name,wxSTC_FIND_WHOLEWORD);
	}
}

void mxSource::AddOneToDesktopTest (wxCommandEvent & evt) {
	desktop_test->AddDesktopVar(GetCurrentKeyword());
}

//void mxSource::AddAllToDesktopTest (wxCommandEvent & evt) {
//	desktop_test->AddDesktopVar(GetCurrentKeyword());
//}

void mxSource::OnClick(wxMouseEvent &evt) {
//	if (evt.ControlDown()) {
//		int p=PositionFromPointClose(evt.GetX(),evt.GetY());
//		SetSelectionStart(p); SetSelectionEnd(p);
//		JumpToCurrentSymbolDefinition();
//	} else {
		wxPoint point=evt.GetPosition();
		int ss=GetSelectionStart(), se=GetSelectionEnd(), p=PositionFromPointClose(point.x,point.y);
		if ( p!=wxSTC_INVALID_POSITION && ss!=se && ( (p>=ss && p<se) || (p>=se && p<ss) ) ) {
//			MarkerDelete(current_line,mxSTC_MARK_CURRENT);
			wxTextDataObject my_data(GetSelectedText());
			wxDropSource dragSource(this);
			dragSource.SetData(my_data);
			mxDropTarget::current_drag_source=this;
			mxDropTarget::last_drag_cancel=false;
			wxDragResult result = dragSource.DoDragDrop(wxDrag_AllowMove|wxDrag_DefaultMove);
			if (mxDropTarget::current_drag_source!=NULL && result==wxDragMove) {
				mxDropTarget::current_drag_source=NULL;
				SetTargetStart(ss); SetTargetEnd(se); ReplaceTarget("");
			} 
			else if (result==wxDragCancel && ss==GetSelectionStart()) {
				DoDropText(evt.GetX(),evt.GetY(),""); // para evitar que se congele el cursor
				SetSelection(p,p);
			} else {
				DoDropText(evt.GetX(),evt.GetY(),"");
			}
		} else
			evt.Skip();
}

void mxSource::SetIndics (int from, int len, int indic, bool on) {
#ifdef WX3
	SetIndicatorCurrent(indic);
	if (on) IndicatorFillRange(from,len);
	else    IndicatorClearRange(from,len);
#else
	indic = indic_to_mask[indic];
	int lse = GetEndStyled();
	StartStyling(from,indic);
	wxStyledTextCtrl::SetStyling(len,on?indic:0);
	StartStyling(lse,0x1F);
#endif
}


#ifndef __APPLE__
bool mxSource::LoadFile (const wxString & fname) {
//#ifdef WX3
//	wxFFile file(fname,_T("r"));
//	wxString full_content;
//	if (file.ReadAll(&full_content,
//					 wxCSConv("ISO-8851")
////					 wxConvUTF8
//					 )) {
//		SetText(full_content);
//		EmptyUndoBuffer();
//		return true;
//	} else 
//#endif
		return wxStyledTextCtrl::LoadFile(fname);
}

bool mxSource::SaveFile (const wxString & fname) {
#ifdef WX3
	wxFFile file(fname,_T("w"));
	wxString full_content = GetText();
	if (file.Write(full_content,wxCSConv("ISO-8851"))) {
		SetModify(false);
		return true;
	} else 
#endif
		return wxStyledTextCtrl::SaveFile(fname);
}
#endif


void mxSource::OnKeyDown(wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_ESCAPE) {
		if (CallTipActive()) HideCalltip();
		else if (AutoCompActive()) AutoCompCancel();
		else main_window->QuickHelp().Hide();
	} else evt.Skip();
}

void mxSource::ShowUserList (wxArrayString &arr, int p1, int p2) {
	// reordenar la lista de palabras... el "desorden" viene de mezclar los menues de identificadores y de palabras clave
	arr.Sort();
	wxString res;
	for(unsigned int i=0;i<arr.GetCount();i++) { 
		if (i) res<<'|'; 
		res<<arr[i];
	}
	SetCurrentPos(p1);
	UserListShow(1,res);
	SetCurrentPos(p2);
}


/**
* Esta funcion esta para evitar el flickering que produce usar el bracehighlight
* del stc cuando se llama desde el evento de udateui. A cambio, para que igual sea
* instantaneo se llama desde el evento painted, y para evitar que reentre mil veces
* se guardan las ultimas posiciones y no se vuelve a llamar si son las mismas.
* El problema es que está recalculando el BraceMatch en cada paint.
**/
void mxSource::MyBraceHighLight (int b1, int b2) {
	if (b1==brace_1&&b2==brace_2) return;
	brace_1=b1; brace_2=b2;
	if (b2==wxSTC_INVALID_POSITION || LineFromPosition(b1)!=LineFromPosition(b2)) BraceBadLight (b1);
	else BraceHighlight (b1,b2);
	Refresh(false);
}

void mxSource::OnPainted (wxStyledTextEvent & event) {
	char c; int p=GetCurrentPos();
	if ((c=GetCharAt(p))=='(' || c==')' /*|| c=='{' || c=='}'*/ || c=='[' || c==']') {
		MyBraceHighLight(p,BraceMatch(p));
	} else if ((c=GetCharAt(p-1))=='(' || c==')' || /*c=='{' || c=='}' ||*/ c=='[' || c==']') {
		int m=BraceMatch(p-1);
		if (m!=wxSTC_INVALID_POSITION)
			MyBraceHighLight(p-1,m);
		else
			MyBraceHighLight();
	} else
		MyBraceHighLight();
	event.Skip();
}

void mxSource::SetJustCreated ( ) {
	SetModify(false);
	just_created = true;
}

void mxSource::FocusKilled ( ) {
	if (CallTipActive()) HideCalltip();
	if (AutoCompActive()) AutoCompCancel();
}

