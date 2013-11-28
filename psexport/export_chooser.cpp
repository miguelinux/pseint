#include "export_chooser.h"
#include "export_cpp.h"
#include "export_php.h"
#include "export_vb.h"


PSE_LANG language=PSE_CPP; // default language
string lang_names[PSE_COUNT];

PSE_LANG get_lang(string name) {
	
	lang_names[PSE_CPP]="cpp";
	lang_names[PSE_PHP]="php";
	lang_names[PSE_VB ]="vb";
	
	for(int i=0;i<PSE_COUNT;i++) { 
		if (lang_names[i]==name) return (PSE_LANG)i;
	}
	return PSE_COUNT;
}
string translate_function(const string &name, const string &args) {
	switch (language) {
	case PSE_CPP: return cpp_function(name,args);
	case PSE_PHP: return php_function(name,args);
	case PSE_VB:  return vb_function (name,args);
	default: return name;
	}
}

void translate_programa(t_output &out, t_programa &prog) {
	switch (language) {
	case PSE_CPP: translate_cpp(out,prog); return;
	case PSE_PHP: translate_php(out,prog); return;
	case PSE_VB:  translate_vb (out,prog); return;
	default: return;
	}
}

