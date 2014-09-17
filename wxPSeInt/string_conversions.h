#ifndef STRING_CONVERSIONS_H
#define STRING_CONVERSIONS_H
#include <wx/string.h>

#if wxABI_VERSION<30000
	#define _Z(x) x
	#define _ZZ(x) wxString(x)
#else	
	#define _Z(x) wxString::From8BitData(x)
	#define _ZZ(x) wxString::From8BitData(x)
#endif

#endif
