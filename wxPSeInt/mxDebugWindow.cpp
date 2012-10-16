#include <wx/sizer.h>
#include <wx/stattext.h>
#include "mxDebugWindow.h"
#include "ids.h"
#include <wx/button.h>
#include <wx/scrolbar.h>
#include "DebugManager.h"
#include "mxMainWindow.h"
#include "ConfigManager.h"
#include "mxProcess.h"
#include "mxHelpWindow.h"
#include "mxDesktopVarsEditor.h"
#include "mxEvaluateDialog.h"
#include "mxSubtitles.h"

mxDebugWindow *debug_panel=NULL;

BEGIN_EVENT_TABLE(mxDebugWindow,wxPanel)
	EVT_BUTTON(mxID_DEBUG_BUTTON, mxDebugWindow::OnDebugButton)
	EVT_BUTTON(mxID_DEBUG_PAUSE, mxDebugWindow::OnDebugPause)
	EVT_BUTTON(mxID_DEBUG_STEP, mxDebugWindow::OnDebugStep)
	EVT_BUTTON(mxID_DEBUG_HELP, mxDebugWindow::OnDebugHelp)
	EVT_BUTTON(mxID_DEBUG_EVALUATE, mxDebugWindow::OnDebugEvaluate)
	EVT_BUTTON(mxID_DEBUG_DESKTOP_VARS, mxDebugWindow::OnDebugDesktopVars)
	EVT_CHECKBOX(mxID_DEBUG_STEP_IN, mxDebugWindow::OnDebugCheckStepIn)
	EVT_CHECKBOX(mxID_DEBUG_SUBTITLES, mxDebugWindow::OnDebugCheckSubtitles)
END_EVENT_TABLE()	

mxDebugWindow::mxDebugWindow(wxWindow *parent):wxPanel(parent,wxID_ANY) {
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(new wxStaticText(this,wxID_ANY,_T(" Estado:")),wxSizerFlags().Proportion(0).Expand().Border(wxTOP,10));
	debug_status = new wxStaticText(this,wxID_ANY,_T("No iniciada"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
	sizer->Add(debug_status,wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(dp_button_run = new wxButton(this,mxID_DEBUG_BUTTON,_T("Comenzar")),wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(dp_button_pause = new wxButton(this,mxID_DEBUG_PAUSE,_T("Pausar/Continuar")),wxSizerFlags().Proportion(0).Expand());
	sizer->Add(dp_button_step = new wxButton(this,mxID_DEBUG_STEP,_T("Primer Paso")),wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	sizer->Add(dp_button_desktop_vars = new wxButton(this,mxID_DEBUG_DESKTOP_VARS,_T("Prueba de Esc...")),wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	dp_button_desktop_vars->SetToolTip(utils->FixTooltip("Con este botón puede configurar una tabla con un conjunto de variables o expresiones para que serán evaluadas en cada paso de la ejecución paso a paso y registradas en dicha tabla automáticamente para analizar luego la evolución de los datos y el algoritmo."));
	sizer->Add(dp_button_evaluate = new wxButton(this,mxID_DEBUG_EVALUATE,_T("Evaluar...")),wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	dp_button_evaluate->SetToolTip(utils->FixTooltip("Puede utilizar este botón para evaluar una expresión o conocer el valor de una variable durante la ejecución paso a paso. Para ello debe primero pausar el algoritmo."));
	sizer->Add(new wxStaticText(this,wxID_ANY,_T(" Velocidad:")),wxSizerFlags().Proportion(0).Expand().Border(wxTOP,10));
	debug_speed=new wxScrollBar(this,mxID_DEBUG_SLIDER);
	debug_speed->SetScrollbar(0,1,100,10);
	debug_speed->SetToolTip(utils->FixTooltip("Con este slider puede variar la velocidad con la que avanza automáticamente la ejecución paso a paso."));
	sizer->Add(debug_speed,wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10));
	dp_check_step_in=new wxCheckBox(this,mxID_DEBUG_STEP_IN,"Entrar en subprocesos");
	dp_check_step_in->SetToolTip(utils->FixTooltip("Cuando esta opción está activada y el proceso llega a la llamada de una función entra en dicha función y muestra pasa a paso cómo se ejecuta la misma, mientras que si está desactivada ejecuta la llamada completa en un solo paso sin mostrar la ejecución de la misma."));
	dp_check_step_in->SetValue(true);
	if (!config->lang.enable_user_functions) dp_check_step_in->Hide();
	sizer->Add(dp_check_step_in,wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10)); 
	
	sizer->AddSpacer(20);
	dp_check_subtitles=new wxCheckBox(this,mxID_DEBUG_SUBTITLES,"Explicar con detalle\ncada paso.");
	dp_check_subtitles->SetToolTip(utils->FixTooltip("Con esta opción, el intérprete explicará cada acción que realiza para cada instrucción, informando qué instrucciones se ejecutan, qué expresiones se analizan, qué decisiones se toman, etc. El objetivo de esta funcionalidad es ayudar a entender la forma correcta de interpretar un algoritmo."));
	dp_check_subtitles->SetValue(false);
	sizer->Add(dp_check_subtitles,wxSizerFlags().Proportion(0).Expand().Border(wxBOTTOM,10)); 
	
	sizer->Add(new wxButton(this,mxID_DEBUG_HELP,_T("Ayuda...")),wxSizerFlags().Proportion(0).Expand().Border(wxTOP,10));
	SetState(DS_STOPPED);
	this->SetSizerAndFit(sizer);
	evaluate_window = new mxEvaluateDialog(parent);
}

void mxDebugWindow::SetSpeed(int speed) {
	debug_speed->SetThumbPosition(speed);
}

void mxDebugWindow::SetState(ds_enum state) {
	ds_state = state;
	switch (state) {
	case DS_STARTING:
		dp_button_run->SetLabel(_T("Finalizar"));
		dp_button_run->SetToolTip(utils->FixTooltip("Utilice este botón para detener definitivamente la ejecución del algoritmo y abandonar el modo de ejecución paso a paso."));
		debug_status->SetLabel(_T("Iniciando"));
		dp_button_step->Disable();
		dp_button_step->SetLabel(_T("Avanzar un Paso"));
		dp_button_step->SetToolTip(utils->FixTooltip("Utilice este botón para avanzar ejecutar solamente la siguiente instrucción del algoritmo."));
		subtitles->button_next->Disable();
		subtitles->button_next->SetLabel("Continuar");
		dp_button_desktop_vars->Disable();
		break;
	case DS_STOPPED: 
		subtitles->button_next->SetLabel(_T("Comenzar"));
		subtitles->button_next->Enable();
		dp_button_run->SetLabel(_T("Comenzar"));
		dp_button_run->SetToolTip(utils->FixTooltip("Utilice este botón para que el algoritmo comience a ejecutarse automáticamente y paso a paso, señalando cada instrucción que ejecuta, según la velocidad definida en el menú configuración."));
		dp_button_step->SetLabel(_T("Primer Paso"));
		dp_button_step->SetToolTip(utils->FixTooltip("Utilice este botón para ejecutar solo la primer instrucción del algoritmo y pausar inmediatamente la ejecución del mismo."));
		dp_button_step->Enable();
		dp_button_desktop_vars->Enable();
		dp_button_evaluate->Disable();
		dp_button_step->Enable();
		dp_button_pause->Disable();
		debug_status->SetLabel(_T("No Iniciada"));
		break;
	case DS_FINALIZED:
		subtitles->button_next->SetLabel(_T("Cerrar"));
		subtitles->button_next->Enable();
		dp_button_run->SetLabel(_T("Cerrar"));
		dp_button_run->SetToolTip(utils->FixTooltip("Ha finalizado la ejecución del algoritmo. Utilice este botón para cerrar la ventana de la ejecución del mismo."));
		dp_button_pause->Disable();
		dp_button_step->Disable();
		debug_status->SetLabel(_T("Finalizada"));
		break;
	case DS_PAUSED:
		dp_button_step->Enable();
		subtitles->button_next->Enable();
		dp_button_pause->Enable();
		dp_button_pause->SetLabel(_T("Continuar"));
		dp_button_pause->SetToolTip(utils->FixTooltip("Utilice este botón para que el algoritmo continúe avanzando paso a paso automáticamente."));
		dp_button_evaluate->Enable();
		debug_status->SetLabel(_T("Pausado"));
		break;
	case DS_RESUMED:
		dp_button_step->Disable();
		subtitles->button_next->Disable();
		dp_button_pause->Enable();
		dp_button_pause->SetLabel(_T("Pausar"));
		dp_button_pause->SetToolTip(utils->FixTooltip("Utilice este botón para detener temporalmente la ejecución del algoritmo. Al detener el algoritmo puede observar el valor de las variables con el botón Evaluar."));
		dp_button_evaluate->Disable();
		debug_status->SetLabel(_T("Ejecutando"));
		break;
	case DS_STEP:
		dp_button_pause->Disable();
		dp_button_evaluate->Disable();
		dp_button_step->Disable();
		subtitles->button_next->Disable();
		debug_status->SetLabel(_T("Ejecutando"));
		break;
	case DS_NONE:
		debug_status->SetLabel(_T("Desconocido"));
	}
}

void mxDebugWindow::OnDebugButton(wxCommandEvent &evt) {
	if (evaluate_window->IsShown()) evaluate_window->Hide();
	if (debug->debugging) {
		debug->Stop();
	} else {
		mxSource *src=main_window->GetCurrentSource();
		if (src) StartDebugging(src,false);
	}
}

void mxDebugWindow::OnDebugPause(wxCommandEvent &evt) {
	if (evaluate_window->IsShown()) evaluate_window->Hide();
	if (ds_state==DS_STEP) return;
	debug->Pause();
}

void mxDebugWindow::OnDebugStep(wxCommandEvent &evt) {
	if (evaluate_window->IsShown()) evaluate_window->Hide();
	if (ds_state==DS_STEP) return;
	if (debug->debugging) {
		debug->Step();
	} else {
		mxSource *src=main_window->GetCurrentSource();
		if (src) StartDebugging(src,true);
	}
}

void mxDebugWindow::StartDebugging(mxSource *source, bool paused) {
	debug_status->SetLabel(_T("iniciando..."));
	bool mod = source->GetModify();
	source->SaveFile(config->temp_file);
	source->SetModify(mod);
	debug->should_pause = paused;
	if ( (new mxProcess(source,source->GetPageText()))->Debug(config->temp_file, true) )
		SetState(DS_STARTING);
}
void mxDebugWindow::OnDebugHelp(wxCommandEvent &evt) {
	if (!helpw) helpw = new mxHelpWindow();
	helpw->ShowHelp(_T("debug.html"));
}
void mxDebugWindow::OnDebugDesktopVars(wxCommandEvent &evt) {
	new mxDesktopVarsEditor(this);
}
void mxDebugWindow::OnDebugEvaluate(wxCommandEvent &evt) {
	evaluate_window->Show();
}

void mxDebugWindow::SetEvaluationValue (wxString val, char tipo) {
	evaluate_window->SetEvaluationValue(val,tipo);
}

bool mxDebugWindow::ProfileChanged ( ) {
	dp_check_step_in->Show(config->lang.enable_user_functions);
	Layout();
}

void mxDebugWindow::OnDebugCheckStepIn(wxCommandEvent &evt) {
	evt.Skip();
	debug->SetStepIn(dp_check_step_in->GetValue());
}

void mxDebugWindow::OnDebugCheckSubtitles(wxCommandEvent &evt) {
	evt.Skip();
	debug->SetSubtitles(dp_check_subtitles->GetValue());
	main_window->ShowSubtitles(dp_check_subtitles->GetValue());
}

void mxDebugWindow::ShowInEvaluateDialog(wxString s) {
	evaluate_window->Show();
	evaluate_window->EvaluateExpression(s);
}

void mxDebugWindow::SetSubtitles(bool on) {
	dp_check_subtitles->SetValue(on);
	wxCommandEvent evt;
	OnDebugCheckSubtitles(evt);
}

ds_enum mxDebugWindow::GetState ( ) {
	return ds_state;
}

