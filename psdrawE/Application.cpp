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
#include "../psdraw3/Load.h"
#include "../psdraw3/Global.h"
#include "../psdraw3/Entity.h"
#include "GLtoWX.h"
#include "Version.h"
#include <wx/choicdlg.h>
using namespace std;

void ProcessMenu(int) {}

class mxApplication : public wxApp {
public:
	virtual bool OnInit();
};

#ifdef __WXMAC__
// esto es para evitar el problema de no poder hacerle foco a la ventana en Mac sin tener que hacer un application bundle (ver OnInit)
#include <ApplicationServices/ApplicationServices.h>
#endif

IMPLEMENT_APP(mxApplication)
	
#if (wxUSE_LIBPNG==1)
#define _IF_PNG(x) x
#else
#define _IF_PNG(x)
#endif
#if (wxUSE_LIBJPEG==1)
#define _IF_JPG(x) x
#else
#define _IF_JPG(x)
#endif

bool mxApplication::OnInit() {
	
#ifdef __WXMAC__
	// esto es para evitar el problema de no poder hacerle foco a la ventana en Mac sin tener que hacer un application bundle
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication); // este es para que pueda recibir el foco
	SetFrontProcess( &PSN ); // este es para que no aparezca en segundo plano
#endif
	
	
	_handle_version_query("psDraw3");
	
	if (argc==1) {
		cerr<<"Use: "<<argv[0]<<" [--nassischneiderman] <input_file> <output_file>"<<endl;
	}
	
	_IF_PNG(wxImage::AddHandler(new wxPNGHandler));
	_IF_JPG(wxImage::AddHandler(new wxJPEGHandler));
	wxImage::AddHandler(new wxBMPHandler);
	
	// cargar el diagrama
	int c=0; bool force=false;
	wxString fin,fout;
	for(int i=1;i<argc;i++) { 
		if (wxString(argv[i])=="--nassischneiderman")
			Entity::nassi_schneiderman=true;
		else if (wxString(argv[i])=="--alternativeio")
			Entity::alternative_io=true;
		else if (wxString(argv[i])=="--force")
			force=true;
		else if (wxString(argv[i])=="--shapecolors")
			Entity::shape_colors=true;
		else if (wxString(argv[i]).Len()) {
			if (fin.Len()) fout=argv[i];
			else fin=argv[i]; 
		}
	}
	
	if (!Load(fin)) {
		wxMessageBox("Error al leer pseudocódigo"); return false;
	}
	edit_on=false;
	draw_shadow=false;
	
	// elegir un proceso/subproceso
	if (procesos.size()>1) {
		wxArrayString procs;
		for(unsigned int i=0;i<procesos.size();i++) 
			procs.Add((procesos[i]->lpre+procesos[i]->label).c_str());
		wxString r=wxGetSingleChoice("Elija el proceso/subproceso a exportar","Exportar diagrama de flujo",procs);
		int i=procs.Index(r);
		if (i!=wxNOT_FOUND)
			SetProc(procesos[i]);
	} else SetProc(procesos[0]);
	
	// calcular tamaño total
	zoom=d_zoom=1;
	int h=0,wl=0,wr=0, margin=10;
	start->Calculate(wl,wr,h); 
	int x0=start->x-wl,y0=start->y,x1=start->x+wr,y1=start->y-h;
	cerr<<x0<<","<<y0<<","<<y0<<","<<y1<<endl;
	start->Calculate();
	
	if (force) {
		zoom=d_zoom=1;
	} else {
		long ret=wxGetNumberFromUser(wxString("Ingrese la escala para la imagen (100% = ")<<(x1-x0)+2*margin<<"x"<<(y0-y1)+2*margin<<"px)","Zoom (%)","Generar imagen",100,1,1000);
		if (ret<=0) return false;
		zoom=d_zoom=float(ret)/100;
	}
	
	// hacer que las entidades tomen sus tamaños ideales
	Entity *e=Entity::all_any;
	do {
		e->Tick();
		e=e->all_next;
	} while (e!=Entity::all_any);

	// generar el bitmap
//	int margin=10;
	int bw=(x1-x0)*zoom+2*margin;
	int bh=(y0-y1)*zoom+2*margin;
	cerr<<bw<<","<<bh<<endl;
	wxBitmap bmp(bw,bh);
	dc=new wxMemoryDC(bmp);
	dc->SetBackground(wxColour(255,255,255));
	dc->Clear();
	
	// dibujar
	Entity *aux=start;
	glLineWidth(2*d_zoom<1?1:int(d_zoom*2));
	glPushMatrix();
	glScaled(d_zoom,-d_zoom,1);
	glTranslated(wl+margin,-margin,0);
	do {
		aux->Draw();
		aux=aux->all_next;
	} while (aux!=start);
	
	// guardar
	if (!force) {
		wxFileName fn(fout);
		wxFileDialog fd(NULL,"Guardar imagen",fn.GetPath(),fn.GetName()+".png",
			_IF_PNG("Imagen PNG|*.png;*.PNG|") _IF_JPG("Imagen jpeg|*.jpg;*.jpeg;*.JPG;*.JPEG|") "Imagen BMP|*.bmp;*.BMP",
			wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		if (fd.ShowModal()!=wxID_OK) { return false;  }
		fout=fd.GetPath();
	}
	
	wxBitmapType type;
	if (fout.Lower().EndsWith(".bmp")) type=wxBITMAP_TYPE_BMP;
	_IF_PNG(if (fout.Lower().EndsWith(".png")) type=wxBITMAP_TYPE_PNG;)
	_IF_JPG(else if (fout.Lower().EndsWith(".jpg")||fout.Lower().EndsWith(".jpeg")) type=wxBITMAP_TYPE_JPEG;)
	bmp.SaveFile(fout,type);
	
	cerr<<"saved"<<endl;
	
	return false;
}


