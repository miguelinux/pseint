#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <dlfcn.h>
using namespace std;

const char *binname="wxPSeInt.bin";

const char *my_getenv(const char *varname) {
	static char empty[]="";
	const char *res = getenv(varname);
	return res?res:empty;
}

void fix_argv(char *argv[]) {
	int n=strlen(argv[0])-1;
	char *argv0=new char[n+15];
	while (n>0&&argv[0][n]!='/') n--;
	if (argv[0][n]=='/') strncpy(argv0,argv[0],++n);
	strcpy(argv0+n,binname);
	argv[0]=argv0;
}


void FixMissingLibPNG12(string pseint_path) {
	void *handle = dlopen ("libpng12.so", RTLD_LAZY);
	if (!handle) {
		string ld_library_path = my_getenv("LD_LIBRARY_PATH");
		if (!ld_library_path.empty()) ld_library_path+=":";
		if (pseint_path.find('/')==string::npos) pseint_path="";
		else pseint_path=pseint_path.substr(0,pseint_path.rfind('/')+1);
		ld_library_path+=pseint_path+"libs";
		setenv("LD_LIBRARY_PATH",ld_library_path.c_str(),1);
	} else {
		dlclose(handle);
	}
}

int main(int argc, char *argv[]) {
	
	FixMissingLibPNG12(argv[0]);
	setenv("UBUNTU_MENUPROXY","",1);
	setenv("LIBOVERLAY_SCROLLBAR","0",1);
	setenv("LANG","C",1); // for fixing the problem with utf8 locale and ansi wx build
	setenv("LC_CTYPE","C",1); // for fixing the problem with utf8 locale and ansi wx build
	fix_argv(argv);
	pid_t pid=fork();
	if (pid==0) {
		execvp(argv[0],argv);
		// ver si es porque no tiene permiso de ejecución, en cuyo caso intentamos cambiarlos
		struct stat sb;
		if (stat(argv[0],&sb)!=-1 && !(sb.st_mode&S_IXUSR)) { 
			string cmd("chmod a+x \""); cmd+=argv[0]; cmd+="\"";
			system(cmd.c_str());
			execvp(argv[0],argv);
			cerr<<"Necesita darle permisos de ejecución al archivo wxPSeInt.bin"<<endl;
		}
	} else {
		int ret=0;
		waitpid(pid,&ret,0);
		if (ret) { // si algo falló en la ejecución, ver si faltan bibliotecas
			string s;
			s+="ldd \""; s+=argv[0]; s+="\""; 
			s+=" | grep \"not found\" >/dev/null";
			if (system(s.c_str())==0) {
				cerr<<endl<<endl;
				cerr<<"Puede que su sistema no tenga todas las bibliotecas necesarias para ejecutar PSeInt."<<endl;
				cerr<<"Instale las bibliotecas faltantes con el gestor de paquetes de su distribución."<<endl;
				cerr<<"Las bibliotecas faltantes son:"<<endl;
				string s; s+="ldd \""; s+=argv[0]; s+="\""; 
				s+=" | grep \"not found\" | cut -d . -f 1";
				system(s.c_str());
				cerr<<endl;
				cerr<<"Por ejemplo, si la biblioteca faltante es \"libpng12.so.0\", podría instalarla con:"<<endl;
				cerr<<"    En Debian/Ubuntu y derivados, el comando: \"sudo apt-get install libpng12-0\""<<endl;
				cerr<<"    En Arch Linux, el comando: \"sudo pacman -S libpng12\""<<endl;
				cerr<<"    En Fedora, el comando: \"yum install libpng12"<<endl;
				cerr<<"    En Suse/OpenSuse, el comando: \"zypper install libpng12-0\""<<endl;
			}
		}
		
	}
	return 0;
}

