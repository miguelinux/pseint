// comunicacion con wxPseInt
#include "Global.h"

bool edit_on=true; // indica si se puede editar el diagrama
bool debugging=false; // indica si el programa se esta ejecutando paso a paso (en ese caso no se puede modificar edit_on)
string fname; // archivo que recibe como argumento
string pname; // nombre original del proceso

bool modified=false; // para saber si hay que preguntar antes de salir sin guardar

// lista de procesos/subprocesos
int choose_process_state; // indica si mostrando la lista de procesos/subprocesos para elegir cual editar
int choose_process_sel=0; // indica si mostrando la lista de procesos/subprocesos para elegir cual editar
int choose_process_d_base=0;
int choose_process_d_delta=0;

// estado para pasar entre eventos para la edicion
Entity *edit=NULL; // entidad seleccionado para editar su texto
Entity *mouse=NULL; // entidad seleccionado por el mouse
Entity *start=NULL; // entidad donde comienza el algoritmo
int m_x0=0; // click del mouse, para referencia en el motion, se setea en el mouse
int m_y0=0; // click del mouse, para referencia en el motion, se setea en el mouse
int blink=0; // cuando se esta editando texto, indica si se muestra o no el cursor, para que parpadee
int cur_x=0, cur_y=0; // ubicacion del raton (durante el passive_motion_cb esta sin aplicar zoom (coord del viewport), durante el motion y click, tiene la correccion del zoom (coord del dibujo))

// barra de entidades
int shapebar_size=0; // ancho para dibujo de la shapebar
bool shapebar=false; // indica si el mouse esta sobre la barra de formas
int shapebar_sel=0; // indica cual tipo de entidad esta seleccionada (1...n, 0 es ninguna) si shapebar==true

// menu
int menu_size_h=0; // ancho para dibujo de la shapebar
int menu_size_w=0; // ancho para dibujo de la shapebar
bool menu=false; // indica si el mouse esta sobre la papelera
int menu_sel=0; // indica cual elemento del menu esta seleccionado (1...n, 0 es ninguno) si menu==true

// papelera
bool trash=false; // indica si el mouse esta sobre la papelera
int trash_size=0; // ancho y alto de dibujo de la papelera

// zoom y panning
bool panning=false; // indica si se esta moviendo el dibujo, para el motion
bool selecting_zoom=false; // para hacer zoom en un area marcada, con el boton del medio, m_x0 y m_y0 guardan la primer esquina
float d_dx=0; // "paning" del dibujo
float d_dy=0; // "paning" del dibujo
float d_zoom=0.1; // zoom del dibujo
float zoom=0.1; // zoom final

// auxiliares varios
Entity *entity_to_del=NULL; // para un delayed delete (cuando suelta uno que sale del shapebar y no queda en ningun lado)
Entity *all_any=NULL;

// constantes con tamaños predefinidos
int win_h=400,win_w=600; // tamaño de la ventana

// colores
const float color_border[3]={0,0,0}; // borde de la forma de una entidad
const float color_label[3]={0,0,.8}; // texto de la etiqueta de una entidad
const float color_label_fix[3]={0.4,0,.4}; // texto de la etiqueta de una entidad
float color_arrow[3]={.8,0,0}; // flechas que guian el flujo y unen entidades
const float color_selection[3]={0,.4,0}; // texto o borde de forma seleccionada
float color_shape[ET_COUNT+1][3]; // fondo de la forma
const float color_shadow[3]={.7,.7,.7}; // sombras de las formas
const float color_back[3]={.97,1,.97}; // fondo de la pantalla
const float color_back_alt[3]={.95,.95,.95}; // fondo de pantalla alternativo para usar con los bloques de colores
const float color_ghost[3]={.7,.7,.7}; // borde fantasma cuando se arrastra una forma
const float color_menu[3]={.5,.2,.2}; // texto de los menues
const float color_menu_back[3]={.92,.97,.97}; // fondo de los menues
const float color_menu_sel[3]={.75,.95,.95}; // fondo de elemento de menu seleccionado
const float color_error[3]={.7,0,0}; // mensajes de error de sintaxis
bool draw_shadow=true; // si debe o no dibujar sombra (para la pantalla si, para exportar no)

bool word_operators=true; // al cargar el pseudocódigo, reemplaza algunos operadores por sus versiones en palabras
bool force_semicolons=false; // al cargar el pseudocódigo, reemplaza algunos operadores por sus versiones en palabras

vector<Entity*> procesos; // para almacenar el proceso principal y los subprocesos
map<string,LineInfo> code2draw; // para asociar las lineas de código al diagrama de flujo
Entity *debug_current=NULL; // la entidad que se esta ejecutando actualmente en el paso a paso

bool loading=false; // indica si se esta cargando un nuevo algoritmo desde un archivo para desactivar el evento de modificacion

void GlobalInit( ) {
	for(int i=0;i<=ET_COUNT;i++) { 
		color_shape[i][0] = 1;
		color_shape[i][1] = 1;
		color_shape[i][2] = .9;
	}
	color_shape[ET_LEER][0] = 1;
	color_shape[ET_LEER][1] = .9;
	color_shape[ET_LEER][2] = .95;
	color_shape[ET_ESCRIBIR][0] = .9;
	color_shape[ET_ESCRIBIR][1] = 1;
	color_shape[ET_ESCRIBIR][2] = .9;
	color_shape[ET_ASIGNAR][0] = 1;
	color_shape[ET_ASIGNAR][1] = .95;
	color_shape[ET_ASIGNAR][2] = .8;
	color_shape[ET_SI][0] = color_shape[ET_SEGUN][0] = color_shape[ET_OPCION][0] = .8;
	color_shape[ET_SI][1] = color_shape[ET_SEGUN][1] = color_shape[ET_OPCION][1] = .9;
	color_shape[ET_SI][2] = color_shape[ET_SEGUN][2] = color_shape[ET_OPCION][2] = 1;
	color_shape[ET_PARA][0] = color_shape[ET_MIENTRAS][0] = .9;
	color_shape[ET_PARA][1] = color_shape[ET_MIENTRAS][1] = .8;
	color_shape[ET_PARA][2] = color_shape[ET_MIENTRAS][2] = 1;
	color_shape[ET_REPETIR][0] = .8;
	color_shape[ET_REPETIR][1] = .7;
	color_shape[ET_REPETIR][2] = 1;
//	color_shape[ET_OPCION][0] = .95;
//	color_shape[ET_OPCION][1] = .95;
//	color_shape[ET_OPCION][2] = .95;
}

