#ifdef _FOR_EXPORT
	#include "GLtoWX.h"
#else
	#ifdef __APPLE__
		#include <OpenGL/gl.h>
		#include <OpenGL/glu.h>
		#include <GLUT/glut.h>
	#else
		#ifdef _USE_DF
			#include "gl_lite.h"
		#endif
		#include <GL/gl.h>
		#include <GL/glu.h>
		#include <GL/glut.h>
	#endif
#endif
