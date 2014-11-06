#ifndef ENTITY_H
#define ENTITY_H
#include <string>
using namespace std;

#ifdef DrawText
// maldito windows.h
#undef DrawText
#endif

/*
Las entidades se relacionan como pseudo-arbol, empezando por start que es "Inicio".
Ademas, todas forman una gran lista enlazada (atributos all_*) para recorrerlas en los eventos
Tipos de entidades:
	ET_LEER: leer, no tiene hijos
	ET_ESCRIBIR: escribir, no tiene hijos
	ET_ASIGNAR: asignar, dimension, definir, no tiene hijos
	ET_PROCESO: inicio y fin, se distinguen por prev y next
	ET_SI: si, dos hijos, el primero es el izquierdo, el segundo el derecho
	ET_SEGUN: 1 o mas hijos, el ultimo siempre es "De Otro Modo"
	ET_OPCION: 1 hijo, hijos de segun con las opciones, el hijo tiene las acciones para esa opcion
	ET_MIENTRAS: mientras, un solo hijo con el contenido del bucle
	ET_REPETIR: repetir, un solo hijo con el contenido del bucle, el default es repetir hasta que, la variante es repetir-mientras que
	ET_PARA: para, 4 hijos, el 0 es el contenido del bucle, el 1,2,3 son val inicial, paso y final, estan fijos, los crea el ctor, la variante es para cada, donde hijo 2 tiene el arreglo
	ET_AUX_PARA: hijos 1,2,3 y de para
*/
enum ETYPE { ET_LEER, ET_PROCESO, ET_ESCRIBIR, ET_SI, ET_SEGUN, ET_OPCION, ET_PARA, ET_MIENTRAS, ET_REPETIR, ET_ASIGNAR, ET_AUX_PARA, ET_COUNT };

struct Entity {
	static bool alternative_io; ///< utilizar simbolos alternativos para las instrucciones Leer y Escribir
	static bool nassi_schneiderman; ///< usar diagramas de Nassi-Schneiderman en lugar de "clásico"
	static bool shape_colors; ///< mostrar los bloques de diferentes colores
	Entity *all_next, *all_prev;
	static Entity *all_any;
	ETYPE type;
	// cosas definidas por el constructor al setear la etiqueta
	int w,h; // tamaño real
	int t_w,t_h,t_dy,t_dx,t_prew; // tamaño del texto de la etiqueta, deltapara el texto (para y segun), y tamaño del prefijo no editable
	int x,y; // posiciones reales absolutas, independientes del dibujo, del punto de entrada al bloque completo
	// cosas que debe setear la funcion Calculate...
	int fx,fy; // posiciones reales absolutas del punto de entrada a la forma de la entidad
	int bwl, bwr,bh; // tamaño del bloque completo real, incluyendo hijos y la cfl
	// cosas auxiliares para animaciones
	int d_fx,d_fy; // posiciones para el dibujo del punto de entrada a la forma de la entidad
	int d_w, d_h; // tamaño de la forma en el dibujo, tiende a w,h con el tiempo
	int d_x, d_y; // posiciones del dibujo, tienden a x+d_dx,y+d_dy con el tiempo
	int d_bwl, d_bwr,d_bh; // tamaño del bloque completo en el dibujo, incluyendo hijos
	// otras
	int m_x,m_y; // coordenadas del mouse relativas a x,y, para cuando se arrastra
	Entity *next,*prev,*parent;
	int child_id; // indice de this en el arreglo child del parent
	Entity **child; // arreglo de hijos (primer entidad de los sub-bloques, 1 para repetitivas, 2 para si, n para segun)
	int *child_dx; // posicion de los hijos relativa al x del padre(this)
	int *child_bh; // arreglo con el alto tiene cada hijo para que el padre sepa desde donde cerrar las flechas
	int n_child; // cantidad de hijos
	int flecha_in; // si las flechas de entrada son mas largas que lo normal (ej, entrada en un repetitivo), se pone aca la diferencia (esto se podria sacar, no?)
	Entity *nolink; // elemento seleccionado, para que los hijos se escondan atras del padre mientras se mueve al padre
	bool variante; // true convierte repetir-hastaque en repetir-mientrasque o para en paracada
	string label, lpre; // rotulo(editable) y prefijo(no editable)
	string error; // mensaje de error si es que la estructura tiene un error de sintaxis
	Entity(ETYPE _type, string _label);
	~Entity();
	void SetEdit();
	void SetMouse();
	void UnSetMouse();
	void SetNolink(Entity *m,bool n);
	void EditSpecialLabel(int key);
	void EditLabel(unsigned char key);
	void GetTextSize(const string &label, int &w, int &h);
	void SetLabel(string _label, bool recalc=false);
	int CheckLinkChild(int x, int y);
	int CheckLinkOpcion(int x, int y);
	bool CheckLinkNext(int x, int y);
	void UnLink();
	void LinkNext(Entity *e);
	void RemoveChild(int j);
	void InsertChild(int i, Entity *e);
	void MoveChild(int i0, int i1);
	void LinkChild(int i, Entity *e);
	void Tick();
	void DrawShapeSolid(const float *color,int x, int y, int w, int h);
	void DrawShapeBorder(const float *color,int x, int y, int w, int h);
	void DrawText();
	void Draw(bool force=false);
	void DrawNassiSchne(bool force=false);
	void DrawClasico(bool force=false);
	void Calculate(bool also_parent=false);
	void MoveX(int dx);
	void ResizeW(int aw, bool up);
	void Calculate(int &gwl, int &gwr, int &gh);
	void CalculateNassiSchne();
	void CalculateClasico();
	void CopyPos(Entity *o);
	bool CheckMouse(int x, int y, bool click=true);
	void Print(ostream &out, string tab, Entity *process, int &line_num);
	void SetPosition(int x0, int y0); // para moverla por la fuerza, para ubicarla en la shapebar cuando se crea
};


static const int flecha_h=25; // separacion entre bloques consecutivos
static const int flecha_w=20; // separacion entre bloques hermanos
static const int flecha_d=5; // tamaño de la punta de la flecha
static const int selection_tolerance_y=15; // tolerancia en y para la seleccion de puntos
static const int selection_tolerance_x=30; // tolerancia en x para la seleccion de puntos
static const int margin=6; // margen entre cuadro y texto en un bloque (y para los botones de confirm, por eso no es static, ¿ni const?)
static const int vf_size=5;
static const int shadow_delta_x=4; // diferencia entre la posicion de un objeto y su sombra
static const int shadow_delta_y=5; // diferencia entre la posicion de un objeto y su sombra

#endif

