#include "Package.h"
#include <wx/filename.h>
#include <wx/zipstrm.h>
#include <cstddef>
#include <wx/wfstream.h>
#include <iostream>
#include <wx/sstream.h>
#include <wx/dir.h>
#include <wx/textfile.h>
#include "version.h"
#include <wx/stream.h>
#include <wx/mstream.h>
#include <wx/fs_mem.h>
#include <wx/image.h>
#include <wx/filesys.h>

class mxFilterInputStream : public wxFilterInputStream {
	wxString key; int pos, len, base_pos;
public:
	mxFilterInputStream(wxInputStream &stream, const wxString &key) : wxFilterInputStream(stream) { 
		this->key=key; 
		len=key.size();
		pos=0; for(int i=0;i<len;i++) pos+=key[i]; 
		if (len) base_pos=pos=(pos%len); else base_pos=0;
	}
protected:
	size_t OnSysRead(void *buffer, size_t size) {
		size_t count = m_parent_i_stream->Read(buffer, size).LastRead();
		if (!len) return count;
		unsigned char *auxb = reinterpret_cast<unsigned char*>(buffer);
		for(unsigned int i=0;i<count;i++) { 
			int x = auxb[i]; 
			x = ( x + 256 - key[pos] ) % 256;
			pos=(pos+1)%len;
			auxb[i] = x;
		}
		return count;
	}
};

class mxFilterOutputStream : public wxFilterOutputStream {
	wxString key; int pos, len, base_pos;
public:
	mxFilterOutputStream(wxOutputStream &stream, const wxString &key) : wxFilterOutputStream(stream) { 
		this->key=key; 
		len=key.size();
		pos=0; for(int i=0;i<len;i++) pos+=key[i]; 
		if (len) base_pos=pos=(pos%len); else base_pos=0;
	}
protected:
	virtual wxOutputStream& Write(const void *buffer, size_t size) {
		if (!len) return m_parent_o_stream->Write(buffer, size);
		static size_t last_size=0; static unsigned char *auxbuf=NULL;
		if (last_size<size) {
			if (last_size) delete [] auxbuf;
			auxbuf=new unsigned char[size+1];
			last_size=size;
		}
		const unsigned char *ubuf = reinterpret_cast<const unsigned char*>(buffer);
		unsigned char *uaux = reinterpret_cast<unsigned char*>(auxbuf);
		for(unsigned int i=0;i<size;i++) { 
			int x = ubuf[i]; 
			x = ( x + key[pos] ) % 256;
			pos=(pos+1)%len;
			uaux[i] = x;
		}
		return m_parent_o_stream->Write(uaux, size);
	}
};

bool Package::Load (const wxString & fname, const wxString &passkey) {
	if (!wxFileName::FileExists(fname)) return false;
	wxZipEntry *entry = NULL;
	wxFileInputStream in_orig(fname);
	mxFilterInputStream in(in_orig,passkey);
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
		if (name.Lower().EndsWith(".png")) {
			wxMemoryOutputStream os;
			zip.Read(os);
#ifdef FOR_WXPSEINT
			images.Add(name);
			wxMemoryInputStream is(os);
			wxImage img(is,wxBITMAP_TYPE_PNG);
			wxMemoryFSHandler::AddFile(name,img,wxBITMAP_TYPE_PNG);
#endif
		} else {
			wxString content;
			wxStringOutputStream os(&content);
			zip.Read(os);
			ProcessFile(name,content);
		}
	}
	return is_ok=read_something;
}

bool Package::Load (const wxString &dir) {
	wxString mdir=dir; if (mdir.Last()=='\\'||mdir.Last()=='/') mdir.RemoveLast();
	Reset();
	wxDir wxdir(mdir);
	if ( wxdir.IsOpened() ) {
		wxString filename;
		bool cont = wxdir.GetFirst(&filename, "*" , wxDIR_FILES);
		while ( cont ) {
			if(filename.Lower().EndsWith(".png")) {
				images.Add(mdir+wxFileName::GetPathSeparator()+filename);
			} else {
				wxFile file(mdir+wxFileName::GetPathSeparator()+filename);
				wxString content; wxStringOutputStream sos(&content);
				wxFileInputStream fis(file); fis.Read(sos);
				ProcessFile(filename,content);
			}
			cont = wxdir.GetNext(&filename);
		}
	}
	return is_ok=true;
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
	} else if (name!="solution.psc") {
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
	Reset();
}

void Package::Reset ( ) {
	is_ok=false;
	help_text.Clear();
	tests.clear();
	base_psc.Clear();
	config.clear();
	images.Clear();
	SetConfigStr  ( "mensaje exito" , "El algoritmo es correcto" );
	SetConfigStr  ( "mensaje error" , "El algoritmo no es correcto" );
	SetConfigBool ( "mostrar soluciones" , true );
	SetConfigStr  ( "mostrar casos fallidos" , "primero" );
	SetConfigBool ( "mezclar casos", true );
	SetConfigInt  ( "version requerida" , PACKAGE_VERSION );
}

void Package::SetConfigStr (const wxString & key, const wxString & value) {
	config[key] = value;
}

void Package::SetConfigBool (const wxString & key, bool value) {
	config[key] = (value?"si":"no");
}

void Package::SetConfigInt (const wxString & key, long value) {
	config[key] = (wxString()<<value);
}

bool Package::SaveConfig (const wxString & fname) {
	wxTextFile fil(fname);
	if (fil.Exists()) fil.Open();
	else fil.Create();
	fil.Clear();
	std::map<wxString,wxString>::iterator it = config.begin();
	fil.AddLine(wxString("# generado por psEval ")<<VERSION<<"-"ARCHITECTURE);
	while (it!=config.end()) {
		fil.AddLine(wxString()+it->first+" = "+it->second);
		++it;
	}
	fil.Write();
	fil.Close();
	return true;
}

wxString Package::GetFullConfig() {
	SetConfigInt  ( "version requerida" , PACKAGE_VERSION );
	wxString ret = wxString("# generado por psEval ")<<VERSION<<"-"ARCHITECTURE<<"\n";
	std::map<wxString,wxString>::iterator it = config.begin();
	while (it!=config.end()) {
		ret<<wxString()<<it->first<<" = "<<it->second<<"\n";
		++it;
	}
	return ret;
}

bool Package::Save (const wxString & fname, const wxString & passkey) {
	wxFFileOutputStream out(fname);
	mxFilterOutputStream fout(out,passkey);
	wxZipOutputStream zip(fout);
	{
		zip.PutNextEntry("config.ini");
		wxString conf=GetFullConfig();
		wxStringInputStream is(conf);
		zip<<is;
	}
	if (!base_psc.IsEmpty()) {
		zip.PutNextEntry("base.psc");
		wxString conf=GetFullConfig();
		wxStringInputStream is(base_psc);
		zip<<is;
	}
	if (!base_psc.IsEmpty()) {
		zip.PutNextEntry("help.html");
		wxStringInputStream is(help_text);
		zip<<is;
	}
	for(std::map<wxString,TestCase>::iterator it = tests.begin();it!=tests.end();++it) {
		zip.PutNextEntry(wxString("input-")+it->first+".txt");
		wxStringInputStream is1(it->second.input);
		zip<<is1;
		zip.PutNextEntry(wxString("output-")+it->first+".txt");
		wxStringInputStream is2(it->second.solution);
		zip<<is2;
	}
	for(unsigned int i=0;i<images.GetCount();i++) {
		wxFileInputStream is(images[i]);
		zip.PutNextEntry(images[i].BeforeLast(wxFileName::GetPathSeparator()));
		zip<<is;
	}
	return true;
}

void Package::UnloadImages( ) {
	for(unsigned int i=0;i<images.GetCount();i++)
		wxMemoryFSHandler::RemoveFile(images[i]);
	images.Clear();
}

