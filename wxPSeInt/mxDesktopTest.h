#ifndef MXDESKTOPTEST_H
#define MXDESKTOPTEST_H
#include <wx/grid.h>


class mxDesktopTest: public wxGrid {
private:
	DECLARE_EVENT_TABLE();
	int selected_row, rows_num, cols_num;
	int old_size, sel_col;
	float *cols_sizes;
	bool created;
	wxArrayString variables;
public:
	mxDesktopTest(wxWindow *parent, wxWindowID id);
	void AddRow();
	~mxDesktopTest();
	void SetVars(wxArrayString &vars);
	void OnKey(wxKeyEvent &event);
	void OnDblClick(wxGridEvent &event);
	void OnRightClick(wxGridEvent &event);
	void OnSelectCell(wxGridEvent &evt);
//	void OnExploreArgs(wxCommandEvent &event);
//	void OnExploreLocals(wxCommandEvent &event);
//	void OnInspectArgs(wxCommandEvent &event);
//	void OnInspectLocals(wxCommandEvent &event);
//	void OnGotoPos(wxCommandEvent &event);
	void OnResize(wxSizeEvent &evt);
	void OnColResize(wxGridSizeEvent &evt);
	const wxArrayString &GetDesktopVars();
	void SetDesktopVars(const wxArrayString &vars);
	void SetAutoevaluation(long l, wxString value);
	void SetLine(long line);
	void ResetTest();
};

#endif

