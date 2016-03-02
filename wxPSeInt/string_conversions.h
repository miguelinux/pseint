#ifndef STRING_CONVERSIONS_H
#define STRING_CONVERSIONS_H
#include <wx/string.h>

#if wxABI_VERSION<30000
#	define _Z(x) x
#	define _ZZ(x) wxString(x)
#	define _W2S(wxs) wxs.c_str()
#	define _C(wxchar) wxchar
#	define _wxEvtTimer_to_wxTimerPtr(evt) evt.GetEventObject()
#	define _if_wx2(a) a
#	define _if_wx3(a) 
#	define _if_wx3_else(a,b) b 
#else	
#	define WX3
#	define _Z(x) wxString::From8BitData(x)
#	define _ZZ(x) wxString::From8BitData(x)
#	define _W2S(wxstring) ((const char*)(wxstring.mb_str()))
#	define _C(wxchar) ((wxchar).GetValue())
#	define _wxEvtTimer_to_wxTimerPtr(evt) &(evt.GetTimer())
#	define _if_wx2(a)
#	define _if_wx3(a) a
#	define _if_wx3_else(a,b) a
#endif

#endif
