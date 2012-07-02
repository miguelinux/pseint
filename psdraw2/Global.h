#ifndef GLOBAL_H
#define GLOBAL_H
#include <string>
using namespace std;

class Entity;

// comunicacion con wxPseInt
extern bool edit_on; // indica si se puede editar el diagrama
extern string fname; // archivo que recibe como argumento
extern string pname; // nombre original del proceso

// cuadro de confirmacion al salir sin guardar
extern bool modified; // para saber si hay que preguntar antes de salir sin guardar
extern bool confirm; // indica si esta preguntando para salir sin guardar
extern int confirm_sel; // que hay seleccionado el cuadro de confirm, 1=si, 2=no, 0=nada

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
#define menu_w_max 240
#define menu_h_min 30
#define menu_w_min 70
#define menu_line_width 1
#define menu_option_height 30


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
#define circle_steps 20 // cantidad de tramos en los que aproximo el circulo para dibujarlo como poligonal
extern double cosx[circle_steps+1], sinx[circle_steps+1]; // para no calcular en el DrawShape del Para

// colores
extern const float color_border[3]; // borde de la forma de una entidad
extern const float color_label[3]; // texto de la etiqueta de una entidad
extern const float color_label_fix[3]; // texto de la etiqueta de una entidad
extern const float color_arrow[3]; // flechas que guian el flujo y unen entidades
extern const float color_selection[3]; // texto o borde de forma seleccionada
extern const float color_shape[3]; // fondo de la forma
extern const float color_shadow[3]; // sombras de las formas
extern const float color_back[3]; // fondo de la pantalla
extern const float color_ghost[3]; // borde fantasma cuando se arrastra una forma
extern const float color_menu[3]; // texto de los menues
extern const float color_menu_back[3]; // fondo de los menues

extern bool word_operators; // al cargar el pseudocódigo, reemplaza algunos operadores por sus versiones en palabras

// para interpolar en las animaciones, good converge pixel perfect, la otra puede que no
//#define interpolate(a,b) a=(2*a+b)/3
#define interpolate(a,b) if ((a)+3>(b) && (a)-3<(b)) a=b; else a=(2*a+b)/3
#define interpolate_good(a,b) if ((a)+3>(b) && (a)-3<(b)) a=b; else a=(2*a+b)/3

//#define interpolate(a,b) a=b
//#define interpolate_good(a,b) a=b

#endif

