#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <fstream>
using namespace std;

class Logger {
private:
	ofstream file;
protected:
public:
	Logger(string where);
	void Write(const wxString &s);
	~Logger();
};

inline wxString &operator<<(wxString &s, void *p) { return s<<reinterpret_cast<unsigned long>(p); }

extern Logger *logger;
#ifdef DEBUG
	#define _LOG(x) cerr<<x<<endl; if (logger) logger->Write(wxString()<<x)
#else
	#define _LOG(x) if (logger) logger->Write(wxString()<<x)
#endif

#endif

