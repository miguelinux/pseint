#ifndef MXSINGLECASEWINDOW_H
#define MXSINGLECASEWINDOW_H
#include <wx/frame.h>

class TestCase;

class mxSingleCaseWindow : public wxFrame {
private:
protected:
public:
	mxSingleCaseWindow(wxWindow *parent, const wxString &test_name, const TestCase &t);
};

#endif

