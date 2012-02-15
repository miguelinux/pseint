#ifndef MXSOURCE_H
#define MXSOURCE_H
#include <wx/stc/stc.h>

#define mxSOURCE_BOLD 1
#define mxSOURCE_ITALIC 2
#define mxSOURCE_UNDERL 4
#define mxSOURCE_HIDDEN 8

#define MAX_COMP_SIZE 100

class mxSource : public wxStyledTextCtrl {
private:
	static const wxChar *comp_list[MAX_COMP_SIZE];
	static const wxChar *comp_text[MAX_COMP_SIZE];
	int comp_from, comp_to;
	int last_s1,last_s2;
	static int comp_count;
	bool is_example;
public:
	bool sin_titulo;
	wxString filename;
	void SetFileName(wxString afilename);
	void SetStyling(bool colour=true);
	void SetWords();
	void SetStyle(int idx, const wxChar *fontName, int fontSize, const wxChar *foreground, const wxChar *background, int fontStyle);
	void SetIndicator(int indic, int p1, int p2);
	void UnExample();
	void SetExample();
	mxSource(wxWindow *parent, wxString afilename=wxEmptyString, bool ais_example=false);
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
	
	int GetIndentLevel(int l, bool *segun=NULL);
	void Indent(int l1, int l2);
	void IndentLine(int l);
	void OnEditIndentSelection(wxCommandEvent &evt);
	void OnEditBeautifyCode(wxCommandEvent &evt);
		
	DECLARE_EVENT_TABLE();
};

#endif

