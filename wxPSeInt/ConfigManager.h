#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <wx/string.h>
#include <wx/arrstr.h>

struct LangSettings {
	bool force_define_vars;
	bool force_init_vars;
	bool force_dot_and_comma;
	bool base_zero_arrays;
	bool allow_concatenation;
	bool allow_dinamyc_dimensions;
	bool overload_equal;
	bool coloquial_conditions;
	bool lazy_syntax;
	bool word_operators;
	bool operator!=(const LangSettings &o) const {
		return 
			force_define_vars!=o.force_define_vars ||
			force_init_vars!=o.force_init_vars ||
			force_dot_and_comma!=o.force_dot_and_comma ||
			base_zero_arrays!=o.base_zero_arrays ||
			allow_concatenation!=o.allow_concatenation ||
			allow_dinamyc_dimensions!=o.allow_dinamyc_dimensions ||
			overload_equal!=o.overload_equal ||
			coloquial_conditions!=o.coloquial_conditions||
			lazy_syntax!=o.lazy_syntax ||
			word_operators!=o.word_operators;
	}
	void Reset() {
		force_define_vars=0;
		force_init_vars=0;
		force_dot_and_comma=0;
		base_zero_arrays=0;
		allow_concatenation=1;
		allow_dinamyc_dimensions=1;
		overload_equal=1;
		coloquial_conditions=1;
		lazy_syntax=1;
		word_operators=1;
	}
	LangSettings() { Reset(); }
};

class ConfigManager {
private:
public:
	int flow_port;
	int debug_port;
	bool fixed_port;
	bool check_for_updates;
	wxString home_dir;
	wxString filename;
	wxArrayString last_files;
	wxString images_path;
	wxString pseint_command;
	wxString psdraw_command;
	wxString psdraw2_command;
	wxString psexport_command;
	bool have_tty_command;
	wxString tty_command;
	wxString last_dir;
	wxString temp_dir;
	wxString temp_file;
	wxString temp_out;
	wxString temp_draw;
	wxString help_dir;
	wxString examples_dir;
	wxString profiles_dir;
	wxString proxy;
	
	wxString profile;
	LangSettings lang;
	
	bool use_colors;
	bool colour_sintax;
//	bool high_res_flows;
	bool show_toolbar;
	bool show_commands;
	bool show_debug_panel;
	bool auto_quickhelp;
	bool autocomp;
	bool calltip_helps;
	bool smart_indent;
	int font_size;
	int tabw;
	int size_x, size_y;
	int pos_x, pos_y;
	int stepstep_speed;
	bool maximized;
	
	ConfigManager();
	void LoadDefaults();
	wxString LoadProfile(wxString pname);
	void Read();
	void Save();
	~ConfigManager();
	
	int GetFlowPort();
	int GetDebugPort();
	
};

extern ConfigManager *config;

#endif

