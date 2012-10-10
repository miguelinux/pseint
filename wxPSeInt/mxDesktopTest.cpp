#include "mxDesktopTest.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "DebugManager.h"
using namespace std;

mxDesktopTest *desktop_test=NULL;

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
	
// cantidad de columnas fijas
#define PRECOLS 2
	
mxDesktopTest::mxDesktopTest(wxWindow *parent, wxWindowID id ):wxGrid(parent,id, wxDefaultPosition,wxSize(400,300)) {
	desktop_test=this;
	created=false;
	sel_col=0;
	CreateGrid(rows_num=0,cols_num=PRECOLS);
	SetColLabelValue(0,_T("Proceso/SubProceso"));
	SetColLabelValue(1,_T("Linea(inst)"));
	SetColSize(0,wxGRID_AUTOSIZE);
	SetColSize(1,wxGRID_AUTOSIZE);
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

void mxDesktopTest::OnDblClick(wxGridEvent &event) {
	int r = event.GetRow();
	long line=-1, inst=-1;
	wxString pre=GetCellValue(r,1);
	if (pre.Contains('(')) {
		pre.AfterFirst('(').BeforeFirst(')').ToLong(&inst);
		pre=pre.BeforeFirst('(');
	}
	if (pre.ToLong(&line) && line!=-1) 
		debug->source->SelectInstruccion(line-1,inst-1);
}

void mxDesktopTest::OnResize(wxSizeEvent &evt) {
	if (created) {
		BeginBatch();
		int w=evt.GetSize().GetWidth()-GetColSize(0)-GetColSize(1);
		double p = double(w)/old_size;
		for (int i=PRECOLS;i<cols_num;i++) {
			cols_sizes[i-PRECOLS]*=p;
			SetColSize(i,int(cols_sizes[i-PRECOLS]));
		}
		old_size=w;
		EndBatch();
	}
	evt.Skip();
}

void mxDesktopTest::OnColResize(wxGridSizeEvent &evt) {
	if (!created) return;
	int col=evt.GetRowOrCol();
	if (old_size && col>=PRECOLS) { evt.Skip(); cols_sizes[col-PRECOLS]=GetColSize(col); }
}

const wxArrayString &mxDesktopTest::GetDesktopVars() {
	return variables;
}

void mxDesktopTest::SetDesktopVars(const wxArrayString &vars) {
	BeginBatch();
	created=false;
	variables=vars;
	DeleteRows(0,rows_num);
	DeleteCols(PRECOLS,cols_num-PRECOLS);
	unsigned int c = vars.GetCount();
	InsertCols(PRECOLS,c);
	old_size = GetSize().GetWidth()-GetColSize(0)-GetColSize(1);
	float col_size = c?old_size/c:0;
	if (col_size<10) col_size=10;
	if (cols_sizes) delete [] cols_sizes;
	cols_sizes = new float[c];
	for (unsigned int i=0;i<c;i++) {
		SetColSize(i+PRECOLS,int(cols_sizes[i]=col_size));
		SetColLabelValue(i+PRECOLS,vars[i]);
	}
	rows_num=0;
	cols_num=PRECOLS+c;
	EndBatch();
	created=true;
}

static const wxColour back_colour(245,245,245);
static const wxColour change_colour(196,0,0);
static const wxColour normal_colour(0,0,0);
static const wxColour error_colour(128,128,128);

void mxDesktopTest::SetAutoevaluation(long l, wxString value) {
	l+=PRECOLS-1;
	if (l>=PRECOLS && l<cols_num && rows_num) SetCellValue(rows_num-1,l,value);
	if (value.StartsWith("<<"))
		SetCellTextColour(rows_num-1,l,error_colour);
	else if (rows_num==1 || GetCellValue(rows_num-2,l)!=value) 
		SetCellTextColour(rows_num-1,l,change_colour);
	else
		SetCellTextColour(rows_num-1,l,normal_colour);
	SetCellBackgroundColour(rows_num-1,l,back_colour);
}

void mxDesktopTest::SetLine(wxString proc, long line, long i) {
	InsertRows(rows_num);
	SetCellValue(rows_num,0,proc);
	if (i>0)
		SetCellValue(rows_num,1,wxString()<<line<<"("<<i<<")");
	else
		SetCellValue(rows_num,1,wxString()<<line);
	SelectBlock(rows_num,1,rows_num,1);
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
