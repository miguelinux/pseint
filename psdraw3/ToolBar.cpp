#include <cstddef>
#include "ToolBar.h"
#include "Events.h"
#include "Global.h"
#include "Draw.h"

ToolBar *g_toolbar = nullptr;

void ToolBar::SetSelection (int i) {
	m_selection = i;
}

void ToolBar::Draw ( ) {
	if (m_selection==MO_NONE) return;
	// the actual drawing of the toolbar is done by wxToolBar
	switch(m_selection) {
		case MO_ZOOM_EXTEND: SetStatus(g_colors.status,"Ajustar el zoom para visualizar todo el diagrama."); break;
		case MO_TOGGLE_FULLSCREEN: SetStatus(g_colors.status,"Alternar entre modo ventana y pantalla completa."); break;
		case MO_TOGGLE_COLORS: SetStatus(g_colors.status,"Cambiar tema de colores."); break;
		case MO_CROP_LABELS: SetStatus(g_colors.status,"Acortar textos muy largos en etiquetas."); break;
		case MO_TOGGLE_COMMENTS: SetStatus(g_colors.status,"Mostrar/Ocultar comentarios."); break;
		case MO_CHANGE_STYLE: SetStatus(g_colors.status,"Cambiar el tipo de diagrama (clásico, o Nassi-Shneiderman)."); break;
		case MO_FUNCTIONS: 
			SetStatus(g_colors.status,
					  g_lang[LS_PREFER_FUNCION] ? "Permite elegir cual función editar, crear una nueva o eliminar una existente."
					  : (g_lang[LS_PREFER_ALGORITMO] ? "Permite elegir cual subalgoritmo editar, crear uno nuevo o eliminar uno existente."
						 : "Permite elegir cual subproceso editar, crear uno nuevo o eliminar uno existente.") ); 
			break;
		case MO_RUN: SetStatus(g_colors.status,"Ejecuta el algoritmo en la terminal de PSeInt."); break;
		case MO_DEBUG: SetStatus(g_colors.status,"Ejecuta el algoritmo paso a paso marcando los pasos sobre el diagrama."); break;
		case MO_EXPORT: SetStatus(g_colors.status,"Permite guardar el diagrama como imagen."); break;
		case MO_CLOSE: SetStatus(g_colors.status,"Cierra el editor, preguntando antes si se deben aplicar los cambios en el pseudocódigo"); break;
		case MO_HELP: SetStatus(g_colors.status,"Muestra una ventana de ayuda que explica cómo utilizar este editor y cuáles son sus atajos de teclado."); break;
		default:
			;
	}
}

void ToolBar::Initialize ( ) {
	g_toolbar = new ToolBar();
}

