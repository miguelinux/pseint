#include "mxConfig.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include "../psdraw3/Global.h"
#include "../psdraw3/Load.h"
#include "../wxPSeInt/string_conversions.h"

enum { MID_NULL = wxID_HIGHEST, MID_ANCHO, MID_ALTO, MID_ZOOM, MID_CROP, MID_COMMENTS, MID_COLORS, MID_STYLE, MID_PROC };

BEGIN_EVENT_TABLE(mxConfig,wxDialog)
	EVT_TEXT(MID_ANCHO,mxConfig::OnAncho)
	EVT_TEXT(MID_ALTO,mxConfig::OnAlto)
	EVT_TEXT(MID_ZOOM,mxConfig::OnZoom)
	EVT_CHECKBOX(MID_COLORS,mxConfig::OnColors)
	EVT_CHECKBOX(MID_COMMENTS,mxConfig::OnComments)
	EVT_CHECKBOX(MID_CROP,mxConfig::OnCrop)
	EVT_COMBOBOX(MID_PROC,mxConfig::OnProc)
	EVT_COMBOBOX(MID_STYLE,mxConfig::OnStyle)
END_EVENT_TABLE()

static wxSizerFlags szflag;

static void AddWithLabel(wxWindow *parent, wxSizer *sizer, wxString text, wxWindow *control) {
	wxSizer *aux_sizer = new wxBoxSizer(wxHORIZONTAL);
	aux_sizer->Add( new wxStaticText(parent,wxID_ANY,text),szflag );
	aux_sizer->Add( control,szflag );
	sizer->Add (aux_sizer, szflag);
}

mxConfig::mxConfig():wxDialog(NULL,wxID_ANY,_Z("Guardar diagrama de flujo"),wxDefaultPosition,wxDefaultSize) {
	ignore_events=true;
	
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	szflag = wxSizerFlags().Border(wxALL,4).Center();
	
	wxArrayString astyles;
	astyles.Add(_Z("Clásico"));
	astyles.Add(_Z("Clásico (i/o alternativa)"));
	astyles.Add(_Z("Nassi-Shneiderman"));
	cm_style = new wxComboBox(this,MID_STYLE,_Z(""),wxDefaultPosition,wxDefaultSize,astyles,wxCB_READONLY|wxCB_SIMPLE);
	cm_style->SetSelection(Entity::nassi_shneiderman?2:(Entity::alternative_io?1:0));
	AddWithLabel(this,sizer,_Z("Estilo:"),cm_style);
	
	wxArrayString procs;
	for(unsigned int i=0;i<procesos.size();i++) 
		procs.Add((procesos[i]->lpre+procesos[i]->label).c_str());
	cm_proc = new wxComboBox(this,MID_PROC,_Z(""),wxDefaultPosition,wxDefaultSize,procs,wxCB_READONLY|wxCB_SIMPLE);
	AddWithLabel(this,sizer,_Z("(Sub)Proceso:"),cm_proc);
	
	tx_zoom = new wxTextCtrl(this,MID_ZOOM,_Z("100"));
	AddWithLabel(this,sizer,_Z("Zoom(%):"),tx_zoom);
	tx_ancho = new wxTextCtrl(this,MID_ANCHO,_Z(""));
	AddWithLabel(this,sizer,_Z("Ancho(px):"),tx_ancho);
	tx_alto = new wxTextCtrl(this,MID_ALTO,_Z(""));
	AddWithLabel(this,sizer,_Z("Alto(px):"),tx_alto);
	
	ch_comments = new wxCheckBox(this,MID_COMMENTS,_Z("Incluir comentarios"));
	ch_comments->SetValue(Entity::show_comments);
	sizer->Add (ch_comments,szflag);
	ch_colors = new wxCheckBox(this,MID_COLORS,_Z("Utilizar colores"));
	ch_colors->SetValue(Entity::shape_colors);
	sizer->Add (ch_colors,szflag);
	ch_crop  = new wxCheckBox(this,MID_CROP,_Z("Cortar textos largos"));
	sizer->Add (ch_crop,szflag);
	
	wxButton *ok = new wxButton(this,wxID_OK,_Z("Guardar"));
	wxButton *cancel = new wxButton(this,wxID_CANCEL,_Z("Cancelar"));
	wxSizer *aux_sizer = new wxBoxSizer(wxHORIZONTAL);
	aux_sizer->Add(cancel,szflag);
	aux_sizer->Add(ok,szflag);
	sizer->Add(aux_sizer,szflag);
	
	ok->SetDefault();
	
	tx_zoom->SetFocus();
	cm_proc->SetSelection(0);
	SetProceso(0);
	
	SetSizerAndFit(sizer);
	ignore_events=false;
}

static void get_wh(float z, int &rw, int &rh) {
	zoom=d_zoom=z;
	int h=0,wl=0,wr=0, margin=10;
	Entity::CalculateAll(true);
	Entity *real_start = start->GetTopEntity();
	real_start->Calculate(wl,wr,h); 
	int x0=real_start->x-wl,y0=real_start->y,x1=real_start->x+wr,y1=real_start->y-h;
	rw=( (x1-x0)+2*margin )*z;
	rh=( (y0-y1)+2*margin )*z;
}

void mxConfig::SetZoom (float f, int noup) {
	float prev_f=100;
	if (f==0) f=prev_f;
	int rw,rh; get_wh(f/100.f,rw,rh);
	wxString sz; sz.Printf("%.2f",f);
	if (noup!=0) tx_zoom->SetValue(sz);
	if (noup!=1) tx_ancho->SetValue(wxString()<<rw);
	if (noup!=2) tx_alto->SetValue(wxString()<<rh);
	prev_f=f;
}

void mxConfig::OnAncho (wxCommandEvent & evt) {
	if (ignore_events) return;
	ignore_events=true;
	evt.Skip();
	long w;
	if (tx_ancho->GetValue().ToLong(&w)) {
		int w1,h1; get_wh(1,w1,h1);
		float z=100.f*(float(w)/float(w1));
		SetZoom(z,1);
	}
	ignore_events=false;
}

void mxConfig::OnAlto (wxCommandEvent & evt) {
	if (ignore_events) return;
	ignore_events=true;
	evt.Skip();
	long h;
	if (tx_alto->GetValue().ToLong(&h)) {
		int w1,h1; get_wh(1,w1,h1);
		float z=100.f*(float(h)/float(h1));
		SetZoom(z,2);
	}
	ignore_events=false;
}

void mxConfig::OnZoom (wxCommandEvent & evt) {
	evt.Skip();
	if (ignore_events) return;
	ignore_events=true;
	double f=100;
	if (tx_zoom->GetValue().ToDouble(&f)) {
		SetZoom(f,0);
	}
	ignore_events=false;
}

void mxConfig::OnComments (wxCommandEvent &evt) {
	evt.Skip();
	if (ignore_events) return;
	Entity::show_comments=ch_comments->GetValue();
	Entity::CalculateAll();
	SetZoom();
}

void mxConfig::OnCrop (wxCommandEvent &evt) {
	evt.Skip();
	if (ignore_events) return;
	Entity::enable_partial_text=ch_crop->GetValue();
	Entity::CalculateAll(true);
	SetZoom();	
}

void mxConfig::SetProceso (int i) {
	SetProc(procesos[i]);
	SetZoom();
}

void mxConfig::OnProc (wxCommandEvent & evt) {
	evt.Skip();
	if (ignore_events) return;
	SetProceso(cm_proc->GetSelection());
}

void mxConfig::OnColors (wxCommandEvent & evt) {
	evt.Skip();
	Entity::shape_colors = ch_colors->GetValue();
}

void mxConfig::OnStyle (wxCommandEvent & evt) {
	evt.Skip();
	int i = cm_style->GetSelection();
	Entity::nassi_shneiderman = i==2;
	Entity::alternative_io = i==1;
	Entity::CalculateAll(true);
	SetZoom();	
}

