#ifndef TOOLBAR_H
#define TOOLBAR_H

class ToolBar {
	int m_selection;
public:
	ToolBar() : m_selection(-1) {}
	void SetSelection(int i);
	void Draw();
	static void Initialize();
};

extern ToolBar *g_toolbar;

#endif

