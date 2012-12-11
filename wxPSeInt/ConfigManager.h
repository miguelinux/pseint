#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <wx/string.h>
#include <wx/arrstr.h>

#define _no_tty "<<sin configurar>>"

struct LangSettings {
	bool force_define_vars;
	bool force_init_vars;
	bool force_dot_and_comma;
	bool base_zero_arrays;
	bool allow_concatenation;
	bool use_nassi_schneiderman;
	bool allow_dinamyc_dimensions;
	bool overload_equal;
	bool coloquial_conditions;
	bool lazy_syntax;
	bool word_operators;
	bool enable_user_functions;
	bool enable_string_functions;
	bool operator!=(const LangSettings &o) const {
		return 
			force_define_vars!=o.force_define_vars ||
			force_init_vars!=o.force_init_vars ||
			force_dot_and_comma!=o.force_dot_and_comma ||
			base_zero_arrays!=o.base_zero_arrays ||
			allow_concatenation!=o.allow_concatenation ||
			use_nassi_schneiderman!=o.use_nassi_schneiderman ||
			allow_dinamyc_dimensions!=o.allow_dinamyc_dimensions ||
			overload_equal!=o.overload_equal ||
			coloquial_conditions!=o.coloquial_conditions||
			lazy_syntax!=o.lazy_syntax ||
			enable_string_functions!=o.enable_string_functions||
			enable_user_functions!=o.enable_user_functions ||
			word_operators!=o.word_operators;
	}
	void Reset() {
		force_define_vars=false;
		force_init_vars=false;
		force_dot_and_comma=false;
		base_zero_arrays=false;
		allow_concatenation=true;
		use_nassi_schneiderman=false;
		allow_dinamyc_dimensions=true;
		overload_equal=true;
		coloquial_conditions=true;
		lazy_syntax=true;
		word_operators=true;
		enable_string_functions=true;
		enable_user_functions=true;
	}
	LangSettings() { Reset(); }
};

class ConfigManager {
private:
	wxString tty_command;
public:
	bool use_psterm;
	wxString GetTTYCommand(); // terminal a usar si no se usa la propia (en GNU/Linux, hay que probar algunas para ver cual hay instalada)
	
	int version; // version del archivo de configuración que se leyó al inicializar
	int comm_port;
	int debug_port;
	bool fixed_port;
	bool check_for_updates;
	wxString pseint_dir;
	wxString home_dir;
	wxString filename;
	wxArrayString last_files;
	wxString images_path;
	wxString pseint_command;
	wxString psterm_command;
	wxString psdraw_command;
	wxString psdraw2_command;
	wxString psexport_command;
	wxString last_dir;
	wxString temp_dir;
	wxString /*temp*/_file;
//	wxString temp_out;
//	wxString temp_draw;
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
	bool show_vars;
	bool show_commands;
	bool show_debug_panel;
	bool auto_quickhelp;
	bool autocomp;
	bool highlight_blocks;
	bool autoclose;
	bool calltip_helps;
	bool rt_syntax;
	bool smart_indent;
	int font_size;
	int tabw;
	int size_x, size_y;
	int pos_x, pos_y;
	int stepstep_speed;
	bool maximized;
	
	ConfigManager(wxString apath);
	void LoadDefaults();
	wxString LoadProfile(wxString pname);
	void Read();
	void Save();
	~ConfigManager();
	
	int GetCommPort();
	int GetDebugPort();
	
//	wxString GetTempPSC();
//	wxString GetTempPSD();
//	wxString GetTempOUT();
	
};

extern ConfigManager *config;

#endif

