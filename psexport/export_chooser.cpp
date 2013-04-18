#include "export_chooser.h"
#include "export_cpp.h"

PSE_LANG language=PSE_CPP; // default language

string translate_function(const string &name, const string &args) {
//	switch (PSE_LANG) {
//	PSE_CPP: 
		return cpp_function(name,args);
//	}
}

void translate_programa(t_output &out, t_programa &prog) {
//	switch (PSE_LANG) {
//	PSE_CPP: 
		translate_cpp(out,prog); return;
//	}
}

