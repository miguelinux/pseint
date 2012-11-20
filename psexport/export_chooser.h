#ifndef EXPORT_CHOOSER_H
#define EXPORT_CHOOSER_H
#include <string>
#include "defines.h"
using namespace std;

enum PSE_LANG { PSE_CPP };

string translate_function(const string &name, const string &args);
void translate_main(t_programa &prog, t_algoritmo &alg);

extern PSE_LANG language;

#endif

