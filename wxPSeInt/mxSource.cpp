#include "mxSource.h"
#include "ConfigManager.h"
#include "ids.h"
#include "mxDropTarget.h"
#include "DebugManager.h"
#include <wx/clipbrd.h>
#include <iostream>
#include <wx/process.h>
using namespace std;

const wxChar *mxSourceWords1 =
	_T("leer proceso definir como dimension si entonces sino segun hacer hasta que para con paso ")
	_T("repetir mientras de otro modo escribir finpara ")
	_T("fin finproceso finsi finmientras finsegun ")
	_T("verdadero falso numerico caracter logico logica entero entera texto cadena numeros enteros reales enteras numericos numericas cadenas logicos logicas ")
	_T("borrar limpiar pantalla borrarpantalla limpiarpantalla imprimir mostrar esperar tecla esperartecla");
//	_T("según finsegún opción ") // scintilla no funciona con los acentos

const wxChar *mxSourceWords1_op =
	_T(" y no o mod ");

const wxChar *mxSourceWords1_extra =
	_T("es sies sinsaltar sin saltar sinbajar bajar opcion caso desde imprimir cada ");

const wxChar *mxSourceWords1_conds =
	_T("es par impar igual divisible multiplo distinto de por cero positivo negativo negativa positiva entero mayor menor");

const wxChar* mxSourceWords2 =
	_T("cos sen tan acos asen atan raiz rc ln abs exp azar trunc redon ");

BEGIN_EVENT_TABLE (mxSource, wxStyledTextCtrl)
	EVT_STC_UPDATEUI (wxID_ANY, mxSource::OnUpdateUI)
	EVT_STC_CHARADDED (wxID_ANY, mxSource::OnCharAdded)
	EVT_STC_USERLISTSELECTION (wxID_ANY, mxSource::OnUserListSelection)
	EVT_STC_ROMODIFYATTEMPT (wxID_ANY, mxSource::OnModifyOnRO)
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
END_EVENT_TABLE()

const wxChar *mxSource::comp_list[MAX_COMP_SIZE];
const wxChar *mxSource::comp_text[MAX_COMP_SIZE];
int mxSource::comp_count=-1;

mxSource::mxSource (wxWindow *parent, wxString afilename, bool ais_example) : wxStyledTextCtrl (parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxVSCROLL) {

	flow=NULL;
	input=NULL;
	
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
		comp_list[comp_count]=_T("FinSi"); comp_text[comp_count++]=_T("FinSi\n");
		comp_list[comp_count]=_T("FinMientras"); comp_text[comp_count++]=_T("FinMientras\n");
		comp_list[comp_count]=_T("FinPara"); comp_text[comp_count++]=_T("FinPara\n");
		comp_list[comp_count]=_T("FinProceso"); comp_text[comp_count++]=_T("FinProceso\n");
		comp_list[comp_count]=_T("FinSegun"); comp_text[comp_count++]=_T("FinSegun\n");
		comp_list[comp_count]=_T("FinSi"); comp_text[comp_count++]=_T("FinSi\n");
		comp_list[comp_count]=_T("Leer"); comp_text[comp_count++]=_T("Leer ");
		comp_list[comp_count]=_T("Limpiar Pantalla"); comp_text[comp_count++]=_T("Limpiar Pantalla;");
		comp_list[comp_count]=_T("Mientras"); comp_text[comp_count++]=_T("Mientras ");
		comp_list[comp_count]=_T("Mientras Que"); comp_text[comp_count++]=_T("Mientras Que ");
		comp_list[comp_count]=_T("Opcion "); comp_text[comp_count++]=_T("Opcion ");
		comp_list[comp_count]=_T("Otro Modo:"); comp_text[comp_count++]=_T("Otro Modo:\n");
		comp_list[comp_count]=_T("Para"); comp_text[comp_count++]=_T("Para ");
		comp_list[comp_count]=_T("Proceso"); comp_text[comp_count++]=_T("Proceso ");
		comp_list[comp_count]=_T("Repetir"); comp_text[comp_count++]=_T("Repetir\n");
		comp_list[comp_count]=_T("Segun"); comp_text[comp_count++]=_T("Segun ");
		comp_list[comp_count]=_T("Sin Saltar"); comp_text[comp_count++]=_T("Sin Saltar");
		comp_list[comp_count]=_T("Sino"); comp_text[comp_count++]=_T("Sino\n");
		comp_list[comp_count]=_T("Verdadero"); comp_text[comp_count++]=_T("Verdadero");
	}
	SetMarginType (0, wxSTC_MARGIN_NUMBER);
	SetMarginWidth (0, TextWidth (wxSTC_STYLE_LINENUMBER, _T(" XXX")));
	StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (_T("DARK GRAY")));
	StyleSetBackground (wxSTC_STYLE_LINENUMBER, *wxWHITE);

	IndicatorSetStyle(0,wxSTC_INDIC_SQUIGGLE);
	IndicatorSetStyle(1,wxSTC_INDIC_BOX);
	IndicatorSetForeground (0, 0x0000ff);
	IndicatorSetForeground (1, 0x005555);
	
	SetDropTarget(new mxDropTarget());
	
	if (is_example) SetReadOnly(true);
	
}

mxSource::~mxSource() {
	debug->Close(this);
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
		CallTipCancel();
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
		if (!config->smart_indent) {
			int currentLine = GetCurrentLine();
			int lineInd = 0;
			if (currentLine > 0)
				lineInd = GetLineIndentation(currentLine - 1);
			if (lineInd == 0) return;
			SetLineIndentation (currentLine, lineInd);
			GotoPos(GetLineIndentPosition(currentLine));
		} else {
//			if (GetCurrentPos()>1&&GetCharAt(GetCurrentPos()-2)==':') {
//				int p=GetCurrentPos();
//				if (GetStyleAt(p-2)==wxSTC_C_OPERATOR) {
//					int l=GetCurrentLine();
//					IndentLine(l-1);
//				}
//			}
			int currentLine = GetCurrentLine();
			if (currentLine>0) IndentLine(currentLine-1);
			IndentLine(currentLine);
//			int lineInd = 0;
//			if (currentLine > 0)
//				lineInd = GetLineIndentation(currentLine - 1);
//			int p = GetLineIndentPosition(currentLine-1);
//			
//			if (GetTextRange(p,p+8).CmpNoCase(_T("proceso "))==0)
//				lineInd = lineInd + config->tabw;
//			else if (GetTextRange(p,p+5).CmpNoCase(_T("para "))==0)
//				lineInd = lineInd + config->tabw;
//			else if (GetTextRange(p,p+6).CmpNoCase(_T("segun "))==0)
//				lineInd = lineInd + config->tabw;
//			else if (GetTextRange(p,p+3).CmpNoCase(_T("si "))==0)
//				lineInd = lineInd + config->tabw;
//			else if (GetTextRange(p,p+9).CmpNoCase(_T("entonces "))==0)
//				lineInd = lineInd + config->tabw;
//			else if (GetTextRange(p,p+5).CmpNoCase(_T("sino "))==0)
//				lineInd = lineInd + config->tabw;
//			else if (GetTextRange(p,p+9).CmpNoCase(_T("mientras "))==0)
//				lineInd = lineInd + config->tabw;
//			else if (GetTextRange(p,p+7).CmpNoCase(_T("repetir"))==0)
//				lineInd = lineInd + config->tabw;
//			
//			if (lineInd == 0) return;
//			SetLineIndentation (currentLine, lineInd);
			GotoPos(GetLineIndentPosition(currentLine));
		}
	} 
//	else if ((chr==' '||chr=='\t')&&config->smart_indent&&GetCurrentPos()>1&&GetCharAt(GetCurrentPos()-2)==':') {
//		int p=GetCurrentPos();
//		if (GetStyleAt(p-2)==wxSTC_C_OPERATOR) {
//			int l=GetCurrentLine();
//			IndentLine(l);
//		}
//	} 
	if (AutoCompActive()) {
		comp_to=GetCurrentPos();
	} else if (chr==' ' && config->autocomp) {
		int p2=comp_to=GetCurrentPos();
		int s=GetStyleAt(p2-1);
		if (s==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_STRING || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRINGEOL) return;
		int p1=comp_from=WordStartPosition(p2-1,true);
		if (p2-p1==4 && GetTextRange(p1,p2-1).Upper()=="FIN") {
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
			if (p2-p1==3 && GetTextRange(p1,p2-1).Upper()=="ES") {
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
			if (GetTextRange(p-3,p+1).Upper()=="FIN ") return;
			if (text==_T("leer")||text==_T("definir"))
				CallTipShow(GetCurrentPos(),_T("{una o mas variables, separadas por comas}"));
			else if (text==_T("escribir")||text==_T("mostrar")||text==_T("imprimir"))
				CallTipShow(GetCurrentPos(),_T("{una o mas expresiones, separadas por comas}"));
			else if (text==_T("mientras"))
				CallTipShow(GetCurrentPos(),_T("{condicion, expresion logica}"));
			else if (text==_T("que"))
				CallTipShow(GetCurrentPos(),_T("{condicion, expresion logica}"));
			else if (text==_T("para"))
				CallTipShow(GetCurrentPos(),_T("{asignacion inicial: variable<-valor}"));
			else if (text==_T("desde"))
				CallTipShow(GetCurrentPos(),_T("{valor inicial}"));
			else if (text==_T("hasta")) {
				int l=LineFromPosition(p+1);
				while (p>0 && (GetCharAt(p)==' ' || GetCharAt(p)=='\t' || GetCharAt(p)=='\r' || GetCharAt(p)=='\n'))
					p--;
				if (LineFromPosition(p+1)==l)
					CallTipShow(GetCurrentPos(),_T("{valor final}"));
			} else if (text==_T("paso"))
				CallTipShow(GetCurrentPos(),_T("{valor del paso}"));
			else if (text==_T("si"))
				CallTipShow(GetCurrentPos(),_T("{condicion, expresion logica}"));
			else if (text==_T("entonces"))
				CallTipShow(GetCurrentPos(),_T("{acciones por verdadero}"));
			else if (text==_T("sino"))
				CallTipShow(GetCurrentPos(),_T("{acciones por falso}"));
			else if (text==_T("segun"))
				CallTipShow(GetCurrentPos(),_T("{variable o expresion numerica}"));
			else if (text==_T("opcion")||text==_T("sies")||text==_T("caso"))
				CallTipShow(GetCurrentPos(),_T("{variable o expresion numerica}"));
			else
				CallTipCancel();
		}
	}
}

void mxSource::SetModify (bool modif) {
	if (is_example) return;
	if (modif) {
		SetTargetStart(0); 
		SetTargetEnd(1);
		ReplaceTarget(GetTextRange(0,1));
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
		if (comp_from>5&&text.Last()==' '&&GetTextRange(comp_from-4,comp_from).Upper()=="FIN ")
			text.Last()='\n';
		ReplaceTarget(text);
		SetSelection(comp_from+text.Len(),comp_from+text.Len());
		int lfp=LineFromPosition(comp_from);
		if (text.Mid(0,3)==_T("Fin") || text==_T("Hasta Que ") || text==_T("Mientras Que ") || text.Mid(0,4)==_T("Sino")||text.Last()=='\n')
			IndentLine(lfp);
		if (text.Last()=='\n') {
			IndentLine(lfp+1);
			lfp=GetLineIndentPosition(lfp+1);
			SetSelection(lfp,lfp);
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

void mxSource::SetIndicator(int indic, int p1, int p2) {
	int lse = GetEndStyled();
	StartStyling(p1,wxSTC_INDICS_MASK);
	if (indic==1)
		wxStyledTextCtrl::SetStyling(p2-p1,wxSTC_INDIC1_MASK);
	else
		wxStyledTextCtrl::SetStyling(p2-p1,wxSTC_INDIC0_MASK);
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
	for (int i=b;i<=e;i++)
		IndentLine(i);
	if (pb==pe) {
		int p=GetLineIndentPosition(b);
		if (pb<p) SetSelection(p,p);
	}
}

void mxSource::IndentLine(int l) {
	bool segun=false,ignore_next=false;
	int cur=GetIndentLevel(l,&segun);
	wxString line=GetLine(l);
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
	if (segun && GetLineEndPosition(l)==GetLineIndentPosition(l)) cur-=4;
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

int mxSource::GetIndentLevel(int l, bool *segun) {
	if (segun) *segun=false;
	if (l<1) return 0;
	wxString line=GetLine(l-1);
	line<<_T(" ");
	int cur=GetLineIndentation(l-1);
	int n=line.Len();
	bool comillas=false,first_word=true,ignore_next=false;
	int wstart=0;
	char c;
	for (int i=0;i<n;i++) {
		c=line[i];
		if (c=='\'' || c=='\"') {
			comillas=!comillas;
		} else if (!comillas) {
			if (c=='/' && i+1<n && line[i+1]=='/')  return cur;
			if (c==':' && line[i+1]!='=') cur+=4;
			else if (c!='_'&&((c|32)<'a'||(c|32)>'z')) {
				if (wstart+1<i) {
					if (ignore_next)
						ignore_next=false;
					else {
						bool old_segun=segun?(*segun):false;
						if (segun) *segun=false;
						wxString word=line.SubString(wstart,i-1);
						word.MakeUpper();
						if (word==_T("SI")) cur+=4;
						else if (word==_T("SINO")) cur+=4;
						else if (word==_T("PROCESO")) cur+=4;
						else if (word==_T("MIENTRAS") && !(i+4<n && line.SubString(wstart,i+4).Upper()==_T("MIENTRAS QUE "))) cur+=4;
						else if (word==_T("SEGUN")) {
							cur+=8; if (segun) *segun=true;
						}
	//					else if (word==_T("HACER")) cur+=4;
						else if (word==_T("PARA")) cur+=4;
						else if (first_word && word==_T("REPETIR")) cur+=4;
						else if (first_word && word==_T("HACER")) cur+=4;
						else if (word==_T("FIN")) { /*cur-=4; */ignore_next=true; }
//						else if (!first_word) { /// para que estaba esto?
//							if (word==_T("DE") && i+10<n && line.SubString(wstart,i+10).Upper()==_T("DE OTRO MODO:")) cur-=4;
//							else if (word==_T("HASTA") && i+4<n && line.SubString(wstart,i+4).Upper()==_T("HASTA QUE ")) cur-=4;
//							else if (word==_T("MIENTRAS ") && i+4<n && line.SubString(wstart,i+4).Upper()==_T("MIENTRAS QUE ")) cur-=4;
//							else if (word==_T("FINSEGUN")) cur-=8;
//							else if (word==_T("FINMIENTRAS")) 
//								cur-=4;
//							else if (word==_T("FINPARA")) cur-=4;
//							else if (word==_T("FIN")) { cur-=4; ignore_next=true; }
//							else if (word==_T("FINSI")) cur-=4;
//							else if (word==_T("FINPROCESO")) cur-=4;
//	//						else if (word==_T("FIN")) cur-=4;
//							else 
//								if (segun) *segun=old_segun;
//						} else 
						if (first_word && segun) *segun=old_segun;
						first_word=false;
					}
				}
				wstart=i+1;
				if (c==';') first_word=true;
			}
		}
	}
//	if (segun) *segun=false;
	return cur;
}

void mxSource::Indent(int l1, int l2) {
	for (int i=l1;i<=l2;i++) 
		IndentLine(i);
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
}

void mxSource::EditFlow ( mxProcess *proc ) {
	flow=proc;
	SetReadOnly(proc!=NULL);
}

