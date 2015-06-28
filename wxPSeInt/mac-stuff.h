#ifndef MAC_STUFF_H
#define MAC_STUFF_H

#ifdef __WXMAC__
// esto es para evitar el problema de no poder hacerle foco a la ventana en Mac sin tener que hacer un application bundle (ver OnInit)
#include <ApplicationServices/ApplicationServices.h>
#endif


static void fix_mac_focus_problem() {
#ifdef __WXMAC__
	// esto es para evitar el problema de no poder hacerle foco a la ventana en Mac sin tener que hacer un application bundle
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication); // este es para que pueda recibir el foco
	SetFrontProcess( &PSN ); // este es para que no aparezca en segundo plano
#endif
}

#endif
