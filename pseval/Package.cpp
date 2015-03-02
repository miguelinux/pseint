#include "Package.h"
#include <wx/filename.h>
#include <wx/zipstrm.h>
#include <cstddef>
#include <wx/wfstream.h>
#include <iostream>
#include <wx/sstream.h>

class mxFilterStream : public wxFilterInputStream {
	wxString key; int pos, len;
public:
	mxFilterStream(wxInputStream &stream, const wxString &key) : wxFilterInputStream(stream) { this->key=key; pos=len=key.size(); }
protected:
	size_t OnSysRead(void *buffer, size_t size) {
		size_t count = m_parent_i_stream->Read(buffer, size).LastRead();
		if (!len) return count;
		unsigned char *auxb = reinterpret_cast<unsigned char*>(buffer);
		for(unsigned int i=0;i<count;i++) { 
			int x = auxb[i]; 
			x = ( x + 256 - key[(pos++)%len] ) % 256;
			auxb[i] = x;
		}
		return count;
	}
};

bool Package::Load (const wxString & fname, const wxString &passkey) {
	if (!wxFileName::FileExists(fname)) return false;
	wxZipEntry *entry = NULL;
	wxFileInputStream in_orig(fname);
	mxFilterStream in(in_orig,passkey);
	if (!in) return false;
	wxZipInputStream zip(in);
	bool read_something=false;
	while ((entry=zip.GetNextEntry())) {
		read_something=true;
		wxString name = entry->GetName();
		if (entry->IsDir()) {
			std::cerr<<"ERROR in Package::Load: Found directory "<<name<<std::endl;
			continue;
		}
		zip.OpenEntry(*entry);
		if (!zip.CanRead()) return false;
		wxString content;
		wxStringOutputStream os(&content);
		zip.Read(os);
		ProcessFile(name,content);
	}
	return read_something;
}

TestCase & Package::GetTest (const wxString &name) {
	return tests.find(name)->second;
}

int Package::GetNames (wxArrayString & names) {
	names.Clear();
	for(std::map<wxString,TestCase>::iterator it = tests.begin();it!=tests.end();++it)
		names.Add(it->first);
	return names.GetCount();
}

void Package::ProcessFile (wxString name, wxString &content) {
	name.MakeLower();
	if (name=="base.psc") {
		base_psc = content;
	} else if (name=="help.html") {
		help_text = content;
	} else if (name=="config.ini") {
		content.Replace("\r","",true);
		content+="\n";
		while (content.Contains('\n')) {
			wxString key=content.BeforeFirst('\n');
			content=content.AfterFirst('\n');
			if (!key.IsEmpty() && !key.StartsWith("#") && key.Contains("=")) 
				config[key.BeforeFirst('=').Trim(true).Trim(false).Lower()]=key.AfterFirst('=').Trim(true).Trim(false);
		}
	} else if (name.StartsWith("input") && name.EndsWith(".txt")) {
		wxString test_name = name.Mid(5,name.Len()-9);
		if (test_name.StartsWith("_")||test_name.StartsWith("-")) test_name=test_name.Mid(1);
		tests[test_name].input=content;
	} else if (name.StartsWith("output") && name.EndsWith(".txt")) {
		wxString test_name = name.Mid(6,name.Len()-10);
		if (test_name.StartsWith("_")||test_name.StartsWith("-")) test_name=test_name.Mid(1);
		tests[test_name].solution=content;
	} else {
		std::cerr<<"Error in Package::ProcessFile: Unknown filename "<<name<<std::endl;
	}
}

bool Package::IsInConfig (const wxString & key) {
	return config.find(key.Lower())!=config.end();
}

wxString &Package::GetConfigStr(const wxString &key) {
	return config[key.Lower()];
}

long Package::GetConfigInt(const wxString & key) {
	long retval; 
	if (!GetConfigStr(key).ToLong(&retval)) return 0;
	return retval;
}

bool Package::GetConfigBool(const wxString & key) {
	wxString val = GetConfigStr(key).Lower();
	return val=="si"||val=="1"||val=="verdadero"||val=="yes"||val=="true"||val=="sí";
}

Package::Package ( ) {
	config["mensaje_exito"] = "El algoritmo es correcto";
	config["mensaje_error"] = "El algoritmo no es correcto";
	config["mostrar_soluciones"] = "si";
	config["mostrar_casos_fallidos"] = "primero";
	config["mezclar_casos"] = "si";
}

