#ifndef MXDEBUGWINDOW_H
#define MXDEBUGWINDOW_H
#include <wx/panel.h>

class wxButton;
class wxScrollBar;
class wxStaticText;
class mxSource;
class wxCheckBox;
class mxEvaluateDialog;

enum ds_enum {DS_NONE,DS_STEP,DS_STARTING,DS_PAUSED,DS_RESUMED,DS_FINALIZED,DS_STOPPED};

class mxDebugWindow : public wxPanel {
private:
	mxEvaluateDialog *evaluate_window;
	ds_enum ds_state;
	wxScrollBar *debug_speed;
	wxButton *dp_button_run, *dp_button_step, *dp_button_pause, *dp_button_evaluate;
	wxCheckBox *dp_check_step_in;
	wxCheckBox *dp_check_subtitles;
	wxCheckBox *dp_check_desktop_test;
public:
	mxDebugWindow(wxWindow *parent);
	void SetSpeed(int speed);
	void SetState(ds_enum state);
	ds_enum GetState();
	void DebugStartFromGui(bool from_psdraw=false);
	void OnDebugButton(wxCommandEvent &evt);
	void OnDebugPause(wxCommandEvent &evt);
	void OnDebugStep(wxCommandEvent &evt);
	void OnDebugDesktopTest(wxCommandEvent &evt);
	void OnDebugEvaluate(wxCommandEvent &evt);
	void OnDebugHelp(wxCommandEvent &evt);
	void OnDebugCheckStepIn(wxCommandEvent &evt);
	void OnDebugCheckSubtitles(wxCommandEvent &evt);
	void OnDesktopTestPanelHide();
	bool IsDesktopTestEnabled();
	void StartDebugging(mxSource *source, bool paused);
	void SetEvaluationValue(wxString val, char tipo);
	void ProfileChanged();
	void ShowInEvaluateDialog(wxString s);
	void SetSubtitles(bool on);
	DECLARE_EVENT_TABLE();
};

extern mxDebugWindow *debug_panel;

#endif

