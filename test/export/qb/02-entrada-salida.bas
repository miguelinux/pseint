DIM a AS LONG ' definir a como entero;
DIM b AS SINGLE ' definir b como real;
DIM c AS STRING ' definir c como cadena;
DIM d AS INTEGER ' definir d como logico;
'// leer de a uno

INPUT "", a ' leer a; // Se requeire poner una cadena de entrada, de lo contrario introduce el caracter "?"
INPUT "", b
INPUT "", c
INPUT "", d

'// escribir de a uno
PRINT a
PRINT b
PRINT c
PRINT d

'// lecturas múltiples
' En QBASIC Lecturas multiples pulse [,] para entrar o separar cada uno de los datos"
INPUT "", a, b, c, d ' leer a,b,c,d;

'// escrituras múltiples separadas
' En QBASIC, para escribir en una sola linea sin saltar se separa los parametros con punto y coma ; y automaticamente agrega espacio de separacion
PRINT a; b; c; d ' equivalente a => Escribir a," " , b," " c," " d;
' otra opcion para escribir en una sola linea sin saltar es separando con comas, pero agrega muchos espacion pues tabula cada salida
PRINT a, b, c, d '

' 	// escritura sin salto de linea al final
' 	Escribir sin saltar "Esta linea no lleva enter al "
'   Escribir "al final"
PRINT "Esta linea no lleva enter al "; ' En QBASIC se pone ; para indicar que no salta
PRINT "final"
' FinProceso
