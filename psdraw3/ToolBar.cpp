#include <cstddef>
#include "ToolBar.h"
#include "Events.h"
#include "Global.h"
#include "Draw.h"

ToolBar *toolbar = NULL;

void ToolBar::SetSelection (int i) {
	m_selection = i;
}

void ToolBar::Draw ( ) {
	if (m_selection==MO_NONE) return;
	// the actual drawing of the toolbar is done by wxToolBar
	switch(m_selection) {
		case MO_ZOOM_EXTEND: SetStatus(color_selection,"Ajustar el zoom para visualizar todo el diagrama."); break;
		case MO_TOGGLE_FULLSCREEN: SetStatus(color_selection,"Alternar entre modo ventana y pantalla completa."); break;
		case MO_TOGGLE_COLORS: SetStatus(color_selection,"Alternar entre modo color y escala de grises."); break;
		case MO_CROP_LABELS: SetStatus(color_selection,"Acortar textos muy largos en etiquetas."); break;
		case MO_TOGGLE_COMMENTS: SetStatus(color_selection,"Mostrar/Ocultar comentarios."); break;
		case MO_CHANGE_STYLE: SetStatus(color_selection,"Cambiar el tipo de diagrama (clásico, o Nassi-Shneiderman)."); break;
		case MO_FUNCTIONS: 
			SetStatus(color_selection,
					  lang[LS_PREFER_FUNCION] ? "Permite elegir cual función editar, crear una nueva o eliminar una existente."
					  : (lang[LS_PREFER_ALGORITMO] ? "Permite elegir cual subalgoritmo editar, crear uno nuevo o eliminar uno existente."
						 : "Permite elegir cual subproceso editar, crear uno nuevo o eliminar uno existente.") ); 
			break;
		case MO_RUN: SetStatus(color_selection,"Ejecuta el algoritmo en la terminal de PSeInt."); break;
		case MO_DEBUG: SetStatus(color_selection,"Ejecuta el algoritmo paso a paso marcando los pasos sobre el diagrama."); break;
		case MO_EXPORT: SetStatus(color_selection,"Permite guardar el diagrama como imagen."); break;
		case MO_CLOSE: SetStatus(color_selection,"Cierra el editor, preguntando antes si se deben aplicar los cambios en el pseudocodigo"); break;
		case MO_HELP: SetStatus(color_selection,"Muestra una ventana de ayuda que explica cómo utilizar este editor y cuáles son sus atajos de teclado."); break;
		default:
			;
	}
}

void ToolBar::Initialize ( ) {
	toolbar = new ToolBar();
}

