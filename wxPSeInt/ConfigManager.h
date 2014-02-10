#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <wx/string.h>
#include <wx/arrstr.h>

#define _no_tty "<<sin configurar>>"

#define NO_PROFILE "..."

struct LangSettings {
	wxString desc;
	bool force_define_vars;
	bool force_init_vars;
	bool force_semicolon;
	bool base_zero_arrays;
	bool allow_concatenation;
	bool use_nassi_schneiderman;
	bool use_alternative_io_shapes;
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
			force_semicolon!=o.force_semicolon ||
			base_zero_arrays!=o.base_zero_arrays ||
			allow_concatenation!=o.allow_concatenation ||
			use_alternative_io_shapes!=o.use_alternative_io_shapes||
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
		desc.Clear();
		force_define_vars=false;
		force_init_vars=false;
		force_semicolon=false;
		base_zero_arrays=false;
		allow_concatenation=true;
		use_nassi_schneiderman=false;
		use_alternative_io_shapes=false;
		allow_dinamyc_dimensions=true;
		overload_equal=true;
		coloquial_conditions=true;
		lazy_syntax=true;
		word_operators=true;
		enable_string_functions=true;
		enable_user_functions=true;
	}
	LangSettings() { Reset(); }
	bool Load(wxString fname);
	bool Save(wxString fname);
	void Log();
};

class ConfigManager {
private:
	wxString tty_command;
public:
	bool use_psterm; ///< utilizar psterm en lugar de la terminal del sistema (recomendado)
	bool use_dark_psterm; ///< esquema de colores de psterm, true=fondo negro, false=fondo blanco
	wxString GetTTYCommand(); ///< terminal a usar si no se usa la propia (en GNU/Linux, hay que probar algunas para ver cual hay instalada)
	
	int version; ///< version del archivo de configuración que se leyó al inicializar
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
	wxString psdrawe_command;
	wxString psdraw3_command;
	wxString psexport_command;
	wxString last_dir;
	wxString temp_dir;
//	wxString /*temp*/_file;
	wxString help_dir;
	wxString examples_dir;
	wxString profiles_dir;
	wxString proxy;
	
	wxString profile;
	LangSettings lang;
	
	bool reorganize_for_debug; ///< si reacomoda o no las ventanas al lanzar la ejecución paso a paso
	bool animate_gui; ///< si al mostrar u ocultar los paneles laterales lo hace con una animación (true) o de forma instantanea (false)
	bool use_colors;
	bool colour_sintax;
	bool show_toolbar;
	bool show_vars;
	bool show_opers;
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
	int stepstep_tspeed; ///< velocidad para la ejecucion paso a paso (posicion del scroll, no tiempo para el interprete)
	bool maximized;
	
	ConfigManager(wxString apath);
	void LoadDefaults();
	wxString LoadProfile(wxString pname);
	void Read();
	void Save();
	~ConfigManager();
	
	int GetCommPort();
	int GetDebugPort();
	
};

extern ConfigManager *config;

// para probar como seria la interfaz sin el result tree (usando el marcado de errores sobre el código y el panel de ayuda rápida para la información adicional)
#define _avoid_results_tree config->rt_syntax

#endif

