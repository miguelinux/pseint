#include "global.h"

queue <string> predef_input; // entrada predefinida para los tests
//vector <string> programa; // programa con sintaxis reorganizada para ejecutar
//vector <int> prog_lines;  // correspondencia con el código original (por numeros de lineas)
ofstream ExeInfo; // Archivo de Inforamacion de Ejecucion para el entorno
bool ExeInfoOn=false;
bool wait_key;
int SynErrores=0; // Contador para errores de Sintaxis
bool for_test=false;
bool raw_errors=false;
bool allow_undef_vars=true;
bool force_semicolon=false;
bool word_operators=true;
bool colored_output=false;
bool allow_dinamyc_dimensions=false;
bool allow_concatenation=true;
bool enable_string_functions=true;
bool base_zero_arrays=false;
bool lazy_syntax=true;
bool overload_equal=false;
bool coloquial_conditions=false;
bool force_var_definition=false;
bool noinput=false;
bool fix_win_charset=false;
bool enable_user_functions=true;
bool remembar_case=false;
bool for_pseint_terminal=false;
bool with_io_references=false;
