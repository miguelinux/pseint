#include <wx/msgdlg.h>
#include <wx/datetime.h>
#include <wx/string.h>
#include "Logger.h"
#include "version.h"

Logger *logger=NULL;

Logger::Logger(string where) {
	file.open(where.c_str(),ios::app);
	if (file.is_open()) logger=this; else wxMessageBox(wxString("No se pudo abrir el archivo de log: ")<<where.c_str());
	file<<endl<<"LOG START"<<endl;
	file<<"   version "<<VERSION<<endl;
	file<<"   date "<<wxDateTime::Now().FormatISODate().c_str()<<" "<<wxDateTime::Now().FormatISOTime().c_str()<<endl;
	file.flush();
}

Logger::~Logger() {
	file<<endl<<"LOG END"<<endl;
	file.close();
}

void Logger::Write (const wxString & s) {
	file<<"   "<<wxDateTime::Now().FormatISOTime().c_str()<<"  "<<s.c_str()<<endl;
	file.flush();
}
