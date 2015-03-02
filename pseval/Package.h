#ifndef PACKAGE_H
#define PACKAGE_H
#include <set>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <map>

/// struct para guardar datos de un caso de prueba
struct TestCase { 
	wxString input, output, solution;
};

/// carga y gestiona todos los datos de un ejercicio (todo lo que contiene el archivo de ejercicio)
class Package {
private:
	std::map<wxString,TestCase> tests; ///< mapea los nombres de los tests a sus entradas/salidas (de los pares de archivos input??.txt/output??.txt, donde ?? es el nombre)
	std::map<wxString,wxString> config; ///< palabras claves de config.txt, indican opciones de configuracion de como probar y mostrar los resultados
	wxString base_psc; ///< codigo inicial a mostrar en el editor
	wxString help_text; ///< texto de ayuda para el problema, opcional
	void ProcessFile(wxString name, wxString &content);
public:
	Package();
	bool Load(const wxString &fname, const wxString &passkey); ///< carga todos los datos del ejercicio a partir del archivo que recibe como argumento
	TestCase &GetTest(const wxString &name); ///< obtiene los datos de un caso a partir de su nombre (se asume que el nombre existe)
	int GetNames(wxArrayString &names); ///< colocar en el arreglo los nombres de todos los casos de prueba, y retorna la cantidad
	bool IsInConfig(const wxString &key); 
	wxString &GetConfigStr(const wxString &key); 
	bool GetConfigBool(const wxString &key); 
	long GetConfigInt(const wxString &key); 
	const wxString &GetBaseSrc() const { return base_psc; }
	const wxString &GetHelp() const { return help_text; }
};

#endif

