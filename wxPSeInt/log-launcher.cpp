#include <iostream>
#include <shlobj.h>
using namespace std;

int main(int argc, char *argv[]) {
	LPITEMIDLIST pidl;
	if (SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&pidl)!=NOERROR) return 1;
	char buf[MAX_PATH];
	SHGetPathFromIDList(pidl,buf);
	string s(buf);
	if (!s.size()) return 2;
	if (s[s.size()-1]!='\\') s+="\\";
	s+="pseint-log.txt";
	system((string("wxPSeint --logger \"")+s+"\"").c_str());
	return 0;
}

