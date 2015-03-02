' PROGRAMACION MODULAR
' es mejor declarar las funciones como buena practica de programacion

DECLARE SUB PorReferencia(c as integer)
DECLARE SUB PorCopia(c as integer)
DECLARE SUB NoRetornaNada(a as integer, b as integer)
DECLARE FUNCTION FuncionDoble (b as integer)
DECLARE FUNCTION FuncionSinParametros
DECLARE SUB RecibeVector ( v(1 TO 10) as integer)
DECLARE SUB ReceivesMatrix ( m( 1 TO 3 , 1 TO 4) as integer)

PRINT FuncionDoble(5) ' Escribir FuncionDoble(5);
PRINT FuncionSinParametros ' si la funcion no
PRINT FuncionSinParametros2 ' si la funcion no
CALL NoRetornaNada(3, 9) ' Llamado a un procedimeiento se requiere de CALL // imprime dentro del procedimiento

DIM c AS INTEGER
c = 0 ' c<-0;
CALL PorReferencia(c) ' PorReferencia(c);
PRINT c 'Escribir c;

c = 0 ' c<-0;
CALL PorCopia(c) ' PorCopia(c);
PRINT c 'Escribir c;

DIM a(1 TO 10) AS INTEGER, b(1 TO 3, 1 TO 4) AS INTEGER ' en base uno ' Dimension a[10],b[3,4];


'//CALL RecibeVector(a()) ' Se requeire llamar el nombre de la variable con parentesis vacios cuando es de tipo arreglo
'//CALL RecibeMatriz(b())

DIM str1 AS STRING
str1 = "hola"
PRINT str1
CALL RecibeCadenaPorRef(str1)
PRINT str1
str1 = "hola"
PRINT str1
CALL RecibeCadenaPorCopia(str1)
PRINT str1
END

'FinProceso

FUNCTION FuncionDoble (b AS INTEGER) ' Funcion a<-FuncionDoble(b por copia)
b_copia = b ' truco para hacer una variable por copia por que por defecto en QBASIC son por referencia
FuncionDoble = 2 * b_copia ' a=2*b; // OJO: se requiere que la variable de salida de PSEINT se renombre con el mismo nombre de la funcion
END FUNCTION ' FinFuncion

FUNCTION FuncionSinParametros () ' puede eliminarse el ()
FuncionSinParametros = 50
END FUNCTION ' FinFuncion

FUNCTION FuncionSinParametros2 ()
FuncionSinParametros2 = 100
END FUNCTION ' FinFuncion

SUB NoRetornaNada (a AS INTEGER, b AS INTEGER) ' SubProceso NoRetornaNada(a,b)
a_copia = a ' truco para hacer una variable por copia por que por defecto en QBASIC son por referencia
b_copia = b
PRINT a_copia + b_copia ' escribir a+b;
END SUB ' FinSubProceso

SUB PorReferencia (b AS INTEGER) ' SubProceso PorReferencia(b por referencia)
b = 7
END SUB ' FinSubProceso

SUB PorCopia (c AS INTEGER) ' SubProceso PorCopia(c por copia)
c_copia = c ' truco para hacer una variable por copia
c_copia = 7
END SUB ' FinSubProceso

SUB RecibeVector (v( 1 TO 10) AS INTEGER) ' SubProceso RecibeVector(v)
' la anterior sentencia es igual a
' SUB RecibeVector (v( ) AS INTEGER) ' sin especificar le tamano
INPUT "v(1)=", v(1) ' Leer v[1];
PRINT "v(1)="; v(1)
INPUT "v(10)=", v(10) ' Leer v[1];
PRINT "v(10)="; v(10)
END SUB ' FinSubProceso

SUB RecibeMatriz (m( 1 TO 3 , 1 TO 4) AS INTEGER) ' SubProceso RecibeMatriz(m)
INPUT "m(1,1)=", m(1, 1) ' Leer m[1,1];
END SUB ' FinSubProceso

SUB RecibeCadenaPorRef(s AS STRING)
s = "world"
PRINT s
END SUB

SUB RecibeCadenaPorCopia (s AS STRING)
DIM s_copia AS STRING
s_copia = s
s_copia = "world"
PRINT s_copia
END SUB