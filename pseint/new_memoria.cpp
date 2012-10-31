#include "new_memoria.h"
#include "new_funciones.h"

Memoria *memoria;
tipo_var vt_error(false,false,false);
tipo_var vt_desconocido(true,true,true);
tipo_var vt_logica(true,false,false);
tipo_var vt_numerica(false,true,false);
tipo_var vt_caracter(false,false,true);
tipo_var vt_caracter_o_numerica(false,true,true);
tipo_var vt_caracter_o_logica(true,true,true);
tipo_var vt_numerica_entera(false,true,false,true);
tipo_var vt_ningun_tipo(false,false,false);

bool Memoria::EsArgumento (string nom) {
	if (!funcion) return false;
	for(int i=1;i<=funcion->cant_arg;i++) 
		if (funcion->nombres[i]==nom) return true;
	return false;
}

