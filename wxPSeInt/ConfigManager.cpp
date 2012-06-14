#include "ConfigManager.h"
#include <wx/filename.h>
#include "mxUtils.h"
#include <wx/textfile.h>
#include "version.h"
#include <wx/msgdlg.h>
using namespace std;

ConfigManager *config;

ConfigManager::ConfigManager() {
#if defined(_WIN32) || defined(__WIN32__)
	home_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("pseint"));
#else
	home_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T(".pseint"));
#endif
	if (!wxFileName::DirExists(home_dir))
		wxFileName::Mkdir(home_dir);
	if (wxFileName::FileExists(_T("config.here")))
		filename=_T("config.here");
	else
		filename = DIR_PLUS_FILE(home_dir,_T("config"));
	LoadDefaults();
	Read();
#if defined(__WIN32__)
#elif defined(__APPLE__)
	tty_command=_T("./mac-terminal-wrapper.bin");
#else
	wxFileName f_path = wxGetCwd(); 
	f_path.MakeAbsolute();
	pseint_command = DIR_PLUS_FILE(f_path.GetFullPath(),_T("pseint"));
	if (pseint_command.Contains(_T(" "))) pseint_command=wxString(_T("\""))<<pseint_command<<_T("\"");
	if (!have_tty_command) { // tratar de detectar automaticamente un terminal adecuado
		if (utils->GetOutput(_T("xterm -version")).Len()) {
			tty_command = _T("xterm -T \"$name\" -e");
		} else if (utils->GetOutput(_T("lxterminal -version")).Len()) {
			tty_command = _T("lxterminal -T \"$name\" -e");
		} else if (utils->GetOutput(_T("gnome-terminal --version")).Len()) {
			tty_command = _T("gnome-terminal --hide-menubar --disable-factory -t \"$name\" -x");
		} else if (utils->GetOutput(_T("konsole --version")).Len()) {
			if (utils->GetOutput(_T("konsole --version")).Find(_T("KDE: 3"))==wxNOT_FOUND) {
				tty_command = _T("konsole -e");
				wxMessageBox(_T("PSeInt requiere de una terminal para ejecutar los algoritmos. La unica terminal conocida encontrada en sus sistema es konosole, de KDE4. Esta genera problemas para ejecutar los algoritmos paso a paso y para obtener correctamente los errores en tiempo de ejecucion. Se recomienda instalar xterm."),_T("Terminal de ejecucion"));
			} else {
				tty_command = _T("konsole --nomenubar --notoolbar -T \"$name\" -e");
			}
		} else
			wxMessageBox(_T("No se ha encontrado una terminal conocida.\nInstale xterm,konsole o gnome-terminal; o\nconfigure el parametro \"Comando del\nTerminal\" en el cuadro de Preferencias.\""),_T("Terminal de ejecucion"));
	}
#endif
}

void ConfigManager::LoadDefaults() {
	profile=_T("...");
	use_colors=true;
	show_debug_panel = false;
	maximized = false;
//	high_res_flows = false;
	colour_sintax = true;
	show_commands = true;
	show_toolbar = true;
	lang.force_dot_and_comma = false;
	lang.force_define_vars = false;
	lang.force_init_vars = true;
	lang.base_zero_arrays = false;
	lang.allow_concatenation = true;
	lang.allow_dinamyc_dimensions = false;
	lang.overload_equal = true;
	lang.coloquial_conditions = true;
	lang.lazy_syntax = true;
	lang.word_operators = true;
	autocomp = true;
	autoclose = true;
	calltip_helps = true;
	auto_quickhelp = true;
	size_x = size_y = 0;
	images_path = _T("imgs");
	font_size = 10;
	tabw = 4;
	stepstep_speed=1;
	debug_port=55374;
	flow_port=55375;
	check_for_updates = true;
	fixed_port = false;
	rt_syntax = true;
	smart_indent = true;
	last_dir=wxFileName::GetHomeDir();
#if defined(_WIN32) || defined(__WIN32__)
	pseint_command = _T("pseint.exe");
	psdraw_command = _T("psdraw.exe");
	psdraw2_command = _T("psdraw2.exe");
	psexport_command = _T("psexport.exe");
	tty_command = _T("");
#else
	pseint_command = _T("./pseint");
	psdraw2_command = _T("./psdraw2");
	psexport_command = _T("./psexport");
	tty_command = _T("<<sin configurar>>");
#endif
	have_tty_command=false;
	help_dir = _T("help");
	proxy = _T("");
	profiles_dir = _T("perfiles");
	examples_dir = _T("ejemplos");
	temp_dir = home_dir;
	if (!wxFileName::DirExists(temp_dir))
		wxFileName::Mkdir(temp_dir);
	temp_file = DIR_PLUS_FILE(temp_dir,_T("temp.psc"));
	temp_out = DIR_PLUS_FILE(temp_dir,_T("temp.out"));
	temp_draw = DIR_PLUS_FILE(temp_dir,_T("temp.psd"));
}

void ConfigManager::Save() {
	wxTextFile fil(filename);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	fil.AddLine(wxString(_T("# generado por PSeInt "))<<VERSION<<_T("-"ARCHITECTURE));
	fil.AddLine(wxString(_T("images_path="))<<images_path);
	fil.AddLine(wxString(_T("pseint_command="))<<pseint_command);
	fil.AddLine(wxString(_T("psexport_command="))<<psexport_command);
	fil.AddLine(wxString(_T("psdraw_command="))<<psdraw_command);
	fil.AddLine(wxString(_T("psdraw2_command="))<<psdraw2_command);
	if (have_tty_command) fil.AddLine(wxString(_T("terminal="))<<tty_command);
	fil.AddLine(wxString(_T("temp_dir="))<<temp_dir);
	fil.AddLine(wxString(_T("temp_draw="))<<temp_draw);
	fil.AddLine(wxString(_T("temp_file="))<<temp_file);
	fil.AddLine(wxString(_T("temp_out="))<<temp_out);
	fil.AddLine(wxString(_T("last_dir="))<<last_dir);
	fil.AddLine(wxString(_T("help_dir="))<<help_dir);
	fil.AddLine(wxString(_T("proxy="))<<proxy);
	fil.AddLine(wxString(_T("profiles_dir="))<<profiles_dir);
	fil.AddLine(wxString(_T("profile="))<<profile);
	fil.AddLine(wxString(_T("examples_dir="))<<examples_dir);
	fil.AddLine(wxString(_T("rt_syntax="))<<(rt_syntax?1:0));
	fil.AddLine(wxString(_T("smart_indent="))<<(smart_indent?1:0));
//	fil.AddLine(wxString(_T("high_res_flows="))<<(high_res_flows?1:0));
	fil.AddLine(wxString(_T("colour_sintax="))<<(colour_sintax?1:0));
	fil.AddLine(wxString(_T("show_commands="))<<(show_commands?1:0));
	fil.AddLine(wxString(_T("show_debug_panel="))<<(show_debug_panel?1:0));
	fil.AddLine(wxString(_T("show_toolbar="))<<(show_toolbar?1:0));
	fil.AddLine(wxString(_T("calltip_helps="))<<(calltip_helps?1:0));
	fil.AddLine(wxString(_T("autocomp="))<<(autocomp?1:0));
	fil.AddLine(wxString(_T("autoclose="))<<(autoclose?1:0));
	fil.AddLine(wxString(_T("auto_quickhelp="))<<(auto_quickhelp?1:0));
	fil.AddLine(wxString(_T("use_colors="))<<(use_colors?1:0));
	fil.AddLine(wxString(_T("base_zero_arrays="))<<(lang.base_zero_arrays?1:0));
	fil.AddLine(wxString(_T("allow_concatenation="))<<(lang.allow_concatenation?1:0));
	fil.AddLine(wxString(_T("allow_dinamyc_dimensions="))<<(lang.allow_dinamyc_dimensions?1:0));
	fil.AddLine(wxString(_T("force_define_vars="))<<(lang.force_define_vars?1:0));
	fil.AddLine(wxString(_T("force_init_vars="))<<(lang.force_init_vars?1:0));
	fil.AddLine(wxString(_T("force_dot_and_comma="))<<(lang.force_dot_and_comma?1:0));
	fil.AddLine(wxString(_T("allow_word_operators="))<<(lang.word_operators?1:0));
	fil.AddLine(wxString(_T("overload_equal="))<<(lang.overload_equal?1:0));
	fil.AddLine(wxString(_T("coloquial_conditions="))<<(lang.coloquial_conditions?1:0));
	fil.AddLine(wxString(_T("lazy_syntax="))<<(lang.lazy_syntax?1:0));
	fil.AddLine(wxString(_T("maximized="))<<(maximized?1:0));
	fil.AddLine(wxString(_T("font_size="))<<font_size);
	fil.AddLine(wxString(_T("tabw="))<<tabw);
	fil.AddLine(wxString(_T("stepstep_speed="))<<stepstep_speed);
	fil.AddLine(wxString(_T("size_x="))<<size_x);
	fil.AddLine(wxString(_T("size_y="))<<size_y);
	fil.AddLine(wxString(_T("pos_x="))<<pos_x);
	fil.AddLine(wxString(_T("pos_y="))<<pos_y);
	fil.AddLine(wxString(_T("maximized="))<<maximized);	
	if (fixed_port) {
		fil.AddLine(wxString(_T("debug_port="))<<debug_port);	
		fil.AddLine(wxString(_T("flow_port="))<<flow_port);	
	}
	fil.AddLine(wxString(_T("check_for_updates="))<<(check_for_updates?1:0));	
	fil.AddLine(wxString(_T("fixed_port="))<<(fixed_port?1:0));	
	for (unsigned int i=0;i<last_files.GetCount();i++)
		fil.AddLine(wxString(_T("history="))<<last_files[i]);
	fil.AddLine(_T(""));
	
	fil.Write();
	fil.Close();
	
}

void ConfigManager::Read() {
	wxTextFile fil(filename);
	if (!fil.Exists()) return;
	fil.Open();
	wxString key, value;
	long l;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		if (str[0]=='#') {
			continue;
		} else {
			key=str.BeforeFirst('=');
			value=str.AfterFirst('=');
			if (key==_T("font_size")) { value.ToLong(&l); font_size=l; }
			else if (key==_T("tabWidth")) { value.ToLong(&l); tabw=l; }
			else if (key==_T("size_x")) { value.ToLong(&l); size_x=l; }
			else if (key==_T("size_y")) { value.ToLong(&l); size_y=l; }
			else if (key==_T("pos_x")) { value.ToLong(&l); pos_x=l; }
			else if (key==_T("pos_y")) { value.ToLong(&l); pos_y=l; }
			else if (key==_T("debug_port")) { value.ToLong(&l); debug_port=l; }
			else if (key==_T("flow_port")) { value.ToLong(&l); flow_port=l; }
			else if (key==_T("check_for_updates")) check_for_updates=utils->IsTrue(value);
			else if (key==_T("fixed_port")) fixed_port=utils->IsTrue(value);
			else if (key==_T("stepstep_speed")) { value.ToLong(&l); stepstep_speed=l; }
			else if (key==_T("rt_syntax")) rt_syntax=utils->IsTrue(value);
			else if (key==_T("smart_indent")) smart_indent=utils->IsTrue(value);
			else if (key==_T("show_commands")) show_commands=utils->IsTrue(value);
			else if (key==_T("show_debug_panel")) show_debug_panel=utils->IsTrue(value);
			else if (key==_T("show_toolbar")) show_toolbar=utils->IsTrue(value);
			else if (key==_T("auto_quickhelp")) auto_quickhelp=utils->IsTrue(value);
			else if (key==_T("calltip_helps")) calltip_helps=utils->IsTrue(value);
			else if (key==_T("autocomp")) autocomp=utils->IsTrue(value);
			else if (key==_T("autoclose")) autoclose=utils->IsTrue(value);
//			else if (key==_T("high_res_flows")) high_res_flows=utils->IsTrue(value);
			else if (key==_T("colour_sintax")) colour_sintax=utils->IsTrue(value);
			else if (key==_T("use_colors")) use_colors=utils->IsTrue(value);
			else if (key==_T("allow_dinamyc_dimensions")) lang.allow_dinamyc_dimensions=utils->IsTrue(value);
			else if (key==_T("allow_concatenation")) lang.allow_concatenation=utils->IsTrue(value);
			else if (key==_T("base_zero_arrays")) lang.base_zero_arrays=utils->IsTrue(value);
			else if (key==_T("force_define_vars")) lang.force_define_vars=utils->IsTrue(value);
			else if (key==_T("force_init_vars")) lang.force_init_vars=utils->IsTrue(value);
			else if (key==_T("force_dot_and_comma")) lang.force_dot_and_comma=utils->IsTrue(value);
			else if (key==_T("allow_word_operators")) lang.word_operators=utils->IsTrue(value);
			else if (key==_T("overload_equal")) lang.overload_equal=utils->IsTrue(value);
			else if (key==_T("coloquial_conditions")) lang.coloquial_conditions=utils->IsTrue(value);
			else if (key==_T("lazy_syntax")) lang.lazy_syntax=utils->IsTrue(value);
			else if (key==_T("images_path")) images_path=value;
			else if (key==_T("help_dir")) help_dir=value;
			else if (key==_T("proxy")) proxy=value;
			else if (key==_T("profiles_dir")) profiles_dir=value;
			else if (key==_T("profile")) profile=value;
			else if (key==_T("examples_dir")) examples_dir=value;
			else if (key==_T("last_dir")) last_dir=value;
			else if (key==_T("temp_dir")) temp_dir=value;
			else if (key==_T("temp_file")) temp_file=value;
			else if (key==_T("temp_draw")) temp_draw=value;
			else if (key==_T("temp_out")) temp_out=value;
			else if (key==_T("pseint_command")) pseint_command=value;
			else if (key==_T("psexport_command")) psexport_command=value;
			else if (key==_T("psdraw_command")) psdraw_command=value;
			else if (key==_T("psdraw2_command")) psdraw2_command=value;
			else if (key==_T("terminal")) { tty_command=value; have_tty_command=true; }
			else if (key==_T("history")) last_files.Add(value);
		}
	}
	fil.Close();
	LoadProfile(profile);
}

ConfigManager::~ConfigManager() {
	
}

wxString ConfigManager::LoadProfile(wxString pname) {
	profile=pname;
	wxTextFile fil(DIR_PLUS_FILE(profiles_dir,profile));
	if (!fil.Exists()) profile=_T("");
	if (profile==_T("<personalizado>") || profile==_T("")) {
		return _T(
			"Esta opcion le permite definir su propia configuracion. Utilice el boton \"Personalizar...\" para definirla."
			);
	} else {
		lang.Reset();
		fil.Open();
		wxString key, value; wxString desc;
		for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
			if (str[0]=='#') continue;
			key=str.BeforeFirst('=');
			value=str.AfterFirst('=');
			if (key==_T("allow_dinamyc_dimensions")) lang.allow_dinamyc_dimensions=utils->IsTrue(value);
			else if (key==_T("force_define_vars")) lang.force_define_vars=utils->IsTrue(value);
			else if (key==_T("force_init_vars")) lang.force_init_vars=utils->IsTrue(value);
			else if (key==_T("allow_concatenation")) lang.allow_concatenation=utils->IsTrue(value);
			else if (key==_T("force_dot_and_comma")) lang.force_dot_and_comma=utils->IsTrue(value);
			else if (key==_T("allow_word_operators")) lang.word_operators=utils->IsTrue(value);
			else if (key==_T("overload_equal")) lang.overload_equal=utils->IsTrue(value);
			else if (key==_T("coloquial_conditions")) lang.coloquial_conditions=utils->IsTrue(value);
			else if (key==_T("lazy_syntax")) lang.lazy_syntax=utils->IsTrue(value);
			else if (key==_T("base_zero_arrays")) lang.base_zero_arrays=utils->IsTrue(value);
			else if (key==_T("desc")) desc+=value+_T("\n");
		}
		fil.Close();
		return desc;
	}
}

int ConfigManager::GetFlowPort ( ) {
	if (fixed_port) return flow_port; else return flow_port+rand()%150+150;
}

int ConfigManager::GetDebugPort ( ) {
	if (fixed_port) return debug_port; else return debug_port+rand()%150;
}

