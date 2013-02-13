#include "mxConfig.h"
#include <wx/sizer.h>
#include "ConfigManager.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "mxArt.h"
#include <wx/filedlg.h>
#include <wx/textdlg.h>

BEGIN_EVENT_TABLE(mxConfig,wxDialog)
	EVT_BUTTON(wxID_OK,mxConfig::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxConfig::OnCancelButton)
	EVT_BUTTON(wxID_OPEN,mxConfig::OnOpenButton)
	EVT_BUTTON(wxID_SAVE,mxConfig::OnSaveButton)
	EVT_CLOSE(mxConfig::OnClose)
END_EVENT_TABLE()

mxConfig::mxConfig(wxWindow *parent):wxDialog(parent,wxID_ANY,_T("Opciones del Lenguaje"),wxDefaultPosition,wxDefaultSize) {
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *opts_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	chk_force_init_vars = utils->AddCheckBox(opts_sizer,this,_T("No permitir utilizar variables o posiciones de arreglos sin inicializar"),true);
	chk_force_init_vars->SetToolTip(utils->FixTooltip(
		"Si esta opción está deshabilitada se puede utilizar variables sin incializar (que nunca fueron leidas o asignadas) "
		"en expresiones o para mostrar en pantalla. En este caso asumen el valor por defecto 0, \"\", o FALSO segun el tipo "
		"que corresponda. Si esta opción está seleccionada escribir o evaluar una expresión que involucra una variable sin "
		"inicializar genera un error en tiempo de ejecución."));
	chk_force_define_vars = utils->AddCheckBox(opts_sizer,this,_T("Obligar a definir los tipos de variables"),true);
	chk_force_define_vars->SetToolTip(utils->FixTooltip(
		"Si esta opción está deshabilitada no se puede utilizar una variable sin previamente definir su tipo. Para esto se debe "
		"utilizar la palabra clave DEFINIR. Ej: \"DEFINIR X COMO REAL\". Los tipos posibles son ENTERO, NUMERICO/REAL, LOGICO, CARACTER/TEXTO."));
	chk_force_dot_and_comma = utils->AddCheckBox(opts_sizer,this,_T("Controlar el uso de ; al final de sentencias secuenciales"),true);
	chk_force_dot_and_comma->SetToolTip(utils->FixTooltip(
		"Si esta opción está activada obliga a colocar punto y coma (;) al final de las instrucciones secuenciales, como se hace "
		"en lenguajes como C++ o Pascal (util para acostumbrarse antes de dar el salto desde el pseudocódigo a un lenguaje real). "
		"si está desactivada el uso del punto y coma es opcional."));
	chk_allow_concatenation = utils->AddCheckBox(opts_sizer,this,_T("Permitir concatenar variables de texto con el operador +"),true);
	chk_allow_concatenation->SetToolTip(utils->FixTooltip(
		"Si esta opción esta activada se puede concatenar el contenido de dos variables de tipo caracter con el operador +. "
		"Por ejemplo: NombreCompleto <- Nombre+\" \"+Apellido;"));
	chk_enable_string_functions = utils->AddCheckBox(opts_sizer,this,_T("Habilitar funciones para el manejo de cadenas"),true);
	chk_enable_string_functions ->SetToolTip(utils->FixTooltip(
		"Esta opción habilita un conjunto de funciones predefinidas que sirven para operar sobre cadenas de "
		"caracteres. Las funciones son: Longitud, SubCadena, Mayusculas, Minusculas y Concatenar)."));
		
	chk_word_operators = utils->AddCheckBox(opts_sizer,this,_T("Permitir las palabras Y, O, NO y MOD para los operadores &&, |, ~ y %"),true);
	chk_word_operators->SetToolTip(utils->FixTooltip(
		"Con esta opción habilitada PSeInt acepta las palabras clave Y, O, NO, y MOD como sinónimos de los operadores &&, |, ~ y % respectivamente. "
		"Notar que en este caso estas palabras serán palabras reservadas y no se podrán utilizar como nombres de variables."));
	chk_base_zero_arrays = utils->AddCheckBox(opts_sizer,this,_T("Utilizar indices en arreglos y cadenas en base 0"),true);
	chk_base_zero_arrays->SetToolTip(utils->FixTooltip(
		"Si esta opción esta activada, el primer elemento de un arreglo de N elementos será el 0 y el último el N-1, mientras "
		"que en caso contrario el primero será el 1 y el último el N"));
	chk_allow_dinamyc_dimensions = utils->AddCheckBox(opts_sizer,this,_T("Permitir utilizar variables para dimensionar arreglos"),true);
	chk_allow_dinamyc_dimensions->SetToolTip(utils->FixTooltip(
		"Si esta opción está seleccionada se puede utilizar una variable o una expresión que involucre variables para dimensionar un "
		"arreglo (por ejemplo, se puede pedir al usuario del programa el tamaño leyendo un numero y utilizarlo luego para crear el "
		"arreglo). Este suele ser el caso de los lenguajes interpretados, mientras que los lenguajes compilados suelen exigir que el "
		"tamaño de los arreglos estáticos sea una constante (por ejemplo, C y C++)."));
	chk_overload_equal = utils->AddCheckBox(opts_sizer,this,_T("Permitir asignar con el signo igual (=)"),true);
	chk_overload_equal->SetToolTip(utils->FixTooltip(
		"Esta opcion habilita la asignación con el signo igual (Ej: x=0;). En muchos casos esta sintaxis de asignación no se"
		"permite, ya que en muchos lenguajes no se utilia el mismo operador para asignar y comparar, como sucede al activar"
		"esta opción. En cualquier caso, las otras dos sintaxis de asignación (con <- y con :=) siguen siendo válidas." ));
	
	chk_enable_user_functions = utils->AddCheckBox(opts_sizer,this,_T("Permitir definir funciones/subprocesos"),true);
	chk_enable_user_functions->SetToolTip(utils->FixTooltip(
		"Con esta opción activada se permite definir subprocesos/funciones en pseudocódigo para mediante la palabra clase SubProceso." ));
	
	chk_lazy_syntax = utils->AddCheckBox(opts_sizer,this,_T("Utilizar sintaxis flexible"),true);
	chk_lazy_syntax->SetToolTip(utils->FixTooltip(
		"Esta opcion habilita variaciones opcionales en la sintaxis: omitir la palabra HACER en un bucle MIENTRAS o PARA, "
		"utilizar la palabra DESDE para indicar el valor de inicio de un ciclo PARA, separar la expresiones/variables en "
		"una lectura/escritura con espacios en lugar de comas, agregar la palabra OPCION, CASO o SI ES antes de un caso en "
		"un SEGUN, separar los valores para un mismo caso en un segun con la palabra O, etc." ));
	chk_coloquial_conditions = utils->AddCheckBox(opts_sizer,this,_T("Permitir condiciones en lenguaje coloquial"),true);
	chk_coloquial_conditions->SetToolTip(utils->FixTooltip(
		"Esta opcion permite expresar las condiciones en un lenguaje más coloquial con construcciones como \"X ES PAR\", "
		"\"X NO ES MULTIPLO DE 5\", \"X ES IGUAL A Y\", \"X ES ENTERO\", etc. Esta opción activa además el uso de palabras "
		"clave para reemplazar operadores." ));
	chk_use_nassi_schneiderman= utils->AddCheckBox(opts_sizer,this,_T("Usar diagramas de Nassi-Schneiderman"),true);
	chk_use_nassi_schneiderman->SetToolTip(utils->FixTooltip(
		"Con esta opción activada, el editor de diagramas utilizará el formato de Nassi-Schneiderman\n"
		"en lugar del formato clásico de diagrama de flujo."));
	
	wxButton *load_button = new wxBitmapButton (this, wxID_OPEN, wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_abrir.png")),wxBITMAP_TYPE_PNG));
	wxButton *save_button = new wxBitmapButton (this, wxID_SAVE, wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_guardar.png")),wxBITMAP_TYPE_PNG));
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, _T("Aceptar"));
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, _T("Cancelar"));
	button_sizer->Add(load_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(save_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->AddStretchSpacer(1);
	button_sizer->Add(cancel_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	button_sizer->Add(ok_button,wxSizerFlags().Border(wxALL,5).Proportion(0).Expand());
	
	sizer->Add(opts_sizer,wxSizerFlags().Proportion(1).Expand());
	sizer->Add(button_sizer,wxSizerFlags().Proportion(0).Expand());
	
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	ReadFromStruct(config->lang);
	
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
	CopyToStruct(config->lang);
	Close();
}

void mxConfig::OnCancelButton(wxCommandEvent &evt) {
	Close();
}


void mxConfig::OnOpenButton (wxCommandEvent & evt) {
	wxFileDialog dlg (this, _T("Cargar perfil desde archivo"), config->last_dir, _T(" "), _T("Cualquier Archivo (*)|*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST );
	if (dlg.ShowModal() == wxID_OK) {
		config->last_dir=dlg.GetDirectory();
		LangSettings l;
		l.Load(dlg.GetPath());
		ReadFromStruct(l);
	}
}

void mxConfig::OnSaveButton (wxCommandEvent & evt) {
	wxFileDialog dlg (this, _T("Guardar perfil en archivo"), config->last_dir, _T(" "), _T("Cualquier Archivo (*)|*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK) {
		config->last_dir=dlg.GetDirectory();
		LangSettings l;
		l.desc=wxGetTextFromUser(_T("Ingrese una descripción breve del perfil."),_T("Guardar Perfil"),_T(""),this);
		CopyToStruct(l);
		l.Save(dlg.GetPath());
	}
}

void mxConfig::ReadFromStruct (LangSettings l) {
	chk_force_define_vars->SetValue(l.force_define_vars);
	chk_force_init_vars->SetValue(l.force_init_vars);
	chk_force_dot_and_comma->SetValue(l.force_dot_and_comma);
	chk_base_zero_arrays->SetValue(l.base_zero_arrays);
	chk_allow_concatenation->SetValue(l.allow_concatenation);
	chk_overload_equal->SetValue(l.overload_equal);
	chk_coloquial_conditions->SetValue(l.coloquial_conditions);
	chk_lazy_syntax->SetValue(l.lazy_syntax);
	chk_word_operators->SetValue(l.word_operators);
	chk_allow_dinamyc_dimensions->SetValue(l.allow_dinamyc_dimensions);
	chk_use_nassi_schneiderman->SetValue(l.use_nassi_schneiderman);
	chk_enable_string_functions->SetValue(l.enable_string_functions);
	chk_enable_user_functions->SetValue(l.enable_user_functions);	
}

void mxConfig::CopyToStruct (LangSettings & l) {
	l.force_define_vars=chk_force_define_vars->GetValue();
	l.force_init_vars=chk_force_init_vars->GetValue();
	l.force_dot_and_comma=chk_force_dot_and_comma->GetValue();
	l.base_zero_arrays=chk_base_zero_arrays->GetValue();
	l.allow_concatenation=chk_allow_concatenation->GetValue();
	l.overload_equal=chk_overload_equal->GetValue();
	l.coloquial_conditions=chk_coloquial_conditions->GetValue();
	l.lazy_syntax=chk_lazy_syntax->GetValue();
	l.word_operators=chk_word_operators->GetValue()||chk_coloquial_conditions->GetValue();
	l.allow_dinamyc_dimensions=chk_allow_dinamyc_dimensions->GetValue();
	l.use_nassi_schneiderman=chk_use_nassi_schneiderman->GetValue();
	l.enable_string_functions=chk_enable_string_functions->GetValue();
	l.enable_user_functions=chk_enable_user_functions->GetValue();	
}

