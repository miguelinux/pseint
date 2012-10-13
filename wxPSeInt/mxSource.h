#ifndef MXSOURCE_H
#define MXSOURCE_H
#include <wx/stc/stc.h>

#define mxSOURCE_BOLD 1
#define mxSOURCE_ITALIC 2
#define mxSOURCE_UNDERL 4
#define mxSOURCE_HIDDEN 8

#include <vector>
using namespace std;
class mxInputDialog;
class mxProcess;
class wxSocketBase;

enum {BT_NONE,BT_PARA,BT_SEGUN,BT_CASO,BT_REPETIR,BT_MIENTRAS,BT_SI,BT_SINO,BT_PROCESO,BT_SUBPROCESO,BT_FUNCION};

class mxSource : public wxStyledTextCtrl {
private:
	bool rt_running; // rt_syntax||highlight_blocks||show_vars
	int comp_from, comp_to;
	int last_s1,last_s2;
	bool is_example;
	mxProcess *flow;
	wxSocketBase *socket;
	int flow_id;
	int debug_line, debug_line_handler_1, debug_line_handler_2;
	wxString page_text;
	wxArrayString rt_errors;
	wxTimer *rt_timer; // se activa al cargar el pseudocodigo y al modificarlo, para llamar al rt_syntax
	
public:
	mxInputDialog *input;
	bool sin_titulo;
	wxString filename;
	void SetFileName(wxString afilename);
	void SetStyling(bool colour=true);
	void SetWords();
	static void SetAutocompletion();
	void SetStyle(int idx, const wxChar *fontName, int fontSize, const wxChar *foreground, const wxChar *background, int fontStyle);
	void SetFieldIndicator(int p1, int p2); // para los campos a completar en las plantillas de instrucciones/estructuras
	void UnExample();
	void SetExample();
	mxSource(wxWindow *parent, wxString ptext, wxString afilename=wxEmptyString, bool ais_example=false);
	~mxSource();
	
	void OnEditCut(wxCommandEvent &evt);
	void OnEditCopy(wxCommandEvent &evt);
	void OnEditPaste(wxCommandEvent &evt);
	void OnEditUndo(wxCommandEvent &evt);
	void OnEditRedo(wxCommandEvent &evt);
	void OnEditComment(wxCommandEvent &evt);
	void OnEditUnComment(wxCommandEvent &evt);
	void OnEditDuplicate(wxCommandEvent &evt);
	void OnEditDelete(wxCommandEvent &evt);
	void OnEditSelectAll(wxCommandEvent &evt);
	void OnEditToggleLinesUp (wxCommandEvent &event);
	void OnEditToggleLinesDown (wxCommandEvent &event);
	void OnModifyOnRO(wxStyledTextEvent &event);
	void OnCharAdded(wxStyledTextEvent &event);
	void OnUserListSelection (wxStyledTextEvent &event);
	void OnUpdateUI(wxStyledTextEvent &event);
	void SetModify(bool);
	void MessageReadOnly();
	
	int GetIndentLevel(int l, bool goup, int *btype=NULL, bool diff_proc_sub_func=false);
	void Indent(int l1, int l2);
	void IndentLine(int l, bool goup=true);
	void OnEditIndentSelection(wxCommandEvent &evt);
	void OnEditBeautifyCode(wxCommandEvent &evt);

	void EditFlow(mxProcess *proc, int id=-1);
	int GetFlowId();
	wxSocketBase *GetFlowSocket();
	void ReloadTemp(wxString file);
	void SetFlowSocket(wxSocketBase *s);
	bool HaveComments();
	
	void SetDebugLine(int l=-1, int i=-1); // para marcar donde va el paso a paso, -1 para desmarcar
	void SetDebugPause(); // cambia de marcador usando la ultima linea que recibio en SetDebugLine
	
	bool LineHasSomething(int l); // false si esta vacia o tiene solo comentarios
	
	
	void SetPageText(wxString ptext);
	wxString GetPageText();
	void OnSavePointReached(wxStyledTextEvent &evt);
	void OnSavePointLeft(wxStyledTextEvent &evt);
	
	// retorna las posiciones donde empieza y termina cada instruccion de una linea
	vector<int> &FillAuxInstr(int _l);
	void SelectInstruccion(int _l, int _i);
	
	void DoRealTimeSyntax();
	void ClearErrors();
	void MarkError(int l, int i, wxString str, bool special=false);
	void StartRTSyntaxChecking();
	void StopRTSyntaxChecking();
	void OnRealTimeSyntaxTimer(wxTimerEvent &te);
	void OnChange(wxStyledTextEvent &event);
	
	bool current_calltip_is_error, current_calltip_is_dwell;
	void ShowCalltip(int pos, const wxString &l, bool is_error=false, bool is_dwell=false);
 	void HideCalltip(bool if_is_error=true,bool if_is_not_error=true);
	void ShowRealTimeError(int pos, const wxString &l, bool is_dwell=false);
	
	void OnToolTipTime (wxStyledTextEvent &event);
	void OnToolTipTimeOut (wxStyledTextEvent &event);
	
	void TryToAutoCloseSomething(int l);
	
	void HighLight(wxString words);
	
	wxArrayInt blocks; // blocks[l1]=l2 guarda un bloque que va de l1 a l2
	wxArrayInt blocks_reverse; // blocks_reverse[l1]=l2 guarda un bloque que va de l2 a l1
	wxArrayInt blocks_markers; // arreglo de handlers de los markers insertados para resaltar un bloque
	void ClearBlocks(); // borra las listas de bloques (blocks y blocks_reverse)
	void AddBlock(int l1, int l2); // registra un bloque que va desde l1 a l2 en blocks y blocks_reverse, lo llama el rt_syntax)
	void UnHighLightBlock(); // borra el resaltado
	void HighLightBlock(); // resalta el bloque de la linea actual si es que hay y el rt_syntax no tiene trabajo pendiente
	
	DECLARE_EVENT_TABLE();
};

#endif

