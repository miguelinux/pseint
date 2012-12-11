#ifndef __APPLE__
#define cimg_use_png
#define cimg_use_jpeg
#endif
//#define cimg_use_tiff
#include "version.h"
#define t_color unsigned char
#include "draw.h"
#include "CImg.h"

using namespace std;
using namespace cimg_library;

unsigned char *me_colores[10];
CImgList<unsigned char> *the_font=NULL;

int get_text_ancho(string s) {
	int c, r=0, i=0, n=s.size();
	while (i<n) {
		c=s[i++]; if (c<0) c+=256;
		r+=(*the_font)[c].width();
	}
	return r;
}

void dibujar(CImg<unsigned char> &img ,t_dibujo &d, int x, int y, bool double_res){	
	{
		list <t_linea>::iterator p=d.lineas.begin(),q=d.lineas.end();
		while (p!=q){
			if (double_res) {
				img.draw_line((*p).x1+d.x+x,(*p).y1+d.y+y+1,(*p).x2+d.x+x,(*p).y2+d.y+y+1,me_colores[(*p).color]);
				img.draw_line((*p).x1+d.x+x,(*p).y1+d.y+y-1,(*p).x2+d.x+x,(*p).y2+d.y+y-1,me_colores[(*p).color]);
				img.draw_line((*p).x1+d.x+x+1,(*p).y1+d.y+y,(*p).x2+d.x+x+1,(*p).y2+d.y+y,me_colores[(*p).color]);
				img.draw_line((*p).x1+d.x+x-1,(*p).y1+d.y+y,(*p).x2+d.x+x-1,(*p).y2+d.y+y,me_colores[(*p).color]);
			}
			img.draw_line((*p).x1+d.x+x,(*p).y1+d.y+y,(*p).x2+d.x+x,(*p).y2+d.y+y,me_colores[(*p).color]);
			p++;
		}
	}

	
	{
		list <t_texto>::iterator p=d.textos.begin(),q=d.textos.end();
		while (p!=q){
			img.draw_text( (*p).x+d.x+x+2, (*p).y+d.y+y-c_textalto ,(*p).texto.c_str(), me_colores[(*p).color],0, 1.f,*the_font);
			p++;
		}
	}
	
	{
		list <t_circulo>::iterator p=d.circulos.begin(),q=d.circulos.end();
		while (p!=q){
			if (double_res) {
				img.draw_ellipse((*p).x+d.x+x+(*p).ancho/2, (*p).y+d.y+y+(*p).alto/2, (*p).ancho/2+1,(*p).alto/2+1, 0,me_colores[(*p).color],1,(unsigned int)-1);
				img.draw_ellipse((*p).x+d.x+x+(*p).ancho/2, (*p).y+d.y+y+(*p).alto/2, (*p).ancho/2+.5,(*p).alto/2+.5, 0,me_colores[(*p).color],1,(unsigned int)-1);
				img.draw_ellipse((*p).x+d.x+x+(*p).ancho/2, (*p).y+d.y+y+(*p).alto/2, (*p).ancho/2-1,(*p).alto/2-1, 0,me_colores[(*p).color],1,(unsigned int)-1);
				img.draw_ellipse((*p).x+d.x+x+(*p).ancho/2, (*p).y+d.y+y+(*p).alto/2, (*p).ancho/2-.5,(*p).alto/2-.5, 0,me_colores[(*p).color],1,(unsigned int)-1);
			}
			img.draw_ellipse((*p).x+d.x+x+(*p).ancho/2, (*p).y+d.y+y+(*p).alto/2, (*p).ancho/2,(*p).alto/2, 0,me_colores[(*p).color],1,(unsigned int)-1);
			p++;
		}
	}
	
	{
		list <t_dibujo>::iterator p=d.dibujos.begin(),q=d.dibujos.end();
		while (p!=q) {
			dibujar(img, (*p), d.x+x, d.y+y,double_res);	
			p++;
		}
	}
	
	
}


int main(int argc, char *argv[]){
	
	if (argc==2 && string(argv[1])=="--version") {
		_print_version_info("psDraw");
		return 0;
	}
	
	if (argc<2 || (argc==3 && argv[2][1]!='\0')) {
		cerr<<"Uso: psedraw <dibujo> [<format> <image_file>]\n";
		cerr<<"     psedraw <dibujo> [+]\n";
		cerr<<"  El + indica alta resolucion. Los formatos disponibles son bmp, png y jpg."<<endl;
		return 1;
	}
	cerr<<"Dibujando...";
	
	bool double_res = argc>2 && argv[2][0]=='+';
	
	
	if (double_res) {
		c_borde*=2.5;
		c_margen*=2.5;
		c_margenint*=2.5;
		c_margencond*=2.5;
		
		c_espacio*=2.5;
		c_flecha*=2.5;
		c_flechaio*=2.5;
		
		c_textline*=2.5;
//		c_textancho*=2.5;
		c_textancho=14;
		c_textalto*=2.5;
		
		c_font_size*=2.5;
	}

	unsigned char ec_negro[3]={0,0,0};
	unsigned char ec_rojo[3]={255,0,0};
	unsigned char ec_azul[3]={0,0,255};
	unsigned char ec_verde[3]={0,255,0};
	unsigned char ec_blanco[3]={255,255,255};
		
	me_colores[c_colnegro]=ec_negro;
	me_colores[c_colrojo]=ec_rojo;
	me_colores[c_colazul]=ec_azul;
	me_colores[c_colverde]=ec_verde;
	me_colores[c_colblanco]=ec_blanco;

	// crear fuente para dibujar mas tarde
	CImgList<unsigned char> font=CImgList<unsigned char>::font(c_font_size,true);
	for (int i=0;i<font.size();i++) { 
		font[i].resize(font[i].width()+2,-100,-100,-100,0);
		font[i].resize(-100,-100,-100,3,1);
	}
	CImg<unsigned char> lp1=font['^'];
	CImg<unsigned char> lp2=font['|'];
	CImg<unsigned char> lp=font['I'];
	
//	lp1.resize(lp.width(),lp.height(),-100,-100,1);
//	lp1.display();
//	lp2.resize(lp.width(),lp.height(),-100,-100,1);
//	lp2.display();
//	cimg_forXYZC(lp,x,y,z,c)
//		lp(x,y,z,c)=(lp1(x,y,z,c)||lp2(x,y,z,c))?255:0; 
	the_font=&font;
	
	//cargar programa y calcular dibujo
	t_programa p=cargar(argv[1]);
	t_dibujo d=diagrama(p);
	
	//crear area de grafico
	int a_ancho=d.derecha+d.izquierda,a_alto=d.alto;
	if (argc==3) {
		if (a_alto<c_size) a_alto=c_size;
		if (a_ancho<c_size) a_ancho=c_size;
	}

	CImg<unsigned char> imagenO(a_ancho,a_alto,1,3);	
	imagenO.fill(255,255,255);
	
//	CImg<unsigned char> black(1,1,1,3);	
//	black.fill(255,255,255);
	
	if ((*(p.begin())).nombre=="ERROR ERROR ERROR!") return 1;
	
	dibujar(imagenO,d,0,0,double_res);
	cerr<<"OK\n";
	
	if (argc>3) {
		cerr<<"Guardando imagen...";
		string format(argv[2]);
		if (format=="jpg" || format=="jpeg" || format=="+jpg" || format=="+jpeg") 
			imagenO.save_jpeg(argv[3]);
		else if (format=="bmp" || format=="+bmp") 
			imagenO.save_bmp(argv[3]);
		else if (format=="png" || format=="+png") 
			imagenO.save_png(argv[3]);
/*		else if (format=="tiff" || format=="tif") 
			imagenO.save_tiff(argv[3]);*/
		else {
			cerr<<"Error!\nAvailable formats are: jpeg, png, bmp.\n";
			return 2;
		}
		cerr<<"OK\n";
		return 0;
	}

	
	CImgDisplay ventana(imagenO.get_crop(0,0,500,500),"Diagrama de Flujo",0); 
	
	int sx=0,sy=0;
	
	
	// valores reales
	int rfw=ventana.screen_width(), rfh=ventana.screen_height();
	int rww=ventana.width(), rwh=ventana.height();
	int riw=imagenO.width(), rih=imagenO.height();

	// valores de despligue
	int sw=rww,sh=rwh;
	int ih=rih, iw=riw;
	int dww=rww,dwh=rwh;
	
	// posicion del cursor (porque al hacer click mouse_x y mouse_y se vuelven -1)
	int mx=rww/2,my=rwh/2;
	
	// recuerdos lwh,lww
	int lwh=0,lww=0;

	int lx=0,ly=0,lw=0,lh=0;
	bool moving=false;
	int lsx=0,lsy=0,lb=0;
	int lws=ventana.wheel(), ws=ventana.wheel();
	float sf=1; 
	float isf=1; 
	
	bool redraw=false, resize=false, zoomOnCursor=false;
	
	int lsw=0,lsh=0;
	
	lw=riw>rww?riw-rww:0;
	lh=rih>rwh?rih-rwh:0;
	
	// posicion inicial
	sx=riw-rww>0?(riw-rww)/2:0;
	redraw=true;

	unsigned int button, key;
//	char info[4]="+00";
	
	int lkey=0;
	while (!ventana.is_closed()){
		resize=false;
		zoomOnCursor=false; 

		key = ventana.key();
		ws = ventana.wheel();
		button = ventana.button();
		if (key!=lkey) lkey=key; else key=0;
		
		if (key==cimg::keyARROWLEFT) { sx-=sx>sw/4?sw/4:sx; redraw=true;  }
		if (key==cimg::keyARROWRIGHT) { sx+=sx+sw/4<lw?sw/4:lw-sx; redraw=true;  }
		if (key==cimg::keyARROWUP){ sy-=sy>sh/4?sh/4:sy; redraw=true;  }
		if (key==cimg::keyARROWDOWN){ sy+=sy+sh/4<lh?sh/4:lh-sy; redraw=true;  }

		if (ventana.is_closed() || key==cimg::keyQ || key==cimg::keyESC) { ventana.close(); break; }
		if (key=='+') { lws--; }
		if (key=='-') { lws++; }
		if (key==cimg::keyPADADD) { lws--; }
		if (key==cimg::keyPADSUB) { lws++; }
		if (button==2 && lb==0) { ws++; button=0; zoomOnCursor=true; }
		if (button==0) { 
			moving=false; 
			lb=0; 
		}
		if (ventana.mouse_x()>=0 && ventana.mouse_y()>=0) {
			mx=ventana.mouse_x(); my=ventana.mouse_y(); 
		}
		if (ws!=lws) {
			lsw=sw;lsh=sh;
			if (lws<ws) {
				sf=sf*1.25;
				isf=isf*0.8;
			} else {
				sf=sf*0.8;
				isf=isf*1.25;
			}
			sw=(int)(rww*isf);
			sh=(int)(rwh*isf);
			
			if (zoomOnCursor){
				sx+=lsw*mx/ventana.width()-sw/2;
				sy+=lsh*my/ventana.height()-sh/2;
			} else {
				sx+=(lsw-sw)/2;
				sy+=(lsh-sh)/2;
			}
			
			resize=true;
			lws=ws;
			
		}
		if (key==cimg::keyF) {
			key=cimg::keyO;
			ventana.toggle_fullscreen();
			if (ventana.is_fullscreen()) {
				lww=rww; lwh=rwh;
				ventana.resize(dww=rww=rfw,dwh=rwh=rfh,false);
			} else {
				ventana.resize(dww=rww=lww,dwh=rwh=lwh,false);
			}
			resize=true;
		}
		if (mx>=0 && my>=0 && moving && lx-mx && ly-my) {
			sx=lsx+(int)((lx-mx)*isf);
			sy=lsy+(int)((ly-my)*isf);
			redraw=true;
			lx=mx; ly=my; lsx=sx; lsy=sy;
		}
		if (button==1 && lb==0) { 
			lb=1; moving=true; 
			lx=mx; ly=my; 
			lsx=sx; lsy=sy;
		}
		
		
		if (ventana.is_resized()) { 
			ventana.resize(false);
			dww=rww=ventana.width();
			dwh=rwh=ventana.height();
			resize=true;
		}

		if (resize) {
			redraw=true;
//			if (rww>=(int)(riw*sf)) rww=(int)(riw*sf);
//			else if ((int)(riw*sf)>=rww) rww=(int)(riw*sf)>dww?dww:(int)(riw*sf);
//			if (rwh>=(int)(rih*sf)) rwh=(int)(rih*sf);
//			else if ((int)(rih*sf)>=rwh) rwh=(int)(rih*sf)>dwh?dwh:(int)(rih*sf);
			if ((int)(ventana.height())!=rwh || (int)(ventana.width())!=rww) {
//				if (ventana.is_fullscreen) ventana<<black;
				ventana.resize(rww,rwh,false);
			}
			sw=(int)(rww*isf);
			sh=(int)(rwh*isf);
			lw=iw>sw?iw-sw:0;
			lh=ih>sh?ih-sh:0;
		}
		if (redraw) {
//			cerr<<1;
//			if (sx>lw) sx=lw;
//			if (sy>lh) sy=lh;
//			if (sx<0) sx=0; if (sy<0) sy=0;
			ventana.render(imagenO.get_crop(sx,sy,sx+sw-1,sy+sh-1,true)).paint();	
			redraw=false;
		} else {
			ventana.wait();
		}
	}
	exit(0);
}


