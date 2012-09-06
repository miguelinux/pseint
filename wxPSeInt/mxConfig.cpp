#include "mxConfig.h"
#include <wx/sizer.h>
#include "ConfigManager.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "mxArt.h"

BEGIN_EVENT_TABLE(mxConfig,wxDialog)
	EVT_BUTTON(wxID_OK,mxConfig::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxConfig::OnCancelButton)
	EVT_CLOSE(mxConfig::OnClose)
END_EVENT_TABLE()

mxConfig::mxConfig(wxWindow *parent):wxDialog(parent,wxID_ANY,_T("Opciones del Lenguaje"),wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *opts_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	chk_force_init_vars = utils->AddCheckBox(opts_sizer,this,_T("No permitir utilizar variables o posiciones de arreglos sin inicializar"),config->lang.force_init_vars);
	chk_force_init_vars->SetToolTip(
		"Si esta opción está deshabilitada se puede utilizar variables sin incializar (que nunca fueron leidas o asignadas) "
		"en expresiones o para mostrar en pantalla. En este caso asumen el valor por defecto 0, \"\", o FALSO segun el tipo "
		"que corresponda. Si esta opción está seleccionada escribir o evaluar una expresión que involucra una variable sin "
		"inicializar genera un error en tiempo de ejecución.");
	chk_force_define_vars = utils->AddCheckBox(opts_sizer,this,_T("Obligar a definir los tipos de variables"),config->lang.force_define_vars);
	chk_force_define_vars->SetToolTip(
		"Si esta opción está deshabilitada no se puede utilizar una variable sin previamente definir su tipo. Para esto se debe "
		"utilizar la palabra clave DEFINIR. Ej: \"DEFINIR X COMO REAL\". Los tipos posibles son ENTERO, NUMERICO/REAL, LOGICO, CARACTER/TEXTO.");
	chk_force_dot_and_comma = utils->AddCheckBox(opts_sizer,this,_T("Controlar el uso de ; al final de sentencias secuenciales"),config->lang.force_dot_and_comma);
	chk_force_dot_and_comma->SetToolTip(
		"Si esta opción está activada obliga a colocar punto y coma (;) al final de las instrucciones secuenciales, como se hace "
		"en lenguajes como C++ o Pascal (util para acostumbrarse antes de dar el salto desde el pseudocódigo a un lenguaje real). "
		"si está desactivada el uso del punto y coma es opcional.");
	chk_allow_concatenation = utils->AddCheckBox(opts_sizer,this,_T("Permitir concatenar variables de texto con el operador +"),config->lang.allow_concatenation);
	chk_allow_concatenation->SetToolTip(
		"Si esta opción esta activada se puede concatenar el contenido de dos variables de tipo caracter con el operador +. "
		"Por ejemplo: NombreCompleto <- Nombre+\" \"+Apellido;");
	chk_word_operators = utils->AddCheckBox(opts_sizer,this,_T("Permitir las palabras Y, O, NO y MOD para los operadores &, |, ~ y %"),config->lang.word_operators);
	chk_word_operators->SetToolTip(
		"Con esta opción habilitada PSeInt acepta las palabras clave Y, O, NO, y MOD como sinónimos de los operadores &, |, ~ y % respectivamente. "
		"Notar que en este caso estas palabras serán palabras reservadas y no se podrán utilizar como nombres de variables.");
	chk_base_zero_arrays = utils->AddCheckBox(opts_sizer,this,_T("Utilizar indices en arreglos y cadenas en base 0"),config->lang.base_zero_arrays);
	chk_base_zero_arrays->SetToolTip(
		"Si esta opción esta activada, el primer elemento de un arreglo de N elementos será el 0 y el último el N-1, mientras "
		"que en caso contrario el primero será el 1 y el último el N");
	chk_allow_dinamyc_dimensions = utils->AddCheckBox(opts_sizer,this,_T("Permitir utilizar variables para dimensionar arreglos"),config->lang.allow_dinamyc_dimensions);
	chk_allow_dinamyc_dimensions->SetToolTip(
		"Si esta opción está seleccionada se puede utilizar una variable o una expresión que involucre variables para dimensionar un "
		"arreglo (por ejemplo, se puede pedir al usuario del programa el tamaño leyendo un numero y utilizarlo luego para crear el "
		"arreglo). Este suele ser el caso de los lenguajes interpretados, mientras que los lenguajes compilados suelen exigir que el "
		"tamaño de los arreglos estáticos sea una constante (por ejemplo, C y C++).");
	chk_overload_equal = utils->AddCheckBox(opts_sizer,this,_T("Permitir asignar con el signo igual (=)"),config->lang.overload_equal);
	chk_overload_equal->SetToolTip(
		"Esta opcion habilita la asignación con el signo igual (Ej: x=0;). En muchos casos esta sintaxis de asignación no se"
		"permite, ya que en muchos lenguajes no se utilia el mismo operador para asignar y comparar, como sucede al activar"
		"esta opción. En cualquier caso, las otras dos sintaxis de asignación (con <- y con :=) siguen siendo válidas." );
	
	chk_enable_string_functions = utils->AddCheckBox(opts_sizer,this,_T("Utilizar funciones para el manejo de cadenas (Longitud, Subcadena, etc)"),config->lang.enable_string_functions);
	chk_enable_string_functions ->SetToolTip(
		"Esta opción habilita un conjunto de funciones predefinidas que sirven para operar sobre cadenas de "
		"caracteres. Las funciones son: Longitud, SubCadena, Mayusculas, Minusculas y Concatenar).");
	chk_enable_user_functions = utils->AddCheckBox(opts_sizer,this,_T("Permitir definir funciones/subprocesos"),config->lang.enable_user_functions);
	chk_enable_user_functions->SetToolTip(
		"Con esta opción activada se permite definir subprocesos/funciones en pseudocódigo para mediante la palabra clase SubProceso." );
	
	chk_lazy_syntax = utils->AddCheckBox(opts_sizer,this,_T("Utilizar sintaxis flexible"),config->lang.lazy_syntax);
	chk_lazy_syntax->SetToolTip(
		"Esta opcion habilita variaciones opcionales en la sintaxis: omitir la palabra HACER en un bucle MIENTRAS o PARA, "
		"utilizar la palabra DESDE para indicar el valor de inicio de un ciclo PARA, separar la expresiones/variables en "
		"una lectura/escritura con espacios en lugar de comas, agregar la palabra OPCION, CASO o SI ES antes de un caso en "
		"un SEGUN, separar los valores para un mismo caso en un segun con la palabra O, etc." );
	chk_coloquial_conditions = utils->AddCheckBox(opts_sizer,this,_T("Permitir condiciones en lenguaje coloquial"),config->lang.coloquial_conditions);
	chk_coloquial_conditions->SetToolTip(
		"Esta opcion permite expresar las condiciones en un lenguaje más coloquial con construcciones como \"X ES PAR\", "
		"\"X NO ES MULTIPLO DE 5\", \"X ES IGUAL A Y\", \"X ES ENTERO\", etc. Esta opción activa además el uso de palabras "
		"clave para reemplazar operadores." );
	chk_use_nassi_schneiderman= utils->AddCheckBox(opts_sizer,this,_T("Usar diagramas de Nassi-Schneiderman"),config->lang.use_nassi_schneiderman);
	chk_use_nassi_schneiderman->SetToolTip(
		"Con esta opción activada, el editor de diagramas utilizará el formato de Nassi-Schneiderman\n"
		"en lugar del formato clásico de diagrama de flujo.");
	
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, _T("Aceptar"));
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, _T("Cancelar"));
	button_sizer->Add(cancel_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(ok_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	
	sizer->Add(opts_sizer,wxSizerFlags().Right());
	sizer->Add(button_sizer,wxSizerFlags().Right());
	
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	SetSizerAndFit(sizer);
//	CentreOnParent();
	ShowModal();
}

mxConfig::~mxConfig() {
	

}
void mxConfig::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxConfig::OnOkButton(wxCommandEvent &evt) {
	config->lang.force_define_vars=chk_force_define_vars->GetValue();
	config->lang.force_init_vars=chk_force_init_vars->GetValue();
	config->lang.force_dot_and_comma=chk_force_dot_and_comma->GetValue();
	config->lang.base_zero_arrays=chk_base_zero_arrays->GetValue();
	config->lang.allow_concatenation=chk_allow_concatenation->GetValue();
	config->lang.overload_equal=chk_overload_equal->GetValue();
	config->lang.coloquial_conditions=chk_coloquial_conditions->GetValue();
	config->lang.lazy_syntax=chk_lazy_syntax->GetValue();
	config->lang.word_operators=chk_word_operators->GetValue()||chk_coloquial_conditions->GetValue();
	config->lang.allow_dinamyc_dimensions=chk_allow_dinamyc_dimensions->GetValue();
	config->lang.use_nassi_schneiderman=chk_use_nassi_schneiderman->GetValue();
	config->lang.enable_string_functions=chk_enable_string_functions->GetValue();
	config->lang.enable_user_functions=chk_enable_user_functions->GetValue();
	Close();
}

void mxConfig::OnCancelButton(wxCommandEvent &evt) {
	Close();
}


