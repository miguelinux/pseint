#ifndef EXPORT_HTML_H
#define EXPORT_HTML_H
#include "export_javascript.h"

class HTMLExporter : public JavaScriptExporter {
public:
	HTMLExporter();
	void translate(t_output &out, t_programa &prog);
};

#endif
