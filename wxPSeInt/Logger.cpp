#include <wx/msgdlg.h>
#include <wx/datetime.h>
#include <wx/string.h>
#include "Logger.h"
#include "version.h"
#include "mxUtils.h"
#include "ConfigManager.h"

Logger *logger=NULL;

Logger::Logger(string where) {
	file.open(where.c_str(),ios::app);
	if (file.is_open()) logger=this; else wxMessageBox(wxString("No se pudo abrir el archivo de log: ")<<where.c_str());
	file<<endl<<"LOG START"<<endl;
	Write(wxString()<<"version "<<VERSION<<"-"<<ARCHITECTURE);
}

Logger::~Logger() {
	file<<"LOG END"<<endl<<endl;
	file.close();
}



void Logger::Write (const wxString & s) {
	file<<"   "<<wxDateTime::Now().FormatISOTime().c_str()<<"  "<<s.c_str()<<endl;
	file.flush();
}

void Logger::DumpVersions ( ) {
	Write("Logger::DumpVersions");
	Write(wxString()<<"   pseint: "<<utils->GetVersion(config->pseint_command));
	Write(wxString()<<"   psterm: "<<utils->GetVersion(config->psterm_command));
	Write(wxString()<<"   psdraw2: "<<utils->GetVersion(config->psdraw2_command));
	Write(wxString()<<"   psdrawe: "<<utils->GetVersion(config->psdrawe_command));
	Write(wxString()<<"   psexport: "<<utils->GetVersion(config->psexport_command));	
}

