#include <iostream>
#include <cstdlib>
#include <wx/app.h>
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/choicdlg.h>
#include "../psdraw3/Load.h"
#include "../psdraw3/Global.h"
#include "../psdraw3/Entity.h"
#include "../pseint/LangSettings.h"
#include "../wxPSeInt/osdep.h"
#include "GLtoWX.h"
#include "Version.h"
#include "mxConfig.h"
#include "../wxPSeInt/string_conversions.h"
using namespace std;

void ProcessMenu(int) {}

class mxApplication : public wxApp {
public:
	virtual bool OnInit();
};

IMPLEMENT_APP(mxApplication)
	
#if (wxUSE_LIBPNG==1)
#	define _IF_PNG(x) x
#else
#	define _IF_PNG(x)
#endif
#if (wxUSE_LIBJPEG==1)
#	define _IF_JPG(x) x
#else
#	define _IF_JPG(x)
#endif
	
LangSettings lang(LS_DO_NOT_INIT);

void SetModified() {}

bool mxApplication::OnInit() {
	
	_handle_version_query("psDrawE");
	
	OSDep::AppInit();
	
	if (argc==1) {
		cerr<<"Use: "<<argv[0]<<" [--use_nassi_shneiderman=1] [--use_alternative_io_shapes=1] [--shape_colors] <input_file> <output_file>"<<endl;
	}

	lang.Reset();
	
	_IF_PNG(wxImage::AddHandler(new wxPNGHandler));
	_IF_JPG(wxImage::AddHandler(new wxJPEGHandler));
	wxImage::AddHandler(new wxBMPHandler);
	
	// cargar el diagrama
	bool force=false;
	Entity::enable_partial_text=false;
	Entity::show_comments=false;
	wxString fin,fout;
	for(int i=1;i<argc;i++) { 
		wxString arg(argv[i]);
		if (arg=="--force") {
			force=true;
		} else if (arg=="--shapecolors") {
			Entity::shape_colors=true;
		} else if (arg=="--nocroplabels") {
			; // siempre es asi, parsear esto es solo para que no genere error
		} else if (arg.StartsWith("--") && lang.ProcessConfigLine(arg.Mid(2).c_str())) {
			; // procesado en lang.ProcessConfigLine
		} else if (arg.Len()) {
			if (fin.Len()) fout=arg;
			else fin=arg;
		}
	}
	lang.Fix();
	Entity::nassi_shneiderman=lang[LS_USE_NASSI_SHNEIDERMAN];
	Entity::alternative_io=lang[LS_USE_ALTERNATIVE_IO_SHAPES];
	GlobalInitPre(); GlobalInitPost();
	if (!Load(fin)) {
		wxMessageBox(_Z("Error al leer pseudocódigo")); return false;
	}
	edit_on=false;
	if ((new mxConfig())->ShowModal()==wxID_CANCEL) return 0; // opciones del usuairo
	
	if (!Entity::shape_colors) {
		// fondo
		color_shape[ET_COUNT][0] = .97f;
		color_shape[ET_COUNT][1] = .97f;
		color_shape[ET_COUNT][2] = .97f;
		// flechas
		color_arrow[0] = .15f;
		color_arrow[1] = .15f;
		color_arrow[2] = .15f;
		// texto
		for(int j=0;j<6;j++) {
			color_label_high[j][0] = 0.0f;
			color_label_high[j][1] = 0.0f;
			color_label_high[j][2] = 0.0f;
		}
	}
	
	// calcular tamaño total
	int h=0,wl=0,wr=0, margin=10;
	Entity *real_start = start->GetTopEntity();
	real_start->Calculate(wl,wr,h); 
	int x0=real_start->x-wl,y0=real_start->y,x1=real_start->x+wr,y1=real_start->y-h;
	real_start->Calculate();
	
	// hacer que las entidades tomen sus tamaños ideales
	Entity::AllIterator it = Entity::AllBegin();
	while (it!=Entity::AllEnd()) {
		it->Tick();
		++it;
	}

	// generar el bitmap
//	int margin=10;
	int bw=((x1-x0)+2*margin)*zoom;
	int bh=((y0-y1)+2*margin)*zoom;
//	cerr<<bw<<","<<bh<<endl;
	wxBitmap bmp(bw,bh);
	dc=new wxMemoryDC(bmp);
	dc->SetBackground(wxColour(255,255,255));
	dc->Clear();
	
	// dibujar
	Entity *aux=real_start;
	line_width_flechas=2*d_zoom<1?1:int(d_zoom*2);
	line_width_bordes=1*d_zoom<1?1:int(d_zoom*1);
	glLineWidth(line_width_flechas);
	glPushMatrix();
	glScaled(d_zoom,-d_zoom,1);
	glTranslated(wl+margin,-margin,0);
	do {
		aux->Draw();
		aux=Entity::NextEntity(aux);
	} while (aux);
	
	// guardar
	if (!force) {
		wxFileName fn(fout);
		wxFileDialog fd(NULL,_Z("Guardar imagen"),fn.GetPath(),fn.GetName()+".png",
			_Z( _IF_PNG("Imagen PNG|*.png;*.PNG|") _IF_JPG("Imagen jpeg|*.jpg;*.jpeg;*.JPG;*.JPEG|") "Imagen BMP|*.bmp;*.BMP"),
			wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		if (fd.ShowModal()!=wxID_OK) { return false;  }
		fout=fd.GetPath();
	}
	
	wxBitmapType type;
	if (fout.Lower().EndsWith(".bmp")) type=wxBITMAP_TYPE_BMP;
	_IF_PNG(if (fout.Lower().EndsWith(".png")) type=wxBITMAP_TYPE_PNG;)
	_IF_JPG(else if (fout.Lower().EndsWith(".jpg")||fout.Lower().EndsWith(".jpeg")) type=wxBITMAP_TYPE_JPEG;)
	if (bmp.SaveFile(fout,type)) {
		if (force) cerr << _Z("Guardado: ")<<fout<<endl;
		else wxMessageBox(_Z("Diagrama guardado"),_Z("PSeInt"));
	}
	
	return false;
}
