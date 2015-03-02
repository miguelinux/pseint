DIM a AS INTEGER, b AS INTEGER

DIM verdadero AS INTEGER ' realizar esta definiciones
DIM falso AS INTEGER
verdadero = -1 ' significa positivo
falso = 0
a = 11: b = 2.5 ' sentencias en una sola linea
c = verdadero
d = falso
DIM e AS STRING, f AS STRING
e = " Hola": f = " Mundo"
PRINT a, b, c, d, e, f


PRINT "algebraicos"
PRINT "a+b = ", a + b ' printa+b;
PRINT "a*b = ", a * b ' printa*b;
PRINT "a/b = ", a / b ' printa/b;
PRINT "a\b = ", a \ b '
PRINT "a-b = ", a - b 'printa-b;
PRINT "a^b = ", a ^ b 'printa^b; // potencia
PRINT "a mod b = ", a MOD 2 'printa%2; // resto de la división entera modulo

PRINT "relacionales (comparaciones)"
PRINT "a=b = ", a = b
PRINT "a<>b = ", a <> b '// distinto
PRINT "a<b = ", a < b
PRINT "a>b = ", a > b
PRINT "a<=b = ", a <= b
PRINT "a>=b = ", a >= b

PRINT "logicos"
PRINT "c and d = ", c AND d
PRINT "c or d = ", c OR d
PRINT "no (c and d)  = ", NOT (c AND d)

PRINT "para cadenas"
PRINT e + " " + f '// el + para cadenas las concatena
PRINT "e=f = ", e = f
PRINT "casa=CASA ", "casa" = "CASA"
PRINT "CASA=CASA ", "CASA" = "CASA"
PRINT "e<>f = ", e <> f
PRINT "e<f = ", e < f
PRINT "e>f = ", e > f
PRINT "e<=f = ", e <= f
PRINT "e>=f = ", e >= f

PRINT "abc" < "abcd"
PRINT "abc" = "abc"
PRINT "ABC" = "abc"
PRINT "abc" = "abcd"
PRINT "abcd" > "abc"

