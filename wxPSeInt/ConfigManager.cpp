#include "ConfigManager.h"
#include <wx/filename.h>
#include "mxUtils.h"
#include <wx/textfile.h>
#include "version.h"
#include <wx/msgdlg.h>
#include "Logger.h"
#include "string_conversions.h"
#include "error_recovery.h"

ConfigManager *config;


ConfigManager::ConfigManager(wxString apath):lang(LS_INIT) {
	
	pseint_dir = apath;
	version=0; 
	
#if defined(_WIN32) || defined(__WIN32__)
	home_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),"pseint");
#else
	home_dir = DIR_PLUS_FILE(wxFileName::GetHomeDir(),".pseint");
#endif
	if (!wxFileName::DirExists(home_dir))
		wxFileName::Mkdir(home_dir);
	if (wxFileName::FileExists("config.here"))
		filename="config.here";
	else
		filename = DIR_PLUS_FILE(home_dir,"config");
	LoadDefaults();
	Read();
#if defined(__WIN32__)
#elif defined(__APPLE__)
	tty_command="./mac-terminal-wrapper.bin";
#else
	wxFileName f_path = wxGetCwd(); 
	f_path.MakeAbsolute();
//	pseint_command = DIR_PLUS_FILE(f_path.GetFullPath(),"pseint");
//	if (pseint_command.Contains(" ")) pseint_command=wxString("\"")<<pseint_command<<"\"";
#endif
	
	lang.Log();
	
	er_init(temp_dir);
}

void ConfigManager::LoadDefaults() {
	profile=NO_PROFILE;
	animate_gui=true;
	reorganize_for_debug=true;
	use_colors=true;
	show_debug_panel = false;
	maximized = false;
	psdraw_nocrop = false;
	shape_colors = true;
	colour_sintax = true;
	show_vars = false;
	show_opers = false;
	show_commands = true;
//	show_toolbar = true;
	autocomp = true;
	highlight_blocks = true;
	autoclose = true;
	calltip_helps = true;
	auto_quickhelp = true;
	size_x = size_y = 0;
	images_path = "imgs";
	font_size = 10;
	tabw = 4;
	stepstep_tspeed=50;
	debug_port=55374;
	comm_port=55375;
	use_dark_psterm = false;
	use_psterm = true;
	check_for_updates = true;
	fixed_port = false;
	rt_syntax = true;
	smart_indent = true;
	last_dir=wxFileName::GetHomeDir();
#if defined(_WIN32) || defined(__WIN32__)
	pseint_command = "pseint.exe";
	psterm_command = "psterm.exe";
	psdrawe_command = "psdrawE.exe";
	psdraw3_command = "psdraw3.exe";
	psexport_command = "psexport.exe";
	pseval_command = "pseval.exe";
	tty_command = "";
#else
	pseint_command = "./pseint";
	psterm_command = "./psterm";
	psdrawe_command = "./psdrawE";
	psdraw3_command = "./psdraw3";
	psexport_command = "./psexport";
	pseval_command = "./pseval";
	tty_command = _no_tty;
#endif
	help_dir = "help";
	proxy = "";
	profiles_dir = "perfiles";
	examples_dir = "ejemplos";
	temp_dir = home_dir;
	if (!wxFileName::DirExists(temp_dir))
		wxFileName::Mkdir(temp_dir);
}

void ConfigManager::Save() {
	wxTextFile fil(filename);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	fil.AddLine(wxString("# generado por PSeInt ")<<VERSION<<"-"ARCHITECTURE);
	fil.AddLine(wxString("version=")<<VERSION);
	fil.AddLine(wxString("images_path=")<<images_path);
//	fil.AddLine(wxString("pseint_command=")<<pseint_command);
//	fil.AddLine(wxString("psterm_command=")<<psterm_command);
//	fil.AddLine(wxString("psexport_command=")<<psexport_command);
//	fil.AddLine(wxString("psdrawe_command=")<<psdrawe_command);
//	fil.AddLine(wxString("psdraw3_command=")<<psdraw3_command);
	if (tty_command!=_no_tty) fil.AddLine(wxString("terminal=")<<tty_command);
	fil.AddLine(wxString("temp_dir=")<<temp_dir);
	fil.AddLine(wxString("last_dir=")<<last_dir);
	fil.AddLine(wxString("help_dir=")<<help_dir);
	fil.AddLine(wxString("proxy=")<<proxy);
	fil.AddLine(wxString("profiles_dir=")<<profiles_dir);
	fil.AddLine(wxString("profile=")<<profile);
	fil.AddLine(wxString("examples_dir=")<<examples_dir);
	fil.AddLine(wxString("rt_syntax=")<<(rt_syntax?1:0));
	fil.AddLine(wxString("smart_indent=")<<(smart_indent?1:0));
	fil.AddLine(wxString("psdraw_nocrop=")<<(psdraw_nocrop?1:0));
	fil.AddLine(wxString("shape_colors=")<<(shape_colors?1:0));
	fil.AddLine(wxString("colour_sintax=")<<(colour_sintax?1:0));
	fil.AddLine(wxString("show_vars=")<<(show_vars?1:0));
	fil.AddLine(wxString("show_opers=")<<(show_opers?1:0));
	fil.AddLine(wxString("show_commands=")<<(show_commands?1:0));
	fil.AddLine(wxString("show_debug_panel=")<<(show_debug_panel?1:0));
//	fil.AddLine(wxString("show_toolbar=")<<(show_toolbar?1:0));
	fil.AddLine(wxString("calltip_helps=")<<(calltip_helps?1:0));
	fil.AddLine(wxString("autocomp=")<<(autocomp?1:0));
	fil.AddLine(wxString("highlight_blocks=")<<(highlight_blocks?1:0));
	fil.AddLine(wxString("autoclose=")<<(autoclose?1:0));
	fil.AddLine(wxString("auto_quickhelp=")<<(auto_quickhelp?1:0));
	fil.AddLine(wxString("use_colors=")<<(use_colors?1:0));
	fil.AddLine(wxString("animate_gui=")<<(animate_gui?1:0));
	fil.AddLine(wxString("reorganize_for_debug=")<<(reorganize_for_debug?1:0));
	for(int i=0;i<LS_COUNT;i++) fil.AddLine(lang.GetConfigLine(i).c_str());
	fil.AddLine(wxString("maximized=")<<(maximized?1:0));
	fil.AddLine(wxString("font_size=")<<font_size);
	fil.AddLine(wxString("tabw=")<<tabw);
	fil.AddLine(wxString("stepstep_tspeed=")<<stepstep_tspeed);
	fil.AddLine(wxString("size_x=")<<size_x);
	fil.AddLine(wxString("size_y=")<<size_y);
	fil.AddLine(wxString("pos_x=")<<pos_x);
	fil.AddLine(wxString("pos_y=")<<pos_y);
	fil.AddLine(wxString("maximized=")<<maximized);	
	if (fixed_port) {
		fil.AddLine(wxString("debug_port=")<<debug_port);	
		fil.AddLine(wxString("comm_port=")<<comm_port);	
	}
	fil.AddLine(wxString("use_psterm=")<<(use_psterm?1:0));	
	fil.AddLine(wxString("use_dark_psterm=")<<(use_dark_psterm?1:0));	
	fil.AddLine(wxString("check_for_updates=")<<(check_for_updates?1:0));	
	fil.AddLine(wxString("fixed_port=")<<(fixed_port?1:0));	
	for (unsigned int i=0;i<last_files.GetCount();i++)
		fil.AddLine(wxString("history=")<<last_files[i]);
	fil.AddLine("");
	
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
			if (key=="version") { value.ToLong(&l); version=l; lang.Reset(version); }
			else if (key=="font_size") { value.ToLong(&l); font_size=l; }
			else if (key=="tabWidth") { value.ToLong(&l); tabw=l; }
			else if (key=="size_x") { value.ToLong(&l); size_x=l; }
			else if (key=="size_y") { value.ToLong(&l); size_y=l; }
			else if (key=="pos_x") { value.ToLong(&l); pos_x=l; }
			else if (key=="pos_y") { value.ToLong(&l); pos_y=l; }
			else if (key=="debug_port") { value.ToLong(&l); debug_port=l; }
			else if (key=="comm_port") { value.ToLong(&l); comm_port=l; }
			else if (key=="use_psterm") use_psterm=utils->IsTrue(value);
			else if (key=="use_dark_psterm") use_dark_psterm=utils->IsTrue(value);
			else if (key=="check_for_updates") check_for_updates=utils->IsTrue(value);
			else if (key=="fixed_port") fixed_port=utils->IsTrue(value);
			else if (key=="stepstep_tspeed") { value.ToLong(&l); stepstep_tspeed=l; }
			else if (key=="rt_syntax") rt_syntax=utils->IsTrue(value);
			else if (key=="smart_indent") smart_indent=utils->IsTrue(value);
			else if (key=="show_commands") show_commands=utils->IsTrue(value);
			else if (key=="show_vars") show_vars=utils->IsTrue(value);
			else if (key=="show_opers") show_opers=utils->IsTrue(value);
			else if (key=="show_debug_panel") show_debug_panel=utils->IsTrue(value);
//			else if (key=="show_toolbar") show_toolbar=utils->IsTrue(value);
			else if (key=="auto_quickhelp") auto_quickhelp=utils->IsTrue(value);
			else if (key=="calltip_helps") calltip_helps=utils->IsTrue(value);
			else if (key=="autocomp") autocomp=utils->IsTrue(value);
			else if (key=="highlight_blocks") highlight_blocks=utils->IsTrue(value);
			else if (key=="autoclose") autoclose=utils->IsTrue(value);
			else if (key=="psdraw_nocrop") psdraw_nocrop=utils->IsTrue(value);
			else if (key=="shape_colors") shape_colors=utils->IsTrue(value);
			else if (key=="colour_sintax") colour_sintax=utils->IsTrue(value);
			else if (key=="use_colors") use_colors=utils->IsTrue(value);
			else if (key=="animate_gui") animate_gui=utils->IsTrue(value);
			else if (key=="reorganize_for_debug") reorganize_for_debug=utils->IsTrue(value);
			else if (key=="images_path") images_path=value;
			else if (key=="help_dir") help_dir=value;
			else if (key=="proxy") proxy=value;
			else if (key=="profiles_dir") profiles_dir=value;
			else if (key=="profile") profile=value;
			else if (key=="examples_dir") examples_dir=value;
			else if (key=="last_dir") last_dir=value;
			else if (key=="temp_dir") temp_dir=value;
//			else if (key=="pseint_command") pseint_command=value;
//			else if (key=="psterm_command") psterm_command=value;
//			else if (key=="psexport_command") psexport_command=value;
//			else if (key=="psdrawe_command") psdrawe_command=value;
//			else if (key=="psdraw3_command") psdraw3_command=value;
			else if (key=="terminal") { tty_command=value; }
			else if (key=="history") last_files.Add(value);
			else lang.ProcessConfigLine(key.c_str(),value.c_str());
		}
	}
	fil.Close();
	LoadProfile(profile); 
	lang.Fix();
	if (version<20130805) use_psterm=true;
	if (version<20150627) shape_colors = true;
}

ConfigManager::~ConfigManager() {
	config=NULL;
}

wxString ConfigManager::LoadProfile(wxString pname) {
	profile=pname;
	if (lang.Load(DIR_PLUS_FILE(profiles_dir,profile))) {
		return lang.descripcion.c_str();
	} else {
		return _Z("Esta opcion le permite definir su propia configuracion. Utilice el boton \"Personalizar...\" para definirla.");
	}
}

int ConfigManager::GetCommPort () {
	if (fixed_port) return comm_port; else return comm_port+rand()%150+150;
}

int ConfigManager::GetDebugPort ( ) {
	if (fixed_port) return debug_port; else return debug_port+rand()%150;
}

wxString ConfigManager::GetTTYCommand ( ) {
	if (use_psterm) return psterm_command+(config->use_dark_psterm?" --darktheme":"");
	if (tty_command==_no_tty) { // tratar de detectar automaticamente un terminal adecuado
		if (utils->GetOutput("xterm -version").Len()) {
			tty_command = "xterm -T \"$name\" -e";
		} else if (utils->GetOutput("lxterminal -version").Len()) {
			tty_command = "lxterminal -T \"$name\" -e";
		} else if (utils->GetOutput("gnome-terminal --version").Len()) {
			tty_command = "gnome-terminal --hide-menubar --disable-factory -t \"$name\" -x";
		} else if (utils->GetOutput("konsole --version").Len()) {
			if (utils->GetOutput("konsole --version").Find("KDE: 3")==wxNOT_FOUND) {
				tty_command = "konsole -e";
				wxMessageBox(_Z("PSeInt requiere de una terminal para ejecutar los algoritmos. La unica terminal conocida encontrada en sus sistema es konosole, de KDE4. Esta genera problemas para ejecutar los algoritmos paso a paso y para obtener correctamente los errores en tiempo de ejecucion. Se recomienda instalar xterm."),_Z("Terminal de ejecucion"));
			} else {
				tty_command = "konsole --nomenubar --notoolbar -T \"$name\" -e";
			}
		} else {
			wxMessageBox(_Z("No se ha encontrado una terminal conocida.\nInstale xterm,konsole o gnome-terminal; o\nconfigure el parametro \"Comando del\nTerminal\" en el cuadro de Preferencias.\""),_Z("Terminal de ejecucion"));
		}
	}
	return tty_command;
}

