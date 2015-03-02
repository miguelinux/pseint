
	DIM a AS INTEGER 'integer para rango entre -32768 a 32767
	a = -32768
	PRINT a

	a = -32769 ' se desborda por esta razon exportar los enteros como LONG y solo INTEGER para valores LOGICOS
	PRINT a

	DIM a1 AS LONG 'integer largo, valores >= abs(+/-32767)
	a1 = -32769
	PRINT a

	DIM a012345678901234567890123456789012345678 AS LONG  ' el nombre de la variable debe ser exportado maximo con 40 caracteres.
	a012345678901234567890123456789012345678 = 2147483647
	PRINT a012345678901234567890123456789012345678

	' En QBasic NO hace falta declarar las variables, pero es mejor exportar el tipo de dato
	' para detectar posibles malas llamadas por mala escritura

	INPUT "¿Cómo te llamas? ", nombre
	PRINT "hola "; nomber ´ escribio mal NOMBER
    ' Una vez que lo ejecutemos podría pasar esto:
    ' ¿Cómo te llamas? Juanma
    ' Hola ""
    ' No ha producido ningún error de Tiempo de Ejecución (No se ha detenido y ha terminado correctamente), pero no nos ha saludado con nuestro nombre!!!
    ' porque la variable a llamar en nomber en lugar de nombre, error que suele suceder constantemente y que es dificil de detectar

	DIM b AS SINGLE
    b = 2.5
    PRINT b

	DIM c AS INTEGER ' (falso o verdadero) en BASIC se debe tratar como entero
	c = 1 '//c<-verdadero;
	PRINT "true:"; c 
	
	DIM d AS INTEGER
	d = 0 'd<-Falso;
	PRINT "false:"; d 
	
	DIM e AS STRING // se requiere obligatoriamente la previa declaracion de la variable como cadena para luego asignarla
 	e = "Hola"  

	DIM f AS STRING
	f = "Mundo" 
	
	INPUT "Digite un dato cualquiera: ", p '// Leer un dato sin tipo predefinido. QBASIC asume como "single precision real number"
	PRINT p 
	PRINT p+8 ' prueba que lo asume como REAL, por que calcula la expresion y no lo toma como cadena
	'FinProceso
	
FinProceso