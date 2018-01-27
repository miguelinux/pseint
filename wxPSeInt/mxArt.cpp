#include "mxArt.h"
#include "mxUtils.h"

#include <wx/dcmemory.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include "ConfigManager.h"
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <iostream>
using namespace std;
	
mxArt *bitmaps;

mxArt::mxArt(wxString img_dir) {
	
//	icons.error = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("icono_error.png")),wxBITMAP_TYPE_PNG);
//	icons.info = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("icono_info.png")),wxBITMAP_TYPE_PNG);
//	icons.warning = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("icono_warning.png")),wxBITMAP_TYPE_PNG);
//	icons.question = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("icono_question.png")),wxBITMAP_TYPE_PNG);
	
	buttons.ok = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_ok.png")),wxBITMAP_TYPE_PNG);
	buttons.cancel = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_cancel.png")),wxBITMAP_TYPE_PNG);
	buttons.help = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_ayuda.png")),wxBITMAP_TYPE_PNG);
	buttons.find = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_buscar.png")),wxBITMAP_TYPE_PNG);
	buttons.replace = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_reemplazar.png")),wxBITMAP_TYPE_PNG);
	buttons.next = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_next.png")),wxBITMAP_TYPE_PNG);
	buttons.prev = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_prev.png")),wxBITMAP_TYPE_PNG);
	buttons.stop = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_stop.png")),wxBITMAP_TYPE_PNG);
	buttons.load = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_abrir.png")),wxBITMAP_TYPE_PNG);
	buttons.save = new wxBitmap(DIR_PLUS_FILE(config->images_path,_T("boton_guardar.png")),wxBITMAP_TYPE_PNG);
	
}


mxArt::~mxArt() {
	delete buttons.ok;
	delete buttons.cancel;
	delete buttons.help;
	delete buttons.replace;
	delete buttons.find;
	delete buttons.next;
	delete buttons.prev;
	delete buttons.stop;
	
//	delete icons.info;
//	delete icons.error;
//	delete icons.warning;
//	delete icons.question;
}
