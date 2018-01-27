#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include "mxOpersWindow.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include <wx/tooltip.h>
#include "ConfigManager.h"
#include <wx/utils.h>
#include <wx/statline.h>
#include <wx/settings.h>

mxOpersWindow *opers_window = NULL;

BEGIN_EVENT_TABLE(mxOpersWindow,wxScrolledWindow)
	EVT_COMMAND_RANGE(mxID_LAST,mxID_LAST+200,wxEVT_COMMAND_BUTTON_CLICKED,mxOpersWindow::OnItem)
	EVT_COMMAND_RANGE(mxID_LAST+250,mxID_LAST+500,wxEVT_COMMAND_BUTTON_CLICKED,mxOpersWindow::OnLabel)
END_EVENT_TABLE()
	
mxOpersWindow::mxOpersWindow(wxWindow *parent):wxScrolledWindow(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize) {
	sizer=new wxBoxSizer(wxVERTICAL);
	
	AddCaterory("Op. Algebraicos");
	Add("+","+","suma","Ej: A+B");
	Add("-","-","resta","Ej: A-B");
	Add("*","*","multiplicación","Ej: A*B");
	Add("/","/","división","Ej: A/B");
	Add("^","^","potencia","Ej: A^N (A elevado a la N-esima potencia)");
	Add("%","%","resto/modulo","A%B (resto de dividir A por B)");
	AddCaterory("Op. Lógicos");
	Add("&","&&","conjunción","Ej: X>5 Y X<20 (Verdadero solo si ambas son Verdadero)");
	Add("|","|","disyunción","Ej: X=0 O Z=5 (Verdadero si al menos una de las dos es Verdadero)");
	Add("~","~","negación","Ej: NO X=5 (Invierte el resultado de la expresión lógica)");
	AddCaterory("Op. Relacionales");
	Add("=","=","igual","Ej: A=B");
	Add("<>","<>","distinto","Ej: A<>B");
	Add("!=","!=","distinto","Ej: A!=B (equivalente a A<>B)");
	Add("<","<","menor","Ej: A<B");
	Add("<=","<=","menor o igual","Ej: A<=B");
	Add(">",">","mayor","Ej: A>B");
	Add(">=",">=","mayor o igual","Ej: A>=B");
	AddCaterory("Func. Matemáticas");
	Add("abs({expresion_numerica})","abs","valor absoluto","abs(X) calcula el valor absoluto de x");
	Add("trunc({expresion_numerica})","trunc","valor truncado","trunc(X) devuelve la parte entera de X");
	Add("redon({expresion_numerica})","redon","valor redondeado","redon(X) redondea X al entero más cercano");
	Add("rc({expr_num_no_negativa})","rc","raiz cuadrada","rx(X) calcula la raiz cuadrada de X");
	Add("sen({angulo_en_radianes})","sen","seno","sen(X) calcula el seno del angulo X (en radianes)");
	Add("cos({angulo_en_radianes})","cos","coseno","cos(X) calcula el coseno del angulo X (en radianes)");
	Add("tan({angulo_en_radianes})","tan","tangente","tan(X) calcula la tangente del angulo X (en radianes)");
	Add("asen({expresion_numerica})","asen","arcoseno","asen(X) calcula el arcoseno (en radianes) de X");
	Add("acos({expresion_numerica})","acos","arcocoseno","acos(X) calcula el arcocoseno (en radianes) de X");
	Add("atan({expresion_numerica})","atan","arcotangente","atan(X) calcula el arcotangenta (en radianes) de X");
	Add("ln({expr_numerica_positiva})","ln","logaritmo natural","ln(X) calcula el logaritmo natural (base e) de X");
	Add("exp({expresion_numerica})","exp","func. exponencial","exp(X) calcula la función exponencial e^X");
	Add("azar({expr_num_entera_positiva})","azar","numero aleatorio","Ej: Azar(N) retorna un numero aleatorio entre 0 y N-1");
	if (cfg_lang[LS_ENABLE_STRING_FUNCTIONS]) {
		AddCaterory("Func. p/Cadenas");
		Add("Longitud({cadena})","Longitud","","Longutid(S) retorna la cantidad de caracteres de la cadena S");
		Add("SubCadena({cadena},{desde},{hasta})","SubCadena","","R<-Subcadena(S,I,J) retorna en R la cadena formada por los caracteres de la cadena S desde la posición I a la J inclusive");
		Add("Concatenar({cadena},{cadena})","Concatenar","","R<-Concatenar(S1,S2) retorna en R una sola cadena con los contenidos unidos de S1 y S2");
		Add("ConvertirANumero({cadena})","ConvertirANumero","","S<-ConvertirANumero(N) recibe un valor/expresión numérico N, y retorna su equivalente de tipo cadena de caracteres en S");
		Add("ConvertirATexto({numero})","ConvertirATexto","","N<-ConvertirATexto(S) recibe una variable o expresión S de tipo cadena de caracteres que contiene un valor numérico, y obtiene ne N su equivalente de tipo numérico");
		Add("Mayusculas({cadena})","Mayusculas","","MAY<-Mayusculas(S) retorna en MAY el contenido de la cadena S con sus letras minusculas reemplazadas por mayusculas");
		Add("Minusculas({cadena})","Minusculas","","MIN<-Minusculas(S) retorna en MIN el contenido de la cadena S con sus letras mayusculas reemplazadas por minusculas");
	}
	AddCaterory("Constantes");
	Add("PI","PI","","Equivale al valor de la constante (3.1415926...)");
	Finish();
	
	SetWordOperators();
	
	SetScrollRate(0,10);
	SetSizerAndFit(sizer);
}

void mxOpersWindow::Add (wxString code, wxString label, wxString name, wxString desc) {
	if (name.Len()) label<<" ("<<name<<")";
	wxButton *but=new wxButton(this,mxID_LAST+lista.size(),label,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
	sizer->Add(but,wxSizerFlags().Border(wxLEFT,20).Expand());
	but->SetToolTip(utils->FixTooltip(desc));
	lista.push_back(oper_item(code,but));
}

void mxOpersWindow::Replace(oper_item &o, wxString f1, wxString t1, wxString f2, wxString t2, bool sent) {
	if (o.code==(sent?f1:t1)) o.code=(sent?t1:f1);
	if (o.but->GetToolTip() && o.but->GetToolTip()->GetTip().Contains(sent?f2:t2)) {
		wxString s=o.but->GetToolTip()->GetTip(); 
		s.Replace(sent?f2:t2,sent?t2:f2);
		o.but->SetToolTip(s);
	}
	if (f1=="&") f1="&&";
	if (o.but->GetLabel().StartsWith(sent?f1:t1)) {
		wxString s=o.but->GetLabel();
		s.Replace(sent?f1:t1,sent?t1:f1,false);
		o.but->SetLabel(s);
	}
}

void mxOpersWindow::SetWordOperators ( ) {
	for(unsigned int i=0;i<lista.size();i++) {  
		Replace(lista[i],"&","Y"," & "," Y ",cfg_lang[LS_WORD_OPERATORS]);
		Replace(lista[i],"|","O"," | "," O ",cfg_lang[LS_WORD_OPERATORS]);
		Replace(lista[i],"~","NO","~ ","NO ",cfg_lang[LS_WORD_OPERATORS]);
		Replace(lista[i],"%","MOD","%"," MOD ",cfg_lang[LS_WORD_OPERATORS]);
	}
}

void mxOpersWindow::AddCaterory (wxString label) {
	if (!lista2.empty()) lista2.back().to=lista.size();
	label_item i(label,new wxButton(this,mxID_LAST+250+lista2.size(),wxString("[-]  ")+label,wxDefaultPosition,wxDefaultSize,wxNO_BORDER|wxBU_EXACTFIT
#ifndef __WIN32__
		|wxBU_LEFT
#endif
		));
	i.from=lista.size();
#ifdef __WIN32__
	// por alguna razon y contrariamente a lo que dice la documentacion en windows wx 2.8 no toma el wxNO_BORDER ni wxBORDER_NONE
	wxBoxSizer *sizer_aux=new wxBoxSizer(wxHORIZONTAL);
	sizer_aux->Add(i.but,wxSizerFlags().Center());	
	sizer_aux->Add(new wxStaticLine(this),wxSizerFlags().Proportion(1).Center());	
	sizer->Add(sizer_aux,wxSizerFlags().Proportion(0).Expand().Border(wxTOP,5));	
#else
	sizer->Add(i.but,wxSizerFlags().Proportion(0).Expand().Border(wxTOP,5));	
#endif
	lista2.push_back(i);
}

void mxOpersWindow::Finish ( ) {
	if (!lista2.empty()) lista2.back().to=lista.size();
	sizer->Add(new wxStaticText(this,wxID_ANY,""));
	sizer->Add(new wxStaticText(this,wxID_ANY,"Haga click en un operador\no en una función para\ninsertarlo/a en expresiones\ndel pseudocódigo."));
}


void mxOpersWindow::OnItem (wxCommandEvent & evt) {
	int i=evt.GetId()-mxID_LAST;
	main_window->InsertCode(lista[i].code);
}


void mxOpersWindow::OnLabel (wxCommandEvent & evt) {
	int i=evt.GetId()-mxID_LAST-250;
	for(int j=lista2[i].from;j<lista2[i].to;j++) {
		if (lista2[i].showing) lista[j].but->Hide();
		else lista[j].but->Show();
		if (!config->animate_gui) continue;
		sizer->Layout();
		FitInside();
		wxYield();
		wxMilliSleep(15);
	}
	lista2[i].showing=!lista2[i].showing;
	if (lista2[i].showing) lista2[i].but->SetLabel(wxString("[-]  ")+lista2[i].label);
	else lista2[i].but->SetLabel(wxString("[+] ")+lista2[i].label+" (...)");
	sizer->Layout();
	FitInside();
}
