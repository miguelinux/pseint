"ABRA ESTOS EJEMPLOS DESDE LA AYUDA ( MENU AYUDA -> EJEMPLOS ) de otra forma no podrá ejecutarlos"

// Calcula el promedio de una lista de N datos utilizando un SubProceso

SubProceso prom <- Promedio ( arreglo, cantidad )
	{Definir i como Entero}{;}
	{Definir suma como Real}{;}
	suma <- 0{;}
	Para i<-1 Hasta cantidad Hacer
		 suma <- suma + arreglo[i]{;}
	FinPara
	prom <- suma/cantidad{;}
FinSubProceso

Proceso Principal

	{Definir i,N como Entero}{;}
	{Definir acum,datos,prom como Reales}{;}
	Dimension datos[100]{;}
	Escribir "Ingrese la cantidad de datos:"{;}
	Leer n{;}
	
	Para i<-1 Hasta n Hacer
		Escribir "Ingrese el dato ",i,":"{;}
		Leer datos[i]{;}
	FinPara
	
	Escribir "El promedio es: ",Promedio(datos,n){;}
	
FinProceso
