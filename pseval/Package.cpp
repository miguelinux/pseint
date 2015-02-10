#include "Package.h"
#include <wx/filename.h>
#include <wx/zipstrm.h>
#include <cstddef>
#include <wx/wfstream.h>
#include <iostream>
#include <wx/sstream.h>


bool Package::Load (const wxString & fname) {
	if (!wxFileName::FileExists(fname)) return false;
	wxZipEntry *entry = NULL;
	wxFileInputStream in(fname);
	if (!in) return false;
	wxZipInputStream zip(in);
	while ((entry=zip.GetNextEntry())) {
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
	return true;
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
	} else if (name=="config.txt") {
		content.Replace("\r","",true);
		content.MakeLower(); content+="\n";
		while (content.Contains('\n')) {
			wxString key=content.BeforeFirst('\n');
			if (!key.IsEmpty()) config.insert(key);
			content=content.AfterFirst('\n');
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
	return config.find(key)!=config.end();
}

