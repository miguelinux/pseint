#include <iostream>
#include "LangSettings.h"
#include <algorithm>
using namespace std;

LangSettings::aux_struct LangSettings::data[LS_COUNT];
bool LangSettings::init_done=false;

void LangSettings::init() {
	
	data[LS_FORCE_DEFINE_VARS		 ].Set("force_define_vars",			false,	
		"Obligar a definir los tipos de variables",
			"Si esta opción está deshabilitada no se puede utilizar una variable sin previamente definir su tipo. "
			"Para esto se debe utilizar la palabra clave DEFINIR. Ej: \"DEFINIR X COMO REAL\". Los tipos posibles "
			"son ENTERO, NUMERICO/REAL, LOGICO, CARACTER/TEXTO."
		);
	data[LS_FORCE_INIT_VARS			 ].Set("force_init_vars",			false,
		"No permitir utilizar variables o posiciones de arreglos sin inicializar",
			"Si esta opción está deshabilitada se puede utilizar variables sin incializar (que nunca fueron leidas o asignadas) "
			"en expresiones o para mostrar en pantalla. En este caso asumen el valor por defecto 0, \"\", o FALSO segun el tipo "
			"que corresponda. Si esta opción está seleccionada escribir o evaluar una expresión que involucra una variable sin "
			"inicializar genera un error en tiempo de ejecución."
		);
	data[LS_FORCE_SEMICOLON			 ].Set("force_semicolon",			false,
		"Controlar el uso de ; al final de sentencias secuenciales",
			"Si esta opción está activada obliga a colocar punto y coma (;) al final de las instrucciones secuenciales, como se hace "
			"en lenguajes como C++ o Pascal (util para acostumbrarse antes de dar el salto desde el pseudocódigo a un lenguaje real). "
			"si está desactivada el uso del punto y coma es opcional."
		);
	data[LS_BASE_ZERO_ARRAYS		 ].Set("base_zero_arrays",			false,
		"Utilizar indices en arreglos y cadenas en base 0",
			"Si esta opción esta activada, el primer elemento de un arreglo de N elementos será el 0 y el último el N-1, mientras "
			"que en caso contrario el primero será el 1 y el último el N"
		);
	data[LS_ALLOW_CONCATENATION		 ].Set("allow_concatenation",		true,
		"Permitir concatenar variables de texto con el operador +",
			"Si esta opción esta activada se puede concatenar el contenido de dos variables de tipo caracter con el operador +. "
			"Por ejemplo: NombreCompleto <- Nombre+\" \"+Apellido;"
		);
	data[LS_USE_NASSI_SHNEIDERMAN	 ].Set("use_nassi_shneiderman",	false,
		"Usar diagramas de Nassi-Shneiderman",
			"Con esta opción activada, el editor de diagramas utilizará el formato de Nassi-Shneiderman\n"
			"en lugar del formato clásico de diagrama de flujo."
		);
	data[LS_USE_ALTERNATIVE_IO_SHAPES].Set("use_alternative_io_shapes",	false,
		"Usar formas alternativas para Leer y Escribir en el diagrama",
			"Con esta opción activada, si se utiliza el diagrama de flujo clásico (no Nassi-Shneiderman), los bloques para las instrucciones"
			"Leer y Escribir serán diferentes entre sí, siguiendo una convención alternativa"
		);
	data[LS_ALLOW_DINAMYC_DIMENSIONS ].Set("allow_dinamyc_dimensions",	true,
		"Permitir utilizar variables para dimensionar arreglos",
			"Si esta opción está seleccionada se puede utilizar una variable o una expresión que involucre variables para dimensionar un "
			"arreglo (por ejemplo, se puede pedir al usuario del programa el tamaño leyendo un numero y utilizarlo luego para crear el "
			"arreglo). Este suele ser el caso de los lenguajes interpretados, mientras que los lenguajes compilados suelen exigir que el "
			"tamaño de los arreglos estáticos sea una constante (por ejemplo, C y C++)."
		);
	data[LS_OVERLOAD_EQUAL			 ].Set("overload_equal",			false,
		"Permitir asignar con el signo igual (=)",
			"Esta opcion habilita la asignación con el signo igual (Ej: x=0;). En muchos casos esta sintaxis de asignación no se"
			"permite, ya que en muchos lenguajes no se utilia el mismo operador para asignar y comparar, como sucede al activar"
			"esta opción. En cualquier caso, las otras dos sintaxis de asignación (con <- y con :=) siguen siendo válidas."
		);
	data[LS_COLOQUIAL_CONDITIONS	 ].Set("coloquial_conditions",		true,
		"Permitir condiciones en lenguaje coloquial",
			"Esta opcion permite expresar las condiciones en un lenguaje más coloquial con construcciones como \"X ES PAR\", "
			"\"X NO ES MULTIPLO DE 5\", \"X ES IGUAL A Y\", \"X ES ENTERO\", etc. Esta opción activa además el uso de palabras "
			"clave para reemplazar operadores."
		);
	data[LS_LAZY_SYNTAX				 ].Set("lazy_syntax",				true,
		"Utilizar sintaxis flexible",
			"Esta opcion habilita variaciones opcionales en la sintaxis de ciertas instrucciones y estructuras de control. Por ejemplo"
			"omitir la palabra HACER en un bucle MIENTRAS o PARA, utilizar la palabra DESDE para indicar el valor de inicio de un ciclo "
			"PARA, separar la expresiones/variables en una lectura/escritura con espacios en lugar de comas, escribir FinProceso como Fin "
			"Proceso, FinSi como Fin Si, etc.."
		);
	data[LS_WORD_OPERATORS			 ].Set("word_operators",			true,
		"Permitir las palabras Y, O, NO y MOD para los operadores &&, |, ~ y %",
			"Con esta opción habilitada PSeInt acepta las palabras clave Y, O, NO, y MOD como sinónimos de los operadores &&, |, ~ y % respectivamente. "
			"Notar que en este caso estas palabras serán palabras reservadas y no se podrán utilizar como nombres de variables."
		);
	data[LS_ENABLE_USER_FUNCTIONS	 ].Set("enable_user_functions",		true,
		"Permitir definir funciones/subprocesos",
			"Con esta opción activada se permite definir subprocesos/funciones en pseudocódigo para mediante la palabra clase SubProceso."
		);
	data[LS_ENABLE_STRING_FUNCTIONS	 ].Set("enable_string_functions",	true,
		"Habilitar funciones para el manejo de cadenas",
			"Esta opción habilita un conjunto de funciones predefinidas que sirven para operar sobre cadenas de "
			"caracteres. Las funciones son: Longitud, SubCadena, Mayusculas, Minusculas y Concatenar)."
		);
	data[LS_INTEGER_ONLY_SWITCH].Set("integer_only_switch",	false,
		"Limitar la estructura Según a variables de control numéricas",
			"Muchos lenguajes solo permiten utilizar números enteros para las expresiones de control de la estructura de "
			"selección múltiple (\"Según\" en PSeInt). Si habilita esta opción, PSeInt aplicará esta restricción. En caso "
			"contrario, podrá utilizar también variables de tipo caracter."
		);
	data[LS_DEDUCE_NEGATIVE_FOR_STEP].Set("deduce_negative_for_step",	true,
		"Permitir omitir el paso -1 en ciclos Para",
			"Con esta opción activa, si no se especifica el valor del \"paso\" en una estructura de tipo \"Para\", se utiliza +1 o -1 "
			"según corresponda. Se determina comparando los valores iniciales y finales, si el primero es mayor al segundo +1, o -1 "
			"en caso contrario. Si se desactiva esta opción, se utilizará siempre +1 como paso por defecto."
		);
#ifdef DEBUG
	for(int i=0;i<LS_COUNT;i++) { 
		if (!data[i].nombre) cerr<<"ERROR!!! campo no inicializado en LangSettings::data["<<i<<"]"<<endl;
	}
#endif
	init_done=true;
};


#ifdef FOR_WXPSEINT
#include <wx/string.h>
#include <wx/textfile.h>
#include "../wxPSeInt/string_conversions.h"
#include "../wxPSeInt/Logger.h"
#include "../wxPSeInt/version.h"


bool LangSettings::Load (const wxString &fname) {
	_LOG("LangSettings::Load "<<fname);
	return Load(std::string(_W2S(fname)));
}

bool LangSettings::Save (const wxString &fname) {
	return Save(std::string(_W2S(fname)));
}

void LangSettings::Log ( ) {
	_LOG("Profile: "<<VERSION<<" "<<GetAsSingleString().c_str());
}
#endif // FOR_WXPSEINT
#include <fstream>
bool LangSettings::Load(const std::string &fname) {
	ifstream fil(fname.c_str());
	if (!fil.is_open()) return false; 
	Reset(0); string str; // reset va despues de los "return false" para evitar resetear el perfil personalizado cuando se llama desde el ConfigManager
	while (getline(fil,str)) ProcessConfigLine(str);
	Fix();
	return true;
}

static void replace(string  &s, string from, string to) {
	size_t p = s.find(from);
	while(p!=string::npos) {
		s.replace(p,from.size(),to);
		p = s.find(from,p+to.size());
	}
}

bool LangSettings::Save (const string &fname) {
	ofstream fil(fname.c_str(),ios::trunc);
	if (!fil.is_open()) return false;
	string tmp = descripcion.c_str(); 
	replace(tmp,"\r",""); replace(tmp,"\n","\ndesc=");
	fil << "desc=" << tmp << endl;
	fil << "version=" <<LS_VERSION << endl;
	for(int i=0;i<LS_COUNT;i++) 
		fil << GetConfigLine(i);
	fil.close();
	return true;
}

std::string LangSettings::GetAsSingleString() {
	std::string retval(LS_COUNT,'?');
	for(int i=0;i<LS_COUNT;i++) retval[i]=settings[i]?'1':'0';
	return retval;
}


void LangSettings::Fix ( ) {
	if (version<20150304) { // antes LS_INTEGER_ONLY_SWITCH y LS_DEDUCE_NEGATIVE_FOR_STEP eran parte de LS_LAZY_SYNTAX
		settings[LS_INTEGER_ONLY_SWITCH]=!settings[LS_LAZY_SYNTAX];
		settings[LS_DEDUCE_NEGATIVE_FOR_STEP]=settings[LS_LAZY_SYNTAX];
	}
	if (settings[LS_COLOQUIAL_CONDITIONS]) // no se puede usar LS_COLOQUIAL_CONDITIONS sin LS_WORD_OPERATORS
		settings[LS_WORD_OPERATORS]=true;
	version=LS_VERSION; // colocar version nueva, para que el fix ya no actualice el perfil en la próxima carga
}

bool LangSettings::SetFromSingleString (const std::string & str) {
	if (str.size()!=LS_COUNT) return false;
	for(int i=0;i<LS_COUNT;i++) settings[i] = str[i]=='1';
	return true;
}

