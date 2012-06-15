#include "mxSource.h"
#include "ConfigManager.h"
#include "ids.h"
#include "mxProcess.h"
#include "mxDropTarget.h"
#include "DebugManager.h"
#include <wx/clipbrd.h>
#include <iostream>
#include <wx/process.h>
#include <wx/socket.h>
#include "mxMainWindow.h"
#include <vector>
#include "RTSyntaxManager.h"
using namespace std;

#define RT_DELAY 1000

const wxChar *mxSourceWords1 =
	_T("leer proceso definir como dimension si entonces sino segun hacer hasta que para con paso ")
	_T("repetir mientras de otro modo escribir finpara ")
	_T("fin finproceso finsi finmientras finsegun ")
	_T("verdadero falso numerico caracter logico logica entero entera texto cadena numeros enteros real reales enteras numericos numericas cadenas logicos logicas ")
	_T("borrar limpiar pantalla borrarpantalla limpiarpantalla esperar tecla esperartecla segundos milisegundos segundo milisegundo sinsaltar sin saltar sinbajar bajar ")
	_T("según finsegún "); // scintilla no funciona con los acentos

const wxChar *mxSourceWords1_op =
	_T(" y no o mod ");

const wxChar *mxSourceWords1_extra =
	_T("es sies opcion caso desde imprimir cada mostrar opción ");

const wxChar *mxSourceWords1_conds =
	_T("es par impar igual divisible multiplo distinto de por cero positivo negativo negativa positiva entero mayor menor");

const wxChar* mxSourceWords2 =
	_T("cos sen tan acos asen atan raiz rc ln abs exp azar trunc redon ");

//const wxChar* mxSourceWords3 = 
//	_T("hacer entonces para ");

BEGIN_EVENT_TABLE (mxSource, wxStyledTextCtrl)
	EVT_STC_UPDATEUI (wxID_ANY, mxSource::OnUpdateUI)
	EVT_STC_CHARADDED (wxID_ANY, mxSource::OnCharAdded)
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
	EVT_MENU (mxID_EDIT_BEAUTIFY_CODE, mxSource::OnEditBeautifyCode)
	EVT_MENU (mxID_EDIT_INDENT_SELECTION, mxSource::OnEditIndentSelection)
	EVT_STC_SAVEPOINTREACHED(wxID_ANY, mxSource::OnSavePointReached)
	EVT_STC_SAVEPOINTLEFT(wxID_ANY, mxSource::OnSavePointLeft)
	EVT_STC_CHANGE(wxID_ANY, mxSource::OnChange)
END_EVENT_TABLE()

const wxChar *mxSource::comp_list[MAX_COMP_SIZE];
const wxChar *mxSource::comp_text[MAX_COMP_SIZE];
int mxSource::comp_count=-1;

mxSource::mxSource (wxWindow *parent, wxString ptext, wxString afilename, bool ais_example) : wxStyledTextCtrl (parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxVSCROLL) {

	flow=NULL;
	input=NULL;
	socket=NULL;
	do_rt_syntax_checking=config->rt_syntax;
	
	page_text=ptext;
	
	last_s1=last_s2=0;
	is_example=ais_example;
	
	filename = afilename;
	sin_titulo = filename==wxEmptyString;
	SetStyling();
	SetTabWidth(config->tabw);
	SetUseTabs (true);
	
	wxFont font (config->font_size, wxMODERN, wxNORMAL, wxNORMAL);
	StyleSetFont (wxSTC_STYLE_DEFAULT, font);
	
	AutoCompSetSeparator('|');
	AutoCompSetIgnoreCase(true);
	SetBackSpaceUnIndents (true);
	SetTabIndents (true);
	SetIndent (4);
	SetIndentationGuides(true);
	if (comp_count==-1) {
		comp_count=0;
		comp_list[comp_count]=_T("Borrar Pantalla"); comp_text[comp_count++]=_T("Borrar Pantalla;");
		comp_list[comp_count]=_T("Caso "); comp_text[comp_count++]=_T("Caso ");
		comp_list[comp_count]=_T("Como Caracter"); comp_text[comp_count++]=_T("Como Caracter;");
		comp_list[comp_count]=_T("Como Entero"); comp_text[comp_count++]=_T("Como Entero;");
		comp_list[comp_count]=_T("Como Logico"); comp_text[comp_count++]=_T("Como Logico;");
		comp_list[comp_count]=_T("Como Real"); comp_text[comp_count++]=_T("Como Real;");
		comp_list[comp_count]=_T("Con Paso"); comp_text[comp_count++]=_T("Con Paso ");
		comp_list[comp_count]=_T("Definir"); comp_text[comp_count++]=_T("Definir ");
		comp_list[comp_count]=_T("Desde"); comp_text[comp_count++]=_T("Desde ");
		comp_list[comp_count]=_T("Dimension"); comp_text[comp_count++]=_T("Dimension ");
		comp_list[comp_count]=_T("Entonces"); comp_text[comp_count++]=_T("Entonces\n");
		comp_list[comp_count]=_T("Es Cero"); comp_text[comp_count++]=_T("Es Cero");
		comp_list[comp_count]=_T("Es Distinto De"); comp_text[comp_count++]=_T("Es Distinto De ");
		comp_list[comp_count]=_T("Es Divisible Por"); comp_text[comp_count++]=_T("Es Divisible Por ");
		comp_list[comp_count]=_T("Es Entero"); comp_text[comp_count++]=_T("Es Entero");
		comp_list[comp_count]=_T("Es Igual A"); comp_text[comp_count++]=_T("Es Igual A ");
		comp_list[comp_count]=_T("Es Impar"); comp_text[comp_count++]=_T("Es Impar");
		comp_list[comp_count]=_T("Es Mayor Que"); comp_text[comp_count++]=_T("Es Mayor Que ");
		comp_list[comp_count]=_T("Es Menor Que"); comp_text[comp_count++]=_T("Es Menor Que ");
		comp_list[comp_count]=_T("Es Multiplo De"); comp_text[comp_count++]=_T("Es Multiplo De ");
		comp_list[comp_count]=_T("Es Negativo"); comp_text[comp_count++]=_T("Es Negativo");
		comp_list[comp_count]=_T("Es Par"); comp_text[comp_count++]=_T("Es Par");
		comp_list[comp_count]=_T("Es Positivo"); comp_text[comp_count++]=_T("Es Positivo");
		comp_list[comp_count]=_T("Escribir"); comp_text[comp_count++]=_T("Escribir ");
		comp_list[comp_count]=_T("Esperar"); comp_text[comp_count++]=_T("Esperar ");
		comp_list[comp_count]=_T("Esperar Tecla"); comp_text[comp_count++]=_T("Esperar Tecla;");
		comp_list[comp_count]=_T("Hacer"); comp_text[comp_count++]=_T("Hacer\n");
		comp_list[comp_count]=_T("Hasta"); comp_text[comp_count++]=_T("Hasta ");
		comp_list[comp_count]=_T("Hasta Que"); comp_text[comp_count++]=_T("Hasta Que ");
		comp_list[comp_count]=_T("Imprimir"); comp_text[comp_count++]=_T("Imprimir ");
		comp_list[comp_count]=_T("Falso"); comp_text[comp_count++]=_T("Falso");
		comp_list[comp_count]=_T("Fin Mientras"); comp_text[comp_count++]=_T("Fin Mientras\n");
		comp_list[comp_count]=_T("Fin Para"); comp_text[comp_count++]=_T("Fin Para\n");
		comp_list[comp_count]=_T("Fin Proceso"); comp_text[comp_count++]=_T("Fin Proceso\n");
		comp_list[comp_count]=_T("Fin Segun"); comp_text[comp_count++]=_T("Fin Segun\n");
		comp_list[comp_count]=_T("Fin Si"); comp_text[comp_count++]=_T("Fin Si\n");
		comp_list[comp_count]=_T("FinMientras"); comp_text[comp_count++]=_T("FinMientras\n");
		comp_list[comp_count]=_T("FinPara"); comp_text[comp_count++]=_T("FinPara\n");
		comp_list[comp_count]=_T("FinProceso"); comp_text[comp_count++]=_T("FinProceso\n");
		comp_list[comp_count]=_T("FinSegun"); comp_text[comp_count++]=_T("FinSegun\n");
		comp_list[comp_count]=_T("FinSi"); comp_text[comp_count++]=_T("FinSi\n");
		comp_list[comp_count]=_T("Leer"); comp_text[comp_count++]=_T("Leer ");
		comp_list[comp_count]=_T("Limpiar Pantalla"); comp_text[comp_count++]=_T("Limpiar Pantalla;");
		comp_list[comp_count]=_T("Mientras"); comp_text[comp_count++]=_T("Mientras ");
		comp_list[comp_count]=_T("Mientras Que"); comp_text[comp_count++]=_T("Mientras Que ");
		comp_list[comp_count]=_T("Milisegundos"); comp_text[comp_count++]=_T("Milisegundos;");
		comp_list[comp_count]=_T("Opcion "); comp_text[comp_count++]=_T("Opcion ");
		comp_list[comp_count]=_T("Otro Modo:"); comp_text[comp_count++]=_T("Otro Modo:\n");
		comp_list[comp_count]=_T("Para"); comp_text[comp_count++]=_T("Para ");
		comp_list[comp_count]=_T("Proceso"); comp_text[comp_count++]=_T("Proceso ");
		comp_list[comp_count]=_T("Repetir"); comp_text[comp_count++]=_T("Repetir\n");
		comp_list[comp_count]=_T("Segun"); comp_text[comp_count++]=_T("Segun ");
		comp_list[comp_count]=_T("Segundos"); comp_text[comp_count++]=_T("Segundos;");
		comp_list[comp_count]=_T("Sin Saltar"); comp_text[comp_count++]=_T("Sin Saltar");
		comp_list[comp_count]=_T("Sino"); comp_text[comp_count++]=_T("Sino\n");
		comp_list[comp_count]=_T("Verdadero"); comp_text[comp_count++]=_T("Verdadero");
	}
	SetMarginType (0, wxSTC_MARGIN_NUMBER);
	SetMarginWidth (0, TextWidth (wxSTC_STYLE_LINENUMBER, _T(" XXX")));
	StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (_T("DARK GRAY")));
	StyleSetBackground (wxSTC_STYLE_LINENUMBER, *wxWHITE);

	IndicatorSetStyle(0,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetStyle(2,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetStyle(1,wxSTC_INDIC_BOX);
//	IndicatorSetStyle(2,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetForeground (0, 0x0000FF);
	IndicatorSetForeground (1, 0x005555);
	IndicatorSetForeground (2, 0x004499);
	
	MarkerDefine(0,wxSTC_MARK_SHORTARROW, _T("BLACK"), _T("GREEN"));
	MarkerDefine(1,wxSTC_MARK_BACKGROUND, wxColour(200,255,200), wxColour(200,255,200));
	MarkerDefine(2,wxSTC_MARK_SHORTARROW, _T("BLACK"), _T("YELLOW"));
	MarkerDefine(3,wxSTC_MARK_BACKGROUND, wxColour(255,255,200), wxColour(255,255,170));
	debug_line=-1;
	
	SetDropTarget(new mxDropTarget());
	
	if (is_example) SetReadOnly(true);
	
	rt_timer = new wxTimer(GetEventHandler());
	Connect(wxEVT_TIMER,wxTimerEventHandler(mxSource::OnRealTimeSyntaxTimer),NULL,this);
	
	SetMouseDwellTime(500);
	
}

mxSource::~mxSource() {
	debug->Close(this);
	if (socket) {
		socket->Write("quit",4);
		flow->SetSourceDeleted();
		socket=NULL; flow_id=-1;
	}
}

void mxSource::SetStyle(int idx, const wxChar *fontName, int fontSize, const wxChar *foreground, const wxChar *background, int fontStyle){
	wxFont font (fontSize, wxMODERN, wxNORMAL, wxNORMAL, false, fontName);
	StyleSetFont (idx, font);
	if (foreground) StyleSetForeground (idx, wxColour (foreground));
	if (background)  StyleSetBackground (idx, wxColour (background));
	StyleSetBold (idx, (fontStyle & mxSOURCE_BOLD) > 0);
	StyleSetItalic (idx, (fontStyle & mxSOURCE_ITALIC) > 0);
	StyleSetUnderline (idx, (fontStyle & mxSOURCE_UNDERL) > 0);
	StyleSetVisible (idx, (fontStyle & mxSOURCE_HIDDEN) == 0);

}

void mxSource::SetStyling(bool colour) {
	SetWords();
	SetStyle(wxSTC_C_DEFAULT,_T(""),config->font_size,_T("BLACK"),_T("WHITE"),0); // default
	SetStyle(wxSTC_C_COMMENT,_T(""),config->font_size,_T("BLACK"),_T("WHITE"),0); // comment
	SetStyle(wxSTC_C_COMMENTLINE,_T(""),config->font_size,_T("Z DARK GRAY"),_T("WHITE"),mxSOURCE_ITALIC); // comment line
	SetStyle(wxSTC_C_COMMENTDOC,_T(""),config->font_size,_T("BLUE"),_T("WHITE"),mxSOURCE_ITALIC); // comment doc
	SetStyle(wxSTC_C_NUMBER,_T(""),config->font_size,_T("SIENNA"),_T("WHITE"),0); // number
	SetStyle(wxSTC_C_WORD,_T(""),config->font_size,_T("DARK BLUE"),_T("WHITE"),mxSOURCE_BOLD); // keywords
	SetStyle(wxSTC_C_STRING,_T(""),config->font_size,_T("RED"),_T("WHITE"),0); // string
	SetStyle(wxSTC_C_CHARACTER,_T(""),config->font_size,_T("RED"),_T("WHITE"),0); // character
	SetStyle(wxSTC_C_UUID,_T(""),config->font_size,_T("ORCHID"),_T("WHITE"),0); // uuid
	SetStyle(wxSTC_C_PREPROCESSOR,_T(""),config->font_size,_T("FOREST GREEN"),_T("WHITE"),0); // preprocessor
	SetStyle(wxSTC_C_OPERATOR,_T(""),config->font_size,_T("BLACK"),_T("WHITE"),mxSOURCE_BOLD); // operator 
	SetStyle(wxSTC_C_IDENTIFIER,_T(""),config->font_size,_T("BLACK"),_T("WHITE"),0); // identifier 
	SetStyle(wxSTC_C_STRINGEOL,_T(""),config->font_size,_T("RED"),_T("Z LIGHT GRAY"),0); // string eol
	SetStyle(wxSTC_C_VERBATIM,_T(""),config->font_size,_T("BLACK"),_T("WHITE"),0); // default verbatim
	SetStyle(wxSTC_C_REGEX,_T(""),config->font_size,_T("ORCHID"),_T("WHITE"),0); // regexp  
	SetStyle(wxSTC_C_COMMENTLINEDOC,_T(""),config->font_size,_T("FOREST GREEN"),_T("WHITE"),0); // special comment 
	SetStyle(wxSTC_C_WORD2,_T(""),config->font_size,_T("DARK BLUE"),_T("WHITE"),0); // extra words
	SetStyle(wxSTC_C_COMMENTDOCKEYWORD,_T(""),config->font_size,_T("CORNFLOWER BLUE"),_T("WHITE"),0); // doxy keywords
	SetStyle(wxSTC_C_COMMENTDOCKEYWORDERROR,_T(""),config->font_size,_T("RED"),_T("WHITE"),0); // keywords errors
	SetStyle(wxSTC_C_GLOBALCLASS,_T(""),config->font_size,_T("BLACK"),_T("WHITE"),0); // keywords errors
	SetStyle(wxSTC_STYLE_BRACELIGHT,_T(""),config->font_size,_T("RED"),_T("Z LIGHT BLUE"),mxSOURCE_BOLD); 
	SetStyle(wxSTC_STYLE_BRACEBAD,_T(""),config->font_size,_T("DARK RED"),_T("WHITE"),mxSOURCE_BOLD); 
	SetLexer(wxSTC_LEX_CPPNOCASE);
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
		ReplaceTarget(_T("//"));
	}	
	EndUndoAction();
}

void mxSource::OnEditUnComment(wxCommandEvent &evt) {
	if (GetReadOnly()) return MessageReadOnly();
	int ss = GetSelectionStart();
	int min=LineFromPosition(ss);
	int max=LineFromPosition(GetSelectionEnd());
	int aux;
	if (max>min && PositionFromLine(max)==GetSelectionEnd()) max--;
	BeginUndoAction();
	for (int i=min;i<=max;i++) {
		if (GetLine(i).Left(2)==_T("//")) {
			SetTargetStart(PositionFromLine(i));
			SetTargetEnd(PositionFromLine(i)+2);
			ReplaceTarget(_T(""));
		} else if (GetLine(i).Left((aux=GetLineIndentPosition(i))-PositionFromLine(i)+2).Right(2)==_T("//")) {
			SetTargetStart(aux);
			SetTargetEnd(aux+2);
			ReplaceTarget(_T(""));
		}
	}
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
		int p2=comp_to=GetCurrentPos();
		int s=GetStyleAt(p2-1);
		if (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_STRING || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRINGEOL) return;
		int p1=comp_from=WordStartPosition(p2-1,true);
		if (p2-p1==4 && GetTextRange(p1,p2-1).Upper()==_T("FIN")) {
			wxString res;
			for (int i=0;i<comp_count;i++) {
				if (comp_list[i][0]=='F'&&comp_list[i][1]=='i'&&comp_list[i][2]=='n'&&comp_list[i][3]==' ') {
					if (res.Len())
						res<<_T("|")<<comp_list[i];
					else
						res=comp_list[i];
				}
			}
			SetCurrentPos(p1);
			UserListShow(1,res);
			SetCurrentPos(p2);
		} else
		if (config->lang.coloquial_conditions) {
			int s=GetStyleAt(p2-1);
			if (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_STRING || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRINGEOL) return;
			int p1=comp_from=WordStartPosition(p2-1,true);
			if (p2-p1==3 && GetTextRange(p1,p2-1).Upper()==_T("ES")) {
				wxString res;
				for (int i=0;i<comp_count;i++) {
					if (comp_list[i][0]=='E'&&comp_list[i][1]=='s'&&comp_list[i][2]==' ') {
						if (res.Len())
							res<<_T("|")<<comp_list[i];
						else
							res=comp_list[i];
					}
				}
				SetCurrentPos(p1);
				UserListShow(1,res);
				SetCurrentPos(p2);
			}
		}
	} else if ((chr|32)>='a' && (chr|32)<='z' && config->autocomp) {
		int p2=comp_to=GetCurrentPos();
		int s=GetStyleAt(p2);
		if (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_STRING || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRINGEOL) return;
		int p1=comp_from=WordStartPosition(p2,true);
		if (p2-p1>2)  {
			wxString str = GetTextRange(p1,p2);
			str.MakeLower();
			int l = str.Len();
			bool show=false;
			wxString res;
//			int li = LineFromPosition(p1);
			for (int j,i=0;i<comp_count;i++) {
				for (j=0;j<l;j++)
					if (str[j]!=(comp_list[i][j]|32))
						break;
				if (j==l && (comp_list[i][3]!=' '||comp_list[i][0]!='F')) {
					if (!show) {
						show=true;
						res=comp_list[i];
					} else
						res<<_T("|")<<comp_list[i];
				}
			}
			if (show) {
				SetCurrentPos(p1);
				UserListShow(1,res);
				SetCurrentPos(p2);
			}
		}
	}
	if (config->calltip_helps && (chr==' ' || chr=='\n' || chr=='\t' || chr=='\r')) {
		int p = GetCurrentPos()-1;
		while (p>0 && (GetCharAt(p)==' ' || GetCharAt(p)=='\t' || GetCharAt(p)=='\r' || GetCharAt(p)=='\n'))
			p--;
		int s = GetStyleAt(p);
		if (s!=wxSTC_C_CHARACTER && s!=wxSTC_C_STRING && s!=wxSTC_C_STRINGEOL && s!=wxSTC_C_COMMENTLINE) {
			int p2=p+1;
			while (p>0 && !(GetCharAt(p)==' ' || GetCharAt(p)=='\t' || GetCharAt(p)=='\r' || GetCharAt(p)=='\n'))
				p--;
			wxString text = GetTextRange(p+1,p2).MakeLower();
			if (GetTextRange(p-3,p+1).Upper()==_T("FIN ")) return;
			if (text==_T("leer")||text==_T("definir"))
				ShowCalltip(GetCurrentPos(),_T("{una o mas variables, separadas por comas}"));
			else if (text==_T("esperar"))
				ShowCalltip(GetCurrentPos(),_T("{\"Tecla\" o intervalo de tiempo}"));
			else if (text==_T("escribir")||text==_T("mostrar")||text==_T("imprimir"))
				ShowCalltip(GetCurrentPos(),_T("{una o mas expresiones, separadas por comas}"));
			else if (text==_T("mientras"))
				ShowCalltip(GetCurrentPos(),_T("{condicion, expresion logica}"));
			else if (text==_T("que"))
				ShowCalltip(GetCurrentPos(),_T("{condicion, expresion logica}"));
			else if (text==_T("para"))
				ShowCalltip(GetCurrentPos(),_T("{asignacion inicial: variable<-valor}"));
			else if (text==_T("desde"))
				ShowCalltip(GetCurrentPos(),_T("{valor inicial}"));
			else if (text==_T("hasta")) {
				int l=LineFromPosition(p+1);
				while (p>0 && (GetCharAt(p)==' ' || GetCharAt(p)=='\t' || GetCharAt(p)=='\r' || GetCharAt(p)=='\n'))
					p--;
				if (LineFromPosition(p+1)==l)
					ShowCalltip(GetCurrentPos(),_T("{valor final}"));
			} else if (text==_T("paso"))
				ShowCalltip(GetCurrentPos(),_T("{valor del paso}"));
			else if (text==_T("si"))
				ShowCalltip(GetCurrentPos(),_T("{condicion, expresion logica}"));
			else if (text==_T("entonces"))
				ShowCalltip(GetCurrentPos(),_T("{acciones por verdadero}"));
			else if (text==_T("sino"))
				ShowCalltip(GetCurrentPos(),_T("{acciones por falso}"));
			else if (text==_T("segun"))
				ShowCalltip(GetCurrentPos(),_T("{variable o expresion numerica}"));
			else if (text==_T("opcion")||text==_T("sies")||text==_T("caso"))
				ShowCalltip(GetCurrentPos(),_T("{variable o expresion numerica}"));
			else
				HideCalltip();
		}
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
		ReplaceTarget(_T(" "));
		SetTargetStart(p); 
		SetTargetEnd(p+1);
		ReplaceTarget(_T(""));
		EndUndoAction();
		if (ro) SetReadOnly(true);
	} else 
		SetSavePoint();
}

void mxSource::OnUserListSelection(wxStyledTextEvent &evt) {
	SetTargetStart(comp_from);
	SetTargetEnd(comp_to);
	if (config->smart_indent) {
		int i=0;
		wxString what = evt.GetText();
		while (comp_list[i]!=what) i++;
		wxString text(comp_text[i]);
		if (!config->lang.force_dot_and_comma && text.Last()==';') text.RemoveLast();
		if (comp_from>5&&text.Last()==' '&&GetTextRange(comp_from-4,comp_from).Upper()==_T("FIN "))
			text.Last()='\n';
		ReplaceTarget(text);
		SetSelection(comp_from+text.Len(),comp_from+text.Len());
		int lfp=LineFromPosition(comp_from);
		if (text.Mid(0,3)==_T("Fin") || text==_T("Hasta Que ") || text==_T("Mientras Que ") || text.Mid(0,4)==_T("Sino")||text.Last()=='\n')
			IndentLine(lfp);
		if (text.Last()=='\n') {
			IndentLine(lfp+1);
			int lip=GetLineIndentPosition(lfp+1);
			SetSelection(lip,lip);
			if (config->autoclose) TryToAutoCloseSomething(lfp+1);
		}
	} else {
		wxString text = evt.GetText();
		while (text.Last()=='\n') text.RemoveLast();
		ReplaceTarget(text);
		SetSelection(comp_from+text.Len(),comp_from+text.Len());
	}
	wxStyledTextEvent evt2;
	evt2.SetKey(' ');
	OnCharAdded(evt2);
}

void mxSource::SetFieldIndicator(int p1, int p2) {
	int lse = GetEndStyled();
	StartStyling(p1,wxSTC_INDICS_MASK);
	wxStyledTextCtrl::SetStyling(p2-p1,wxSTC_INDIC1_MASK);
	GotoPos(p1);
	SetSelection(p1,p2);
	StartStyling(lse,0x1F);
}

void mxSource::OnUpdateUI (wxStyledTextEvent &event) {
	int p = GetCurrentPos();
	int s = GetStyleAt(p);
	if (s&wxSTC_INDIC1_MASK) {
		int p2=p;
		while (GetStyleAt(p2)&wxSTC_INDIC1_MASK)
			p2++;
		while (GetStyleAt(p)&wxSTC_INDIC1_MASK)
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
	} else if (s&(wxSTC_INDIC0_MASK|wxSTC_INDIC2_MASK)) {
		unsigned int l=GetCurrentLine();
		if (rt_errors.GetCount()>l && rt_errors[l].Len()) ShowRealTimeError(p,rt_errors[l].Mid(0,rt_errors[l].Len()-1));
	} else {
		HideCalltip(true,false);
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
			AppendText(_T("\n"));
		SetTargetStart(PositionFromLine(max+1));
		SetTargetEnd(PositionFromLine(max+1));
		ReplaceTarget(line);
		SetTargetStart(PositionFromLine(min-1));
		SetTargetEnd(PositionFromLine(min));
		ReplaceTarget(_T(""));
		EndUndoAction();
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
		ReplaceTarget(_T(""));
		SetTargetStart(PositionFromLine(min));
		SetTargetEnd(PositionFromLine(min));
		ReplaceTarget(line);
		if (ss==-1) SetSelectionStart(PositionFromLine(min+1));
		if (se==-1) SetSelectionStart(PositionFromLine(min+1));
		EndUndoAction();
	}	
}

void mxSource::OnModifyOnRO (wxStyledTextEvent &event) {
	MessageReadOnly();
}

void mxSource::MessageReadOnly() {
	if (flow) wxMessageBox(_T("Cierre la ventana del editor de diagramas de flujo para este algortimo, antes de continuar editando el pseudocódigo."));
	else if (!is_example) wxMessageBox(_T("No se puede modificar el pseudocodigo mientras esta ejecutandose paso a paso."));
	else wxMessageBox(_T("No se permite modificar los ejemplos, pero puede copiarlo y pegarlo en un nuevo archivo."));
}

void mxSource::SetExample() {
	wxString total;
	for (int i=2;i<GetLineCount();i++) {
		wxString str=GetLine(i),aux;
//		cerr<<str<<endl;
		int p0=str.Index('{'),p1,p2;
		while (p0!=wxNOT_FOUND) {
			aux=str.Mid(p0);
			p1=aux.Index('#');
			p2=aux.Index('}');
			if (p2==wxNOT_FOUND) {
				wxMessageBox(_T("Ha ocurrido un error al procesar el ejemplo. Puede que el pseudocodigo no sea correcto."));
				break;
			}
			if (p1==wxNOT_FOUND||p1>p2) {
				
				if (p2==2&&aux[1]==';') {
					if (config->lang.force_dot_and_comma)
						str=str.Mid(0,p0)+";"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+aux.Mid(p2+1);
				
				} else if (p2==2&&aux[1]=='&') {
					if (config->lang.word_operators)
						str=str.Mid(0,p0)+"Y"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+"&"+aux.Mid(p2+1);
				
				} else if (p2==2&&aux[1]=='|') {
					if (config->lang.word_operators)
						str=str.Mid(0,p0)+"O"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+"|"+aux.Mid(p2+1);
				
				} else if (p2==2&&aux[1]=='~') {
					if (config->lang.word_operators)
						str=str.Mid(0,p0)+"NO"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+"~"+aux.Mid(p2+1);
				
				} else if (p2==2&&aux[1]=='%') {
					if (config->lang.word_operators)
						str=str.Mid(0,p0)+"MOD"+aux.Mid(p2+1);
					else
						str=str.Mid(0,p0)+"%"+aux.Mid(p2+1);
				
				} else if (p2>8 && ( aux.Mid(1,8)=="DEFINIR "|| aux.Mid(1,8)=="Definir "|| aux.Mid(1,8)=="definir " ) ) {
					if (config->lang.force_define_vars)
						str=str.Mid(0,p0)+str.Mid(p0+1,p2-1)+aux.Mid(p2+1);
					else
						str="";
				
				} else {
					cerr<<"ERROR PARSING EXAMPLE!!!"<<endl;
					str=str.Mid(0,p0)+str.Mid(p0+1,p2-1)+aux.Mid(p2+1);
				}
				
			} else {
				
				p1+=p0; p2+=p0;
				if (config->lang.base_zero_arrays)
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

void mxSource::IndentLine(int l, bool goup) {
	int btype,ignore_next=false;
	int cur=GetIndentLevel(l,goup,&btype);
	wxString line=GetLine(l);
	if (line.StartsWith("//")) return;
	line<<_T(" "); int i=0,n=line.Len();
	while (i<n&&(line[i]==' '||line[i]=='\t')) i++;
	int ws=i;
	if (i<n) {
		while (i<n && (line[i]=='_'||((line[i]|32)>='a'&&(line[i]|32)<='z'))) i++;
		if (ignore_next)
			ignore_next=false;
			else {
			wxString word=line.SubString(ws,i-1);
			word.MakeUpper();
			if (word==_T("SINO")) cur-=4;
			else if (word==_T("DE") && i+10<n && line.SubString(ws,i+10).Upper()==_T("DE OTRO MODO:")) cur-=4;
			else if (word==_T("HASTA") && i+4<n && line.SubString(ws,i+4).Upper()==_T("HASTA QUE ")) cur-=4;
			else if (word==_T("MIENTRAS") && i+4<n && line.SubString(ws,i+4).Upper()==_T("MIENTRAS QUE ")) cur-=4;
			else if (word==_T("FINSEGUN")) cur-=8;
			else if (word==_T("FINMIENTRAS")) cur-=4;
			else if (word==_T("FINPARA")) cur-=4;
			else if (word==_T("FIN")) { cur-=4; ignore_next=true; }
			else if (word==_T("FINSI")) cur-=4;
			else if (word==_T("FINPROCESO")) cur-=4;
			else {
				bool comillas=false;
				while (i<n) {
					if (i+1<n && line[i]=='/' && line[i+1]=='/') break;
					if (line[i]=='\''||line[i]=='\"')
						comillas=!comillas;
					else if (!comillas) {
						if (line[i]==';') break;
						else if (line[i]==':' && line[i+1]!=':') {cur-=4; break;}
					}
					i++;
				}
			}
	//		else if (word=="FIN") cur-=4;
		}
	}
	if (btype==BT_SEGUN && GetLineEndPosition(l)==GetLineIndentPosition(l)) cur-=4;
	if (cur<0) cur=0;
//	int cp=GetCurrentPos();
	if (GetCurrentPos()==GetLineIndentPosition(l)) {
		SetLineIndentation(l,cur);
		SetSelection(GetLineIndentPosition(l),GetLineIndentPosition(l));
	} else
		SetLineIndentation(l,cur);
	
}

void mxSource::OnEditBeautifyCode(wxCommandEvent &evt) {
	for (int i=0;i<GetLineCount();i++)
		IndentLine(i);
}

int mxSource::GetIndentLevel(int l, bool goup, int *e_btype) {
	int btype=BT_NONE;
	if (goup) while (l>=1 && !LineHasSomething(l-1)) l--;
	if (l<1) return 0;
	wxString line=GetLine(l-1);
	line<<_T(" ");
	int cur=GetLineIndentation(l-1);
	int n=line.Len();
	bool comillas=false;
	bool first_word=true; // para saber si es la primer palabra de la instruccion
	bool ignore_next=false; // para que despues de Fin se saltee lo que sigue
	int wstart=0; // para guardar donde empezaba la palabra
	char c;
	for (int i=0;i<n;i++) {
		c=line[i];
		if (c=='\'' || c=='\"') {
			comillas=!comillas;
		} else if (!comillas) {
			if (c=='/' && i+1<n && line[i+1]=='/')  return cur;
			if (c==':' && line[i+1]!='=') { cur+=4; btype=BT_CASO; }
			else if (c!='_'&&((c|32)<'a'||(c|32)>'z')) {
				if (wstart+1<i) {
					if (ignore_next)
						ignore_next=false;
					else {
//						int old_btype=btype;
//						if (btype) *btype=BT_NONE;
						wxString word=line.SubString(wstart,i-1);
						word.MakeUpper();
						if (word==_T("SI")) { cur+=4; btype=BT_SI; }
						else if (word==_T("SINO")) { cur+=4; btype=BT_SINO; }
						else if (word==_T("PROCESO")) { cur+=4; btype=BT_PROCESO; }
						else if (word==_T("MIENTRAS") && !(i+4<n && line.SubString(wstart,i+4).Upper()==_T("MIENTRAS QUE "))) cur+=4;
						else if (word==_T("SEGUN")) { cur+=8; btype=BT_SEGUN; }
						else if (word==_T("PARA")) { cur+=4; btype=BT_PARA;	}
						else if (word==_T("REPETIR")||(first_word && word==_T("HACER"))) { cur+=4; btype=BT_REPETIR; }
						else if (word==_T("FIN")) { ignore_next=true; btype=BT_NONE; }
						else if (btype!=BT_NONE && (word=="FINSEGUN"||word=="FINSEGÚN"||word=="FINPARA"||word=="FINMIENTRAS"||word=="FINSI"||word=="MIENTRAS"||word=="FINPROCESO")) {
							if (btype==BT_SEGUN) cur-=4;
							btype=BT_NONE; cur-=4;
						}
//						if (first_word && btype) *btype=old_btype;
						first_word=false;
					}
				}
				wstart=i+1;
				if (c==';') first_word=true;
			}
		}
	}
	if (e_btype) *e_btype=btype;
	return cur;
}

void mxSource::Indent(int l1, int l2) {
	BeginUndoAction();
	bool goup=true;
	for (int i=l1;i<=l2;i++) {
		IndentLine(i,goup);
		if (goup && LineHasSomething(i)) goup=false;
	}
	EndUndoAction();
}

void mxSource::UnExample() {
	SetReadOnly(is_example=false);
}

void mxSource::SetWords() {
	wxString s1=mxSourceWords1;
	if (config->lang.word_operators) s1<<mxSourceWords1_op;
	if (config->lang.lazy_syntax) s1<<mxSourceWords1_extra;
	if (config->lang.coloquial_conditions) s1<<mxSourceWords1_conds;
	SetKeyWords (0, s1.c_str());
	SetKeyWords (1, mxSourceWords2);
//	SetKeyWords (2, mxSourceWords3);
}

void mxSource::EditFlow ( mxProcess *proc, int id ) {
	flow=proc; flow_id=id;
	SetReadOnly(proc!=NULL||is_example);
}

int mxSource::GetFlowId() { 
	return flow_id; 
}

void mxSource::ReloadTemp (wxString file) {
	bool isro=GetReadOnly();
	if (isro) SetReadOnly(false);
	LoadFile(file);
	SetModify(true);
	if (isro) SetReadOnly(true);
}

wxSocketBase * mxSource::GetFlowSocket ( ) {
	return socket;
}
void mxSource::SetFlowSocket ( wxSocketBase *s ) {
	socket=s;
}

void mxSource::SetDebugPause() {
	if (debug_line!=-1) {
		MarkerDeleteHandle(debug_line_handler_1);
		MarkerDeleteHandle(debug_line_handler_2);
		debug_line_handler_1=MarkerAdd(debug_line,2);
		debug_line_handler_2=MarkerAdd(debug_line,3);
	}
}

void mxSource::SetDebugLine(int l, int i) {
	if (debug_line!=-1) {
		MarkerDeleteHandle(debug_line_handler_1);
		MarkerDeleteHandle(debug_line_handler_2);
	}
	if ((debug_line=l)!=-1) {
		debug_line_handler_1=MarkerAdd(l,0);
		debug_line_handler_2=MarkerAdd(l,1);
		EnsureVisibleEnforcePolicy(l);
		if (i!=-1) SelectInstruccion(l,i);
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
	int s; char c;
	for (int i=i1;i<=i2;i++) {
		c=GetCharAt(i); s=GetStyleAt(i);
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
	main_window->notebook->SetPageText(main_window->notebook->GetPageIndex(this),page_text+_T("*"));	
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
				if (s[i]==';'||s[i]=='\n') { v.push_back(last_ns); starting=true; }
				else if ((s[i]|32)=='e' && i+8<len && s.Mid(i,8).Upper()=="ENTONCES" && ((s[i+8]|32)<'a'||(s[i+8]|32)>'z') && (s[i+8]<'0'||s[i+8]>'9') && s[i+8]!='_') {
					if (v.back()!=i) { v.push_back(last_ns); v.push_back(i); } v.push_back(i+8); 
					i+=7; starting=true;
				}
				else if ((s[i]|32)=='h' && i+5<len && s.Mid(i,5).Upper()=="HACER" && ((s[i+5]|32)<'a'||(s[i+5]|32)>'z') && (s[i+5]<'0'||s[i+5]>'9') && s[i+5]!='_') {
					if (v.back()!=i) { v.push_back(last_ns); v.push_back(i); } v.push_back(i+5); 
					i+=4; starting=true;
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
	else SetSelection(_l+v[2*_i],_l+v[2*_i+1]);
}

void mxSource::DoRealTimeSyntax ( ) {
	RTSyntaxManager::Process(this);
}

void mxSource::ClearErrors() {
	rt_errors.Clear();
	int lse = GetEndStyled();
	StartStyling(0,wxSTC_INDIC0_MASK|wxSTC_INDIC2_MASK);
	wxStyledTextCtrl::SetStyling(GetLength(),0);
	StartStyling(lse,0x1F);
}

void mxSource::MarkError(int l, int i, wxString str, bool special) {
	if (l<0) return;
	if (l>=GetLineCount()) return;
	if (l>=int(rt_errors.GetCount())) 
		rt_errors.Insert(wxEmptyString,rt_errors.GetCount(),GetLineCount()-rt_errors.GetCount());
	rt_errors[l]+=wxString("(")<<i+1<<") "<<str+"\n";
	int lse = GetEndStyled();
	vector<int> &v=FillAuxInstr(l);
	if (int(v.size())<=2*i+1) return;
	l=PositionFromLine(l);
	StartStyling(l+v[2*i],special?wxSTC_INDIC2_MASK:wxSTC_INDIC0_MASK);
	wxStyledTextCtrl::SetStyling(v[2*i+1]-v[2*i],special?wxSTC_INDIC2_MASK:wxSTC_INDIC0_MASK);
	StartStyling(lse,0x1F);
}

void mxSource::StartRTSyntaxChecking ( ) {
	do_rt_syntax_checking=true;
	rt_timer->Start(RT_DELAY,true);
}

void mxSource::StopRTSyntaxChecking ( ) {
	do_rt_syntax_checking=false;
	rt_timer->Stop(); ClearErrors();
}

void mxSource::OnRealTimeSyntaxTimer (wxTimerEvent & te) {
	DoRealTimeSyntax();
}

void mxSource::OnChange(wxStyledTextEvent &event) {
	if (do_rt_syntax_checking) rt_timer->Start(RT_DELAY,true);
	event.Skip();
}

void mxSource::ShowCalltip (int pos, const wxString & l, bool is_error, bool is_dwell) {
	current_calltip_is_error=is_error;
	current_calltip_is_dwell=is_dwell;
	CallTipShow(pos,l);
}

void mxSource::ShowRealTimeError (int pos, const wxString & l, bool is_dwell) {
	ShowCalltip(pos,l,true,is_dwell);
}

void mxSource::HideCalltip (bool if_is_error, bool if_is_not_error) {
	if (current_calltip_is_error && if_is_error) CallTipCancel();
	else if (!current_calltip_is_error && if_is_not_error) CallTipCancel();
}

void mxSource::TryToAutoCloseSomething (int l) {
	// ver si se abre una estructura
	int btype;
	GetIndentLevel(l,false,&btype); 
	// buscar la siguiente linea no nula
	int l2=l+1,ln=GetLineCount();
	if (btype==BT_NONE||btype==BT_SINO||btype==BT_PROCESO||btype==BT_CASO) return;
	while (l2<ln && GetLineEndPosition(l2)==GetLineIndentPosition(l2)) l2++;
	// comparar los indentados para ver si la siguiente esta dentro o fuera
	int i1=GetLineIndentPosition(l-1)-PositionFromLine(l-1);
	int i2=GetLineIndentPosition(l2)-PositionFromLine(l2);
	if (l2<ln && i1<i2) return; // si estaba dentro no se hace nada
	// ver que dice la siguiente para que no coincida con lo que vamos a agregar
	wxString sl2=GetLine(l2); sl2.MakeUpper(); 
	int i=0, sl=sl2.Len(); 
	while (i<sl && sl2[i]==' '||sl2[i]=='\t')i++;
	if (i) sl2.Remove(0,i);
	// agregar FinAlgo
	if (btype==BT_PARA) {
		if (sl2.StartsWith("FINPARA") || sl2.StartsWith("FIN PARA")) return;
		InsertText(PositionFromLine(l+1),"FinPara\n");
		IndentLine(l+1,true);
	} else if (btype==BT_SI) {
		if (sl2.StartsWith("FINSI") || sl2.StartsWith("FIN SI")) return;
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
	if (current_calltip_is_dwell) HideCalltip(true);
}

void mxSource::OnToolTipTime (wxStyledTextEvent &event) {
	if (!config->rt_syntax || !main_window->IsActive()) return; 
	int p = event.GetPosition();
	int s = GetStyleAt(p);
	if (s&(wxSTC_INDIC0_MASK|wxSTC_INDIC2_MASK)) {
		unsigned int l=LineFromPosition(p);
		if (rt_errors.GetCount()>l && rt_errors[l].Len()) ShowRealTimeError(p,rt_errors[l].Mid(0,rt_errors[l].Len()-1),true);
	}
}
