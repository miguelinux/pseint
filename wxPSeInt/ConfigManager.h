#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <wx/string.h>
#include <wx/arrstr.h>
#include "../pseint/LangSettings.h"

#define _no_tty "<<sin configurar>>"

#define NO_PROFILE "..."

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
	wxString pseval_command;
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
	bool shape_colors; ///< usar diferentes colores de fondo para las distintas estructuras de control
	bool psdraw_nocrop; ///< no cortar labels largos en el diagrama (por defecto remplaza "xxxxxxxxxx" por "xxx...")
//	bool show_toolbar;
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

