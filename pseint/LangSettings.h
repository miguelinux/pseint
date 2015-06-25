#ifndef LANGSETTINGS_H
#define LANGSETTINGS_H

#include <string>
#ifdef FOR_WXPSEINT
#	include <wx/string.h>
#endif

#define LS_VERSION 20150304
#include <cstdlib>

enum LS_INIT_ENUM {
	LS_INIT,
	LS_DO_NOT_INIT
};

enum LS_ENUM {
	LS_FORCE_INIT_VARS,
	LS_FORCE_DEFINE_VARS,
	LS_FORCE_SEMICOLON,
	LS_ALLOW_CONCATENATION,
	LS_ENABLE_STRING_FUNCTIONS,
	LS_WORD_OPERATORS,
	LS_BASE_ZERO_ARRAYS,
	LS_ALLOW_DINAMYC_DIMENSIONS,
	LS_OVERLOAD_EQUAL,
	LS_ENABLE_USER_FUNCTIONS,
	LS_LAZY_SYNTAX,
	LS_COLOQUIAL_CONDITIONS,
	LS_INTEGER_ONLY_SWITCH,
	LS_DEDUCE_NEGATIVE_FOR_STEP,
	LS_USE_NASSI_SHNEIDERMAN,
	LS_USE_ALTERNATIVE_IO_SHAPES,
	LS_COUNT 
};

struct LangSettings {
	
	struct aux_struct { 
		const char *nombre; bool default_value; 
#ifdef FOR_WXPSEINT
		wxString user_desc, long_desc;
#endif
		aux_struct(){ nombre=NULL; default_value=false; }
		void Set(const char *nom, bool val, const char *udesc, const char *ldesc) { 
			nombre=nom; default_value=val;
#ifdef FOR_WXPSEINT
			user_desc=udesc; long_desc=ldesc;
#endif
		}
	};
	static aux_struct data[LS_COUNT];
	static bool init_done;
	static void init();
	
	int version;
	std::string descripcion;
	bool settings[LS_COUNT];
	
	bool &operator[](LS_ENUM x) { return settings[x]; }
	bool operator!=(const LangSettings &o) const {
		for(int i=0;i<LS_COUNT;i++)
			if (settings[i]!=o.settings[i]) return true;
		return false;
	}
	void Reset(int init_version = LS_VERSION) {
		if (!init_done) init();
		version=init_version;
		descripcion.clear();
		for(int i=0;i<LS_COUNT;i++) 
			settings[i]=data[i].default_value;
	}
	LangSettings(LS_INIT_ENUM init_mode) { 
		if (init_mode==LS_DO_NOT_INIT) return; 
		Reset();
	}
	std::string GetAsSingleString();
	bool SetFromSingleString(const std::string &str);
	
#ifdef FOR_WXPSEINT
	bool Load(const wxString &fname);
	bool Save(const wxString &fname);
	void Log();
#else
	bool Load(const std::string &fname);
#endif

	bool IsTrue(const std::string &value) const {
		return (value.size() && (value[0]=='1' || value[0]=='V' || value[0]=='v' || value[0]=='s' || value[0]=='S' || value[0]=='T' || value[0]=='t'));
	}
	
	std::string GetConfigLine(int i) {
		std::string ret(data[i].nombre); ret+="="; ret+=settings[i]?"1":"0"; return ret;
	}
	bool ProcessConfigLine(const std::string &key, const std::string &value) {
		if (key=="desc") { descripcion+=value+"\n"; return true; }
		else if (key=="profile") { Load(value.c_str()); return true; } // el .c_str es por si usa el Load(wxString)
		else if (key=="binprofile") { SetFromSingleString(value); return true; }
		else if (key=="version") { version=atoi(value.c_str()); return true; }
		else if (key=="use_nassi_schneiderman") return ProcessConfigLine("use_nassi_shneiderman",value); // estaba mal deletreado... sChneiderman
		else {
			for(int i=0;i<LS_COUNT;i++) { 
				if (key==data[i].nombre) {
					settings[i]=IsTrue(value);
					return true;
				}
			}
		}
		return false;
	}
	bool ProcessConfigLine(std::string line) {
		if (line.size()==0 || line[0]=='#') return false;
		size_t pos=line.find('='); if (pos==std::string::npos) return false;
		return ProcessConfigLine(line.substr(0,pos),line.substr(pos+1));
	}
	void Fix();
};

#endif
