#include "export_chooser.h"
#include "export_cpp.h"

PSE_LANG language=PSE_CPP; // default language

string translate_function(const string &name, const string &args) {
//	switch (PSE_LANG) {
//	PSE_CPP: 
		return cpp_function(name,args);
//	}
}

void translate_main(t_programa & prog, t_algoritmo & alg) {
//	switch (PSE_LANG) {
//	PSE_CPP: 
		cpp_main(prog,alg); return;
//	}
}

