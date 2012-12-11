#ifndef VERSION_COMMON_H
#define VERSION_COMMON_H

#if defined(__WIN32__)
#define ARCHITECTURE "w32"
#elif defined(__WIN64__)
#define ARCHITECTURE "w64"
#elif defined(__APPLE__)
#define ARCHITECTURE "mac"
#elif defined(__linux__) && defined(__x86_64__)
#define ARCHITECTURE "l64"
#elif defined(__linux__) && defined(__i386__)
#define ARCHITECTURE "l32"
#elif defined(__x86_64__)
#define ARCHITECTURE "?64"
#elif defined(__i386__)
#define ARCHITECTURE "?32"
#else
#define ARCHITECTURE "???"
#endif

#define _print_version_info(what) \
	cout<<what<<" "<<VERSION<<"-"<<ARCHITECTURE<<"\n" \
		"  http://pseint.sourceforge.net\n" \
		"  by Pablo Novara (zaskar_84@yahoo.com.ar)"<<endl

#endif
