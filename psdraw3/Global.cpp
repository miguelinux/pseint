// comunicacion con wxPseInt
#include <cmath>
#include "Global.h"

//LangSettings lang; // no definir aca, ver comentario en el global.cpp de pseint

bool edit_on=true; // indica si se puede editar el diagrama
bool debugging=false; // indica si el programa se esta ejecutando paso a paso (en ese caso no se puede modificar edit_on)
string fname; // archivo que recibe como argumento
string pname; // nombre original del proceso

bool modified=false; // para saber si hay que preguntar antes de salir sin guardar

// estado para pasar entre eventos para la edicion
Entity *edit=NULL; // entidad seleccionado para editar su texto
Entity *mouse=NULL; // entidad seleccionado por el mouse
Entity *start=NULL; // entidad donde comienza el algoritmo
int m_x0=0; // click del mouse, para referencia en el motion, se setea en el mouse
int m_y0=0; // click del mouse, para referencia en el motion, se setea en el mouse
int blink=0; // cuando se esta editando texto, indica si se muestra o no el cursor, para que parpadee
int cur_x=0, cur_y=0; // ubicacion del raton (durante el passive_motion_cb esta sin aplicar zoom (coord del viewport), durante el motion y click, tiene la correccion del zoom (coord del dibujo))


// zoom y panning
bool panning=false; // indica si se esta moviendo el dibujo, para el motion
bool selecting_zoom=false; // para hacer zoom en un area marcada, con el boton del medio, m_x0 y m_y0 guardan la primer esquina
bool selecting_entities=false; // para selecciones múltiples (rectangulares, shift+drag derecho)
float d_dx=0; // "paning" del dibujo
float d_dy=0; // "paning" del dibujo
float d_zoom=0.1; // zoom del dibujo
float zoom=0.1; // zoom final
int line_width_flechas=2, line_width_bordes=1;

// auxiliares varios
Entity *entity_to_del=NULL; // para un delayed delete (cuando suelta uno que sale del shapebar y no queda en ningun lado)
Entity *all_any=NULL;

// constantes con tamaños predefinidos (se inicializan en GlobalInit porque dependen de los DPI)
bool big_icons = false;
int win_h/*=400*/,win_w/*=600*/; // tamaño de la ventana
wxString imgs_path/* = "imgs/flow/24"*/;

// colores
float color_border[ET_COUNT+1][3]={0}; // borde de la forma de una entidad
const float color_comment[3]={.4,.4,.4}; // borde de la forma de una entidad
/*const */float color_label_high[6][3] = {
	{0,0,0}, // identificadores
	{.60,.40,.15}, // numeros
	{.7,0,0}, // cadenas
	{0,0,.6}, // palabras claves
	{.4,.4,.4}, // comentarios
	{.3,.3,.3} // operadores
};
float color_arrow[3]={.8,0,0}; // flechas que guian el flujo y unen entidades
const float color_selection[3]={0,.4,0}; // texto o borde de forma seleccionada
float color_shape[ET_COUNT+1][3]; // fondo de la forma
const float color_back[3]={.97,1,.97}; // fondo de la pantalla
const float color_back_alt[3]={1,1,1}; // fondo de pantalla alternativo para usar con los bloques de colores
const float color_ghost[3]={.7,.7,.7}; // borde fantasma cuando se arrastra una forma
const float color_menu[3]={.4,.4,.4}; // texto de los menues
/*const */float color_menu_back[3]={.92,.97,.97}; // fondo de los menues
/*const */float color_menu_sel[3]={.75,.95,.95}; // fondo de elemento de menu seleccionado
const float color_error[3]={.7,0,0}; // mensajes de error de sintaxis

vector<Entity*> procesos; // para almacenar el proceso principal y los subprocesos
map<string,LineInfo> code2draw; // para asociar las lineas de código al diagrama de flujo
Entity *debug_current=NULL; // la entidad que se esta ejecutando actualmente en el paso a paso

bool loading=false; // indica si se esta cargando un nuevo algoritmo desde un archivo para desactivar el evento de modificacion

void GlobalInitPre( ) {
	
	for(int i=0;i<ET_COUNT;i++)
		Entity::max_label_len[i]=15;
	Entity::max_label_len[ET_COMENTARIO]=25;
	Entity::max_label_len[ET_LEER]=25;
	Entity::max_label_len[ET_PROCESO]=25;
	Entity::max_label_len[ET_ESCRIBIR]=25;
	Entity::max_label_len[ET_ASIGNAR]=25;
	Entity::max_label_len[ET_SI]=15;
	Entity::max_label_len[ET_SEGUN]=15;
	Entity::max_label_len[ET_OPCION]=10;
	Entity::max_label_len[ET_PARA]=15;
	Entity::max_label_len[ET_MIENTRAS]=15;
	Entity::max_label_len[ET_REPETIR]=15;
	Entity::max_label_len[ET_AUX_PARA]=10;
	
	for(int i=0;i<ET_COUNT;i++) { 
		color_shape[i][0] = 1;
		color_shape[i][1] = .95;
		color_shape[i][2] = .8;
	}
	color_shape[ET_COUNT][0] = .95;
	color_shape[ET_COUNT][1] = .95;
	color_shape[ET_COUNT][2] = .95;
	color_shape[ET_LEER][0] = 1;
	color_shape[ET_LEER][1] = .9;
	color_shape[ET_LEER][2] = .95;
	color_shape[ET_ESCRIBIR][0] = .9;
	color_shape[ET_ESCRIBIR][1] = 1;
	color_shape[ET_ESCRIBIR][2] = .9;
	color_shape[ET_ASIGNAR][0] = 1;
	color_shape[ET_ASIGNAR][1] = 1;
	color_shape[ET_ASIGNAR][2] = .85;
	color_shape[ET_SI][0] = color_shape[ET_SEGUN][0] = color_shape[ET_OPCION][0] = .85;
	color_shape[ET_SI][1] = color_shape[ET_SEGUN][1] = color_shape[ET_OPCION][1] = .9;
	color_shape[ET_SI][2] = color_shape[ET_SEGUN][2] = color_shape[ET_OPCION][2] = 1;
	color_shape[ET_PARA][0] = color_shape[ET_MIENTRAS][0] = .95;
	color_shape[ET_PARA][1] = color_shape[ET_MIENTRAS][1] = .8;
	color_shape[ET_PARA][2] = color_shape[ET_MIENTRAS][2] = 1;
	color_shape[ET_REPETIR][0] = .8;
	color_shape[ET_REPETIR][1] = .7;
	color_shape[ET_REPETIR][2] = 1;
	color_shape[ET_COMENTARIO][0] = 1;
	color_shape[ET_COMENTARIO][1] = 1;
	color_shape[ET_COMENTARIO][2] = 1;
	for(int i=0;i<ET_COUNT;i++) { 
		for(int j=0;j<3;j++) 
			color_border[i][j]=pow(color_shape[i][j],7)*.75;
	}
	
}

void GlobalInitPost() {
	win_h = big_icons ? 800 : 600;
	win_w = big_icons ? 1200 : 800; // tamaño de la ventana
	imgs_path = big_icons ? "imgs/flow/32/" : "imgs/flow/24/";
}

