#ifndef STRING_CONVERSIONS_H
#define STRING_CONVERSIONS_H
#include <wx/string.h>

#if wxABI_VERSION<30000
#	ifdef __APPLE__
static wxCSConv cscUTF8("utf8");
static wxCSConv cscMAC("MAC");
#		define _Z(x)   cscMAC.cWC2MB(cscUTF8.cMB2WC(x))
#		define _ZZ(x)  wxString(cscUTF8.cMB2WC(x),cscMAC)
#		define _FixW(x) cscMAC.cWC2MB(x.wchar_str())
#	else
#		define _ZZ(x) wxString(x)
#		define _Z(x) x
#		define _FixW(x) x
#	endif
#	define _S2W(stds) stds.c_str()
#	define _W2S(wxs) wxs.c_str()
#	define _W2S_Len(wxs) wxs.Len()
#	define _C(wxchar) wxchar
#	define _wxEvtTimer_to_wxTimerPtr(evt) evt.GetEventObject()
#	define _if_wx2(a) a
#	define _if_wx3(a) 
#	define _if_wx3_else(a,b) b 
#	define _if_unicode(x)
#else	
//#	define _FixW(x) cscMAC.cWC2MB(x.wchar_str())
#	define _FixW(x) x
#	define WX3
#	define _Z(x) wxString::From8BitData(x)
#	define _ZZ(x) wxString::From8BitData(x)
#	define _W2S(wxstring) ((const char*)(wxstring.mb_str()))
#	define _W2S_Len(wxstring) strlen((const char*)(wxstring.mb_str()))
#	define _C(wxchar) ((wxchar).GetValue())
#	define _wxEvtTimer_to_wxTimerPtr(evt) &(evt.GetTimer())
#	define _if_wx2(a)
#	define _if_wx3(a) a
#	define _if_wx3_else(a,b) a
# 	ifdef UNICODE
#		define _if_unicode(x) x
#	else
#		define _if_unicode(x)
#	endif
#endif

#endif
