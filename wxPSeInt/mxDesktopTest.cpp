#include "mxDesktopTest.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "DebugManager.h"
using namespace std;

BEGIN_EVENT_TABLE(mxDesktopTest, wxGrid)
//	EVT_KEY_DOWN(mxDesktopTest::OnKey)
	EVT_GRID_CELL_LEFT_DCLICK(mxDesktopTest::OnDblClick)
//	EVT_GRID_CELL_RIGHT_CLICK(mxDesktopTest::OnRightClick)
//	EVT_MENU(mxID_BACKTRACE_GOTO_POS,mxDesktopTest::OnGotoPos)
//	EVT_MENU(mxID_BACKTRACE_INSPECT_ARGS,mxDesktopTest::OnInspectArgs)
//	EVT_MENU(mxID_BACKTRACE_INSPECT_LOCALS,mxDesktopTest::OnInspectLocals)
//	EVT_MENU(mxID_BACKTRACE_EXPLORE_ARGS,mxDesktopTest::OnExploreArgs)
//	EVT_MENU(mxID_BACKTRACE_EXPLORE_LOCALS,mxDesktopTest::OnExploreLocals)
	EVT_GRID_SELECT_CELL(mxDesktopTest::OnSelectCell)
	EVT_SIZE(mxDesktopTest::OnResize)
	EVT_GRID_COL_SIZE(mxDesktopTest::OnColResize)
END_EVENT_TABLE()
	

mxDesktopTest::mxDesktopTest(wxWindow *parent, wxWindowID id ):wxGrid(parent,id, wxDefaultPosition,wxSize(400,300)) {
	created=false;
	sel_col=0;
	CreateGrid(cols_num=0,rows_num=0);
	InsertCols(); cols_num++;
	SetColLabelValue(0,_T("Linea"));
	SetRowLabelSize(0);
	EnableEditing(false);
	EnableDragRowSize(false);
//	SetColLabelSize(wxGrid::wxGridAutoSize);
	SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	SetDefaultCellAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
//	SetSelectionMode(wxGrid::wxGridSelectRows);
//	SetCellHighlightPenWidth(0);
	old_size=100;
	cols_sizes=NULL;
	created=true;
}

mxDesktopTest::~mxDesktopTest() {
	
}

void mxDesktopTest::OnDblClick(wxGridEvent &event) {
	int r = event.GetRow();
	long line=-1;
	GetCellValue(r,0).ToLong(&line);
	if (line!=-1) {
		main_window->SelectLine(debug->source,line);
	}
}

void mxDesktopTest::OnResize(wxSizeEvent &evt) {
	if (created) {
		BeginBatch();
		int w=evt.GetSize().GetWidth()-GetColSize(0);
		double p = double(w)/old_size;
		for (int i=1;i<cols_num;i++) {
			cols_sizes[i-1]*=p;
			SetColSize(i,int(cols_sizes[i-1]));
		}
		old_size=w;
		EndBatch();
	}
	evt.Skip();
}

void mxDesktopTest::OnColResize(wxGridSizeEvent &evt) {
	if (!created) return;
	int col=evt.GetRowOrCol();
	if (old_size && col>0) { evt.Skip(); cols_sizes[col-1]=GetColSize(col); }
}

const wxArrayString &mxDesktopTest::GetDesktopVars() {
	return variables;
}

void mxDesktopTest::SetDesktopVars(const wxArrayString &vars) {
	BeginBatch();
	created=false;
	variables=vars;
	DeleteRows(0,rows_num);
	DeleteCols(1,cols_num-1);
	unsigned int c = vars.GetCount();
	InsertCols(1,c);
	old_size = GetSize().GetWidth()-GetColSize(0);
	float col_size = c?old_size/c:0;
	if (col_size<10) col_size=10;
	if (cols_sizes) delete [] cols_sizes;
	cols_sizes = new float[c];
	for (unsigned int i=0;i<c;i++) {
		SetColSize(i+1,int(cols_sizes[i]=col_size));
		SetColLabelValue(i+1,vars[i]);
	}
	rows_num=0;
	cols_num=1+c;
	EndBatch();
	created=true;
}

static const wxColour back_colour(245,245,245);
static const wxColour change_colour(196,0,0);
static const wxColour normal_colour(0,0,0);
static const wxColour error_colour(128,128,128);

void mxDesktopTest::SetAutoevaluation(long l, wxString value) {
	if (l>0 && rows_num) SetCellValue(rows_num-1,l,value);
	if (value.StartsWith("<<"))
		SetCellTextColour(rows_num-1,l,error_colour);
	else if (rows_num==1 || GetCellValue(rows_num-2,l)!=value) 
		SetCellTextColour(rows_num-1,l,change_colour);
	else
		SetCellTextColour(rows_num-1,l,normal_colour);
	SetCellBackgroundColour(rows_num-1,l,back_colour);
}

void mxDesktopTest::SetLine(long line, long i) {
	InsertRows(rows_num);
	if (i>0)
		SetCellValue(rows_num,0,wxString()<<line<<"("<<i<<")");
	else
		SetCellValue(rows_num,0,wxString()<<line);
	SelectRow(rows_num);
	SelectCol(0);
	MakeCellVisible(rows_num,sel_col);
	rows_num++;
}

void mxDesktopTest::ResetTest() {
	DeleteRows(0,rows_num);
	rows_num=0;
}

void mxDesktopTest::OnSelectCell(wxGridEvent &evt) {
	sel_col = evt.GetCol(); evt.Skip();
}
