// comunicacion con wxPseInt
#include "Global.h"

bool edit_on=true; // indica si se puede editar el diagrama
string fname; // archivo que recibe como argumento
string pname; // nombre original del proceso

// cuadro de confirmacion al salir sin guardar
bool modified=false; // para saber si hay que preguntar antes de salir sin guardar
bool confirm=false; // indica si esta preguntando para salir sin guardar
int confirm_sel=0; // que hay seleccionado el cuadro de confirm, 1=si, 2=no, 0=nada

// lista de procesos/subprocesos
bool choose_process=false; // indica si mostrando la lista de procesos/subprocesos para elegir cual editar
bool choose_process_aux=false; // para evitar que al entrar al modo choose_process tome el mismo click del mouse que lo hizo entrar como eleccion del proceso
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
int cur_x, cur_y; // ubicacion del raton (en coord del dibujo)

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
double cosx[circle_steps+1], sinx[circle_steps+1]; // para no calcular en el DrawShape del Para

// colores
const float color_border[3]={0,0,0}; // borde de la forma de una entidad
const float color_label[3]={0,0,.9}; // texto de la etiqueta de una entidad
const float color_label_fix[3]={0.4,0,.4}; // texto de la etiqueta de una entidad
const float color_arrow[3]={.9,0,0}; // flechas que guian el flujo y unen entidades
const float color_selection[3]={0,.4,0}; // texto o borde de forma seleccionada
const float color_shape[3]={1,1,.9}; // fondo de la forma
const float color_shadow[3]={.7,.7,.7}; // sombras de las formas
const float color_back[3]={.95,1,.95}; // fondo de la pantalla
const float color_ghost[3]={.7,.7,.7}; // borde fantasma cuando se arrastra una forma
const float color_menu[3]={.7,.2,.2}; // texto de los menues
const float color_menu_back[3]={.8,.95,.95}; // fondo de los menues

bool word_operators=true; // al cargar el pseudocódigo, reemplaza algunos operadores por sus versiones en palabras

vector<Entity*> procesos;
