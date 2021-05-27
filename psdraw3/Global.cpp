// comunicacion con wxPseInt
#include <cmath>
#include "Global.h"

GConfig g_config;
GConstants g_constants;
GColors g_colors;
GState g_state;
GView g_view;
GCode g_code;

//LangSettings lang; // no definir aca, ver comentario en el global.cpp de pseint

void GlobalInitPre( ) {
	
	for(int i=0;i<ET_COUNT;i++)
		g_constants.max_label_len[i]=15;
	g_constants.max_label_len[ET_COMENTARIO]=25;
	g_constants.max_label_len[ET_LEER]=25;
	g_constants.max_label_len[ET_PROCESO]=25;
	g_constants.max_label_len[ET_ESCRIBIR]=25;
	g_constants.max_label_len[ET_ASIGNAR]=25;
	g_constants.max_label_len[ET_SI]=15;
	g_constants.max_label_len[ET_SEGUN]=15;
	g_constants.max_label_len[ET_OPCION]=10;
	g_constants.max_label_len[ET_PARA]=15;
	g_constants.max_label_len[ET_MIENTRAS]=15;
	g_constants.max_label_len[ET_REPETIR]=15;
	g_constants.max_label_len[ET_AUX_PARA]=10;
	
	SetColors();
}

void GlobalInitPost() {
	g_view.win_h = g_config.big_icons ? 800 : 600;
	g_view.win_w = g_config.big_icons ? 1200 : 800; // tamaño de la ventana
	g_constants.imgs_path = g_config.big_icons ? "imgs/flow/32/" : "imgs/flow/24/";
}

static void SetColor(float *c, float r, float g, float b) { c[0] = r; c[1] = g; c[2] = b; }

void SetColors( ) {

	SetColor(g_colors.border[ET_COUNT+1], 0,0,0);
	SetColor(g_colors.comment, .4,.4,.4);
	SetColor(g_colors.label_high[0],0,0,0);
	SetColor(g_colors.label_high[1],.60,.40,.15);
	SetColor(g_colors.label_high[2],0,.55,0);
	SetColor(g_colors.label_high[3],0,0,.6);
	SetColor(g_colors.label_high[4],.4,.4,.4);
	SetColor(g_colors.label_high[5],0,0,.6);
	SetColor(g_colors.arrow,.8,0,0);
	SetColor(g_colors.selection,0,.4,0);
	SetColor(g_colors.ghost,.7,.7,.7);
	SetColor(g_colors.menu,.4,.4,.4);
	SetColor(g_colors.menu_bold,0,0,0);
	SetColor(g_colors.menu_back,.92,.97,.95);
	SetColor(g_colors.menu_sel,.75,.95,.95);
	SetColor(g_colors.error,.7,0,0);
	
	if (g_config.shape_colors) SetColor(g_colors.back, .97,1,.97); // fondo de la pantalla
	else                       SetColor(g_colors.back, 1,1,1); // fondo de pantalla alternativo para usar con los bloques de colores
	
	
	for(int i=0;i<ET_COUNT;i++) { 
		g_colors.shape[i][0] = 1;
		g_colors.shape[i][1] = .95;
		g_colors.shape[i][2] = .8;
	}
	g_colors.shape[ET_COUNT][0] = .95;
	g_colors.shape[ET_COUNT][1] = .95;
	g_colors.shape[ET_COUNT][2] = .95;
	g_colors.shape[ET_LEER][0] = 1;
	g_colors.shape[ET_LEER][1] = .9;
	g_colors.shape[ET_LEER][2] = .95;
	g_colors.shape[ET_ESCRIBIR][0] = .9;
	g_colors.shape[ET_ESCRIBIR][1] = 1;
	g_colors.shape[ET_ESCRIBIR][2] = .9;
	g_colors.shape[ET_ASIGNAR][0] = 1;
	g_colors.shape[ET_ASIGNAR][1] = 1;
	g_colors.shape[ET_ASIGNAR][2] = .85;
	g_colors.shape[ET_SI][0] = g_colors.shape[ET_SEGUN][0] = g_colors.shape[ET_OPCION][0] = .85;
	g_colors.shape[ET_SI][1] = g_colors.shape[ET_SEGUN][1] = g_colors.shape[ET_OPCION][1] = .9;
	g_colors.shape[ET_SI][2] = g_colors.shape[ET_SEGUN][2] = g_colors.shape[ET_OPCION][2] = 1;
	g_colors.shape[ET_PARA][0] = g_colors.shape[ET_MIENTRAS][0] = .95;
	g_colors.shape[ET_PARA][1] = g_colors.shape[ET_MIENTRAS][1] = .8;
	g_colors.shape[ET_PARA][2] = g_colors.shape[ET_MIENTRAS][2] = 1;
	g_colors.shape[ET_REPETIR][0] = .8;
	g_colors.shape[ET_REPETIR][1] = .7;
	g_colors.shape[ET_REPETIR][2] = 1;
	g_colors.shape[ET_COMENTARIO][0] = 1;
	g_colors.shape[ET_COMENTARIO][1] = 1;
	g_colors.shape[ET_COMENTARIO][2] = 1;
	for(int i=0;i<ET_COUNT;i++) { 
		for(int j=0;j<3;j++) 
			g_colors.border[i][j]=pow(g_colors.shape[i][j],7)*.75;
	}
}

void SetColors(wxColour toolbar_color, wxColour selection_color) {
	g_colors.menu_back[0] = toolbar_color.Red()/255.f;
	g_colors.menu_back[1] = toolbar_color.Green()/255.f;
	g_colors.menu_back[2] = toolbar_color.Blue()/255.f;
	g_colors.menu_sel[0] = selection_color.Red()/255.f;
	g_colors.menu_sel[1] = selection_color.Green()/255.f;
	g_colors.menu_sel[2] = selection_color.Blue()/255.f;
	
}

