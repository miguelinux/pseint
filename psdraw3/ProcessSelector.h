#ifndef PROCESSSELECTOR_H
#define PROCESSSELECTOR_H

class ProcessSelector {
	int m_state;
	int m_selection;
	int m_anim_base;
	int m_anim_delta;
public:
	ProcessSelector();
	void Show();
	void Hide();
	void Draw();
	void ProcessMotion(int x, int y);
	void ProcessClick(int button, int state, int x, int y);
	void ProcessIddle();
	bool IsActive() const { return m_state!=0; }
	static void Initialize();
};

extern ProcessSelector *process_selector;

#endif

