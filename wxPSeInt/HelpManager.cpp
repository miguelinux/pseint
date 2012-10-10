#include "HelpManager.h"
#include <wx/textfile.h>
#include "ConfigManager.h"

HelpManager *help;

HelpManager::HelpManager() {
	LoadErrors();
	LoadCommands();
}

void HelpManager::LoadCommands() {
	wxTextFile fil(DIR_PLUS_FILE(config->help_dir,_T("comandos.hlp")));
	if (!fil.Exists()) return;
	fil.Open();
	wxString command, help_text;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		if (str[0]==' ') {
			help_text<<str;//<<_T("<br>");
		} else {
			commands[command] = help_text;
			help_text.Clear();
			command=str;
		}
	}
	commands[command] = help_text;
	fil.Close();
}

void HelpManager::LoadErrors() {
	wxTextFile fil(DIR_PLUS_FILE(config->help_dir,_T("errores.hlp")));
	if (!fil.Exists()) return;
	fil.Open();
	wxString command(_T("0")), help_text;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		if (str[0]==' ') {
			help_text<<str<<_T("<br>");
		} else {
			long e=0;
			command.ToLong(&e);
			errors[e] = help_text;
			help_text.Clear();
			command=str;
		}
	}
	long e=0;
	command.ToLong(&e);
	errors[e] = help_text;
	fil.Close();
}

wxString HelpManager::GetErrorText(wxString text, int num) {
	wxString ret(_T("<HTML><HEAD><TITLE>PSeInt QuickHelp</TITLE></HEAD><BODY><B>"));
	if (num>=MAX_ERRORS_TXT || errors[num].Len()==0) 
		ret<<utils->ToHtml(text)<<_T("</B><BR><BR> No se encontro descripcion para este error <BR><BR></BODY></HTML>");
	else
		ret<<utils->ToHtml(text)<<_T("</B><BR><BR>")<<errors[num]<<_T("<BR><BR></BODY></HTML>");
	return ret;
}

wxString HelpManager::GetCommandText(wxString com) {
	wxString ret(_T("<HTML><HEAD><TITLE>PSeInt QuickHelp</TITLE></HEAD><BODY><B>"));
	ret<<com<<_T("</B><BR><BR>")<<commands[com]<<_T("<BR><BR></BODY></HTML>");
	return ret;
}

HelpManager::~HelpManager() {
	
}

wxString HelpManager::GetQuickHelp(wxString key) {
	wxString str=_T("nohelp.html");
	key.MakeLower();
	if (key==_("leer")) str=_T("lectura.html");
	else if (key==_("lectura")) str=_T("lectura.html");
	else if (key==_("mayusculas")) str=_T("funciones.html");
	else if (key==_("mayúsculas")) str=_T("funciones.html");
	else if (key==_("minusculas")) str=_T("funciones.html");
	else if (key==_("minúsculas")) str=_T("funciones.html");
	else if (key==_("longitud")) str=_T("funciones.html");
	else if (key==_("subcadena")) str=_T("funciones.html");
	else if (key==_("concatenar")) str=_T("funciones.html");
	else if (key==_("subproceso")) str=_T("subprocesos.html");
	else if (key==_("subprocesos")) str=_T("subprocesos.html");
	else if (key==_("función")) str=_T("subprocesos.html");
	else if (key==_("funcion")) str=_T("subprocesos.html");
	else if (key==_("sin")) str=_T("escritura.html");
	else if (key==_("saltar")) str=_T("escritura.html");
	else if (key==_("bajar")) str=_T("escritura.html");
	else if (key==_("sinbajar")) str=_T("escritura.html");
	else if (key==_("sinsaltar")) str=_T("escritura.html");
	else if (key==_("mostrar")) str=_T("escritura.html");
	else if (key==_("imprimir")) str=_T("escritura.html");
	else if (key==_("escribir")) str=_T("escritura.html");
	else if (key==_("escritura")) str=_T("escritura.html");
	else if (key==_("dimension")) str=_T("arreglos.html");
	else if (key==_("dimensión")) str=_T("arreglos.html");
	else if (key==_("matriz")) str=_T("arreglos.html");
	else if (key==_("matrices")) str=_T("arreglos.html");
	else if (key==_("arreglos")) str=_T("arreglos.html");
	else if (key==_("arreglo")) str=_T("arreglos.html");
	else if (key==_("vector")) str=_T("arreglos.html");
	else if (key==_("vectores")) str=_T("arreglos.html");
	else if (key==_("definir")) str=_T("definir.html");
	else if (key==_("definición")) str=_T("definir.html");
	else if (key==_("definicion")) str=_T("definir.html");
	else if (key==_("como")) str=_T("definir.html");
	else if (key==_("numérico")) str=_T("tipos.html");
	else if (key==_("numérica")) str=_T("tipos.html");
	else if (key==_("numero")) str=_T("tipos.html");
	else if (key==_("número")) str=_T("tipos.html");
	else if (key==_("logica")) str=_T("tipos.html");
	else if (key==_("logico")) str=_T("tipos.html");
	else if (key==_("cadena")) str=_T("tipos.html");
	else if (key==_("texto")) str=_T("tipos.html");
	else if (key==_("caracter")) str=_T("tipos.html");
	else if (key==_("si")) str=_T("si.html");
	else if (key==_("finsi")) str=_T("si.html");
	else if (key==_("entonces")) str=_T("si.html");
	else if (key==_("sino")) str=_T("si.html");
	else if (key==_("asignar")) str=_T("asignacion.html");
	else if (key==_("asignacion")) str=_T("asignacion.html");
	else if (key==_("repetir")) str=_T("repetir.html");
	else if (key==_("paso")) str=_T("para.html");
	else if (key==_("finpara")) str=_T("para.html");
	else if (key==_("para")) str=_T("para.html");
	else if (key==_("cada")) str=_T("para.html");
	else if (key==_("en")) str=_T("para.html");
	else if (key==_("de")) str=_T("para.html");
	else if (key==_("desde")) str=_T("para.html");
	else if (key==_("paso")) str=_T("para.html");
	else if (key==_("hasta")) str=_T("para.html");
	else if (key==_("opcion")) str=_T("segun.html");
	else if (key==_("caso")) str=_T("segun.html");
	else if (key==_("segun")) str=_T("segun.html");
	else if (key==_("finsegun")) str=_T("segun.html");
	else if (key==_("según")) str=_T("segun.html");
	else if (key==_("finsegún")) str=_T("segun.html");
	else if (key==_("finmientras")) str=_T("mientras.html");
	else if (key==_("mientras")) str=_T("mientras.html");
	else if (key==_("proceso")) str=_T("forma.html");
	else if (key==_("finproceso")) str=_T("forma.html");
	else if (key==_("vectores")) str=_T("arreglos.html");
	else if (key==_("operador")) str=_T("operadores.html");
	else if (key==_("operadores")) str=_T("operadores.html");
	else if (key==_("funciones")) str=_T("funciones.html");
	else if (key==_("sen")) str=_T("funciones.html");
	else if (key==_("asen")) str=_T("funciones.html");
	else if (key==_("cos")) str=_T("funciones.html");
	else if (key==_("acos")) str=_T("funciones.html");
	else if (key==_("tan")) str=_T("funciones.html");
	else if (key==_("atan")) str=_T("funciones.html");
	else if (key==_("azar")) str=_T("funciones.html");
	else if (key==_("trunc")) str=_T("funciones.html");
	else if (key==_("rc")) str=_T("funciones.html");
	else if (key==_("ln")) str=_T("funciones.html");
	else if (key==_("redon")) str=_T("funciones.html");
	else if (key==_("abs")) str=_T("funciones.html");
	else if (key==_("hacer")) str=_T("hacer.html");
	else if (key==_("tipos")) str=_T("tipos.html");
	else if (key==_("tipo")) str=_T("tipos.html");
	else if (key==_("otro")) str=_T("segun.html");
	else if (key==_("modo")) str=_T("segun.html");
	else if (key==_("sies")) str=_T("segun.html");
	else if (key==_("case")) str=_T("segun.html");
	else if (key==_("opcion")) str=_T("segun.html");
	else if (key==_("mod")) str=_T("operadores.html");
	else if (key==_("divisible")) str=_T("colcond.html");
	else if (key==_("multiplo")) str=_T("colcond.html");
	else if (key==_("positivo")) str=_T("colcond.html");
	else if (key==_("negativo")) str=_T("colcond.html");
	else if (key==_("impar")) str=_T("colcond.html");
	else if (key==_("par")) str=_T("colcond.html");
	else if (key==_("cero")) str=_T("colcond.html");
	else if (key==_("distinto")) str=_T("colcond.html");
	else if (key==_("igual")) str=_T("colcond.html");
	else if (key==_("es")) str=_T("colcond.html");
	else if (key==_("menor")) str=_T("colcond.html");
	else if (key==_("mayor")) str=_T("colcond.html");
	else if (key==_("esperartecla")) str=_T("secotras.html");
	else if (key==_("tecla")) str=_T("secotras.html");
	else if (key==_("esperar")) str=_T("secotras.html");
	else if (key==_("limpiar")) str=_T("secotras.html");
	else if (key==_("limpiarpantalla")) str=_T("secotras.html");
	else if (key==_("borrar")) str=_T("secotras.html");
	else if (key==_("borrarpantalla")) str=_T("secotras.html");
	else if (key==_("pantalla")) str=_T("secotras.html");
	else if (key==_("variable")) str=_T("variables.html");
	else if (key==_("variables")) str=_T("variables.html");
	else if (key==_("ejemplo")) str=_T("ejemplos.html");
	else if (key==_("ejemplos")) str=_T("ejemplos.html");
	return DIR_PLUS_FILE(config->help_dir,str);	
}
