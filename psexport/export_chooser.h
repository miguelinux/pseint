#ifndef EXPORT_CHOOSER_H
#define EXPORT_CHOOSER_H
#include <string>
#include "defines.h"
using namespace std;

enum PSE_LANG { PSE_CPP=0, PSE_VB, PSE_PHP, PSE_COUNT };

extern PSE_LANG language;
extern string lang_names[PSE_COUNT];
PSE_LANG get_lang(string name);

string translate_function(const string &name, const string &args);
void translate_programa(t_output &out, t_programa &prog);


#endif

