#include "mxDesktopTestGrid.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "DebugManager.h"
using namespace std;

mxDesktopTestGrid *desktop_test=NULL;

BEGIN_EVENT_TABLE(mxDesktopTestGrid, wxGrid)
	EVT_GRID_LABEL_RIGHT_CLICK(mxDesktopTestGrid::OnLabelRightClick)
	EVT_GRID_LABEL_LEFT_DCLICK(mxDesktopTestGrid::OnLabelDblClick)
	EVT_GRID_CELL_LEFT_DCLICK(mxDesktopTestGrid::OnCellDblClick)
	EVT_GRID_CELL_RIGHT_CLICK(mxDesktopTestGrid::OnCellRightClick)
	EVT_GRID_SELECT_CELL(mxDesktopTestGrid::OnSelectCell)
	EVT_SIZE(mxDesktopTestGrid::OnResize)
	EVT_GRID_COL_SIZE(mxDesktopTestGrid::OnColResize)
END_EVENT_TABLE()
	
// cantidad de columnas fijas
#define PRECOLS 2
	
mxDesktopTestGrid::mxDesktopTestGrid(wxWindow *parent, wxWindowID id ):wxGrid(parent,id, wxDefaultPosition,wxSize(400,300)) {
	EnableDragColMove();
	desktop_test=this;
	created=false;
	editable=true;
	sel_col=0;
	CreateGrid(rows_num=0,PRECOLS);
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
	SetCellHighlightPenWidth(0);
	old_size=100;
	cols_sizes=NULL;
	created=true;
}

void mxDesktopTestGrid::OnLabelDblClick(wxGridEvent &event) {
	if (!editable || event.GetCol()<PRECOLS) return;
	wxString val=wxGetTextFromUser("Variable o expresión a evaluar:","Prueba de Escritorio",GetColLabelValue(event.GetCol()));
	if (val.size()) {
		variables[event.GetCol()-PRECOLS]=val;
		SetColLabelValue(event.GetCol(),val);
		Refresh();
	}
}

void mxDesktopTestGrid::OnLabelRightClick(wxGridEvent &event) {
	if (!editable || event.GetCol()<PRECOLS) return;
	
	int res=wxMessageBox(wxString("¿Eliminar la columna \"")<<GetColLabelValue(event.GetCol())<<"\" de la tabla?", "Prueba de escritorio", wxYES_NO,main_window);
	if (res==wxYES) {
		variables.RemoveAt(event.GetCol()-PRECOLS);
		SetCols();
	}
}

void mxDesktopTestGrid::OnCellDblClick(wxGridEvent &event) {
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

void mxDesktopTestGrid::OnResize(wxSizeEvent &evt) {
	if (created) {
		BeginBatch();
		int w=evt.GetSize().GetWidth()-GetColSize(0)-GetColSize(1);
		double p = double(w)/old_size;
		int cols_num=GetNumberCols();
		for (int i=PRECOLS;i<cols_num;i++) {
			cols_sizes[i-PRECOLS]*=p;
			SetColSize(i,int(cols_sizes[i-PRECOLS]));
		}
		old_size=w;
		EndBatch();
	}
	evt.Skip();
}

void mxDesktopTestGrid::OnColResize(wxGridSizeEvent &evt) {
	if (!created) return;
	int col=evt.GetRowOrCol();
	if (old_size && col>=PRECOLS) { evt.Skip(); cols_sizes[col-PRECOLS]=GetColSize(col); }
}

const wxArrayString &mxDesktopTestGrid::GetDesktopVars() {
	return variables;
}

void mxDesktopTestGrid::SetCols() {
	Freeze();
	BeginBatch();
	created=false;
	DeleteRows(0,rows_num);
	int cols_num=GetNumberCols();
	DeleteCols(PRECOLS,cols_num-PRECOLS);
	unsigned int c = variables.GetCount();
	InsertCols(PRECOLS,c);
	old_size = GetSize().GetWidth()-GetColSize(0)-GetColSize(1);
	float col_size = c?old_size/c:0;
	if (col_size<10) col_size=10;
	if (cols_sizes) delete [] cols_sizes;
	cols_sizes = new float[c];
	for (unsigned int i=0;i<c;i++) {
		SetColSize(i+PRECOLS,int(cols_sizes[i]=col_size));
		SetColLabelValue(i+PRECOLS,variables[i]);
	}
	rows_num=0;
	EndBatch();
	Thaw();
	created=true;
}

static const wxColour back_colour(245,245,245);
static const wxColour change_colour(196,0,0);
static const wxColour normal_colour(0,0,0);
static const wxColour error_colour(128,128,128);

void mxDesktopTestGrid::SetAutoevaluation(long l, wxString value) {
	l+=PRECOLS-1;
	if (l>=PRECOLS && l<GetNumberCols() && rows_num) SetCellValue(rows_num-1,l,value);
	if (value.StartsWith("<<"))
		SetCellTextColour(rows_num-1,l,error_colour);
	else if (rows_num==1 || GetCellValue(rows_num-2,l)!=value) 
		SetCellTextColour(rows_num-1,l,change_colour);
	else
		SetCellTextColour(rows_num-1,l,normal_colour);
	SetCellBackgroundColour(rows_num-1,l,back_colour);
}

void mxDesktopTestGrid::SetLine(wxString proc, long line, long i) {
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

void mxDesktopTestGrid::ResetTest() {
	DeleteRows(0,rows_num);
	rows_num=0;
}

void mxDesktopTestGrid::OnSelectCell(wxGridEvent &evt) {
	sel_col = evt.GetCol(); evt.Skip();
}

void mxDesktopTestGrid::OnCellRightClick (wxGridEvent & event) {
	event.Skip();
	OnCellDblClick(event);
}

void mxDesktopTestGrid::SetEditable (bool can_edit) {
	editable=can_edit;
}

void mxDesktopTestGrid::OnAddDesktopVar ( ) {
	wxString val=wxGetTextFromUser("Variable o expresión a evaluar:","Prueba de Escritorio","");
	if (val.size()) {
		variables.Add(val);
		SetCols();
//		AppendCols(1,false);
//		SetColLabelValue(GetNumberCols()-1,val);
	}
}

void mxDesktopTestGrid::OnClearVars ( ) {
	ResetTest();
	DeleteCols(PRECOLS,variables.GetCount(),false);
	SetColLabelValue(0,_T("Proceso/SubProceso"));
	SetColLabelValue(1,_T("Linea(inst)"));
	variables.Clear();
}

