#ifndef COMMONPARSINGFUNCTIONS_H
#define COMMONPARSINGFUNCTIONS_H
#include <wx/string.h>

template<typename TChar>
inline bool EsLetra(const TChar &c, bool incluir_nros=true) {
	return 
		c=='_'||
		((c|32)>='a'&&(c|32)<='z')||
		c=='á'||c=='Á'||c=='é'||c=='É'||
		c=='ó'||c=='Ó'||c=='í'||c=='Í'||
		c=='ú'||c=='Ú'||c=='ñ'||c=='Ñ'||
		(incluir_nros&&c>='0'&&c<='9');
}

#if wxABI_VERSION>=30000
template<>
inline bool EsLetra(const wxUniCharRef &c, bool incluir_nros) {
	return EsLetra(c.GetValue(),incluir_nros);
}
template<>
inline bool EsLetra(const wxUniChar &c, bool incluir_nros) {
	return EsLetra(c.GetValue(),incluir_nros);
}
#endif

template<typename TString>
int SkipWhite(const TString &s, int i, int l) {
	while (i<l && (s[i]==' '||s[i]=='\t')) i++;
	return i;
}

template<typename TString>
inline int SkipWord(const TString &s, int i, int l) {
	while (i<l && EsLetra(s[i],true)) i++;
	return i;
}

template<typename TString>
void MakeUpper(TString &word) {
	for(unsigned int i=0;i<word.size();i++) { 
		if (word[i]>='A'||word[i]<='Z') word[i] = toupper(word[i]);
		else if (word[i]>='0'||word[i]<='9') continue;
		else if (word[i]=='á') word[i]='Á';
		else if (word[i]=='é') word[i]='É';
		else if (word[i]=='í') word[i]='Í';
		else if (word[i]=='ó') word[i]='Ó';
		else if (word[i]=='ú') word[i]='Ú';
		else if (word[i]=='ü') word[i]='Ü';
		else if (word[i]=='ñ') word[i]='Ñ';
	}
}

template<typename TString>
TString Upper(TString word) {
	for(unsigned int i=0;i<word.size();i++) { 
		if (word[i]>='A'||word[i]<='Z') word[i]=toupper(word[i]);
		else if (word[i]>='0'||word[i]<='9') continue;
		else if (word[i]=='á') word[i]='Á';
		else if (word[i]=='é') word[i]='É';
		else if (word[i]=='í') word[i]='Í';
		else if (word[i]=='ó') word[i]='Ó';
		else if (word[i]=='ú') word[i]='Ú';
		else if (word[i]=='ü') word[i]='Ü';
		else if (word[i]=='ñ') word[i]='Ñ';
	}
	return word;
}

template<typename TString> 
int SkipString(const TString &line, int i, int len) {
	do {
		++i;
	} while(i<len && line[i]!='\'' && line[i]!='\"');
	return i;
}

template<typename TString> 
int SkipParentesis(const TString &line, int i, int len) {
	int par_level = 0;
	do {
		if (line[i]=='(' || line[i]=='[') ++par_level;
		else if (line[i]==']' || line[i]==')') --par_level;
		else if (line[i]=='\'' || line[i]=='\"') i = SkipString(line,i,len);
		++i;
	} while(i<len && par_level>0);
	return i;
}

inline wxString GetFirstWord(wxString s) {
	int l = s.Len();
	int i0 = SkipWhite(s,0,l);
	int i1 = SkipWord(s,i0,l);
	if (i1>i0) {
		s = s.Mid(i0,i1-i0);
		MakeUpper(s);
	} else {
		s = "";
	}
	return s;
}

#endif
