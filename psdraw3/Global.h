#ifndef GLOBAL_H
#define GLOBAL_H
#include <string>
#include <vector>
#include <map>
#include "Entity.h"
#include "../pseint/LangSettings.h"
using namespace std;


// comunicacion con wxPseInt
extern LangSettings lang;
extern bool edit_on; // indica si se puede editar el diagrama
extern bool debugging; // indica si el programa se esta ejecutando paso a paso (en ese caso no se puede modificar edit_on)
extern string fname; // archivo que recibe como argumento

extern bool modified; // para saber si hay que preguntar antes de salir sin guardar

// lista de procesos/subprocesos
extern int choose_process_state; // indica si mostrando la lista de procesos/subprocesos para elegir cual editar
extern int choose_process_sel; // indica si mostrando la lista de procesos/subprocesos para elegir cual editar
extern int choose_process_d_base;
extern int choose_process_d_delta;
#define choose_process_base 40
#define choose_process_delta 30

// estado para pasar entre eventos para la edicion
extern Entity *edit; // entidad seleccionado para editar su texto
extern Entity *mouse; // entidad seleccionado por el mouse
extern Entity *start; // entidad donde comienza el algoritmo
extern int m_x0; // click del mouse, para referencia en el motion, se setea en el mouse
extern int m_y0; // click del mouse, para referencia en el motion, se setea en el mouse
extern int blink; // cuando se esta editando texto, indica si se muestra o no el cursor, para que parpadee
extern int cur_x, cur_y; // ubicacion del raton (en coord del dibujo)

// barra de entidades
extern int shapebar_size; // ancho para dibujo de la shapebar
#define shapebar_size_min 25
#define shapebar_size_max 150
extern bool shapebar; // indica si el mouse esta sobre la barra de formas
extern int shapebar_sel; // indica cual tipo de entidad esta seleccionada (1...n, 0 es ninguna) si shapebar==true

// menu
extern int menu_size_h; // ancho para dibujo de la shapebar
extern int menu_size_w; // ancho para dibujo de la shapebar
extern bool menu; // indica si el mouse esta sobre la papelera
extern int menu_sel; // indica cual elemento del menu esta seleccionado (1...n, 0 es ninguno) si menu==true
#define menu_w_max_base 240
#define menu_w_min_base 70
#ifdef _USE_TEXTURES
#define menu_w_max (menu_w_max_base+(use_textures?20:0))
#define menu_w_min (menu_w_min_base+(use_textures?10:0))
#else
#define menu_w_max menu_w_max_base
#define menu_w_min menu_w_min_base
#endif
#define menu_h_min 30
#define menu_line_width 1
#define menu_option_height 25


// papelera
extern bool trash; // indica si el mouse esta sobre la papelera
extern int trash_size; // ancho y alto de dibujo de la papelera
#define trash_size_max 100
#define trash_size_min 30

// zoom y panning
extern int win_h,win_w; // tamaño de la ventana
extern bool panning; // indica si se esta moviendo el dibujo, para el motion
extern bool selecting_zoom; // para hacer zoom en un area marcada, con el boton del medio, m_x0 y m_y0 guardan la primer esquina
extern float d_dx; // "paning" del dibujo
extern float d_dy; // "paning" del dibujo
extern float d_zoom; // zoom del dibujo
extern float zoom; // zoom final

// auxiliares varios
extern Entity *entity_to_del; // para un delayed delete (cuando suelta uno que sale del shapebar y no queda en ningun lado)
extern Entity *all_any;

// colores
extern float color_border[ET_COUNT+1][3]; // borde de la forma de una entidad
extern const float color_comment[3]; // para borde y fuente de las entidades con comentarios
extern const float color_label[3]; // texto de la etiqueta de una entidad
extern const float color_label_fix[3]; // texto de la parte (pre)fija de la etiqueta de una entidad
extern float color_arrow[3]; // flechas que guian el flujo y unen entidades
extern const float color_selection[3]; // texto o borde de forma seleccionada
extern float color_shape[ET_COUNT+1][3]; // fondo de la forma
//extern const float color_shadow[3]; // sombras de las formas
extern const float color_back[3]; // fondo de la pantalla
extern const float color_back_alt[3]; // fondo de pantalla alternativo para usar con los bloques de colores
extern const float color_ghost[3]; // borde fantasma cuando se arrastra una forma
extern const float color_menu[3]; // texto de los menues
extern const float color_menu_back[3]; // fondo de los menues
extern const float color_menu_sel[3]; // fondo de elemento de menu seleccionado
extern const float color_error[3]; // mensajes de error de sintaxis
//extern bool draw_shadow; // si debe o no dibujar sombra (para la pantalla si, para exportar no)

// para interpolar en las animaciones, good converge pixel perfect, la otra puede que no
#ifdef _FOR_EXPORT
	#define interpolate(a,b) a=b
	#define interpolate_good(a,b) a=b
#else
	//#define interpolate(a,b) a=(2*a+b)/3
	#define interpolate(a,b) if ((a)+3>(b) && (a)-3<(b)) a=b; else a=(2*a+b)/3
	#define interpolate_good(a,b) if ((a)+3>(b) && (a)-3<(b)) a=b; else a=(2*a+b)/3
#endif


// para almacenar el proceso principal y los subprocesos
// la forma de gestionar todo esto es tener los diagramas cargados todos a la
// vez pero independientes, para lo cual trucheo start y Entity::all_any en 
// Load y SetProc
extern vector<Entity*> procesos;

// para asociar las lineas de código al diagrama de flujo
struct LineInfo { Entity *proceso, *entidad; LineInfo (Entity *p=NULL, Entity *e=NULL):proceso(p),entidad(e) {} };
extern map<string,LineInfo> code2draw;
extern Entity *debug_current;  // la entidad que se esta ejecutando actualmente en el paso a paso

extern bool loading; // indica si se esta cargando un nuevo algoritmo desde un archivo para desactivar el evento de modificacion

void GlobalInit(); // inicializaciones globales que no pueden ser estáticas	

#endif

