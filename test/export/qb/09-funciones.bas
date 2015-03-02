'// EN PSEINT: ejemplo#09: // funciones basicas, trigonometricas, redondeo

CONST PI = 3.141593
x = 1.5
PRINT SQR(x) ' Escribir rc(x); // raiz cuadrada // 1.224744871
PRINT ABS(x) ' Escribir abs(x); // valor absoluto // 1.5
PRINT ABS(5) ' Se requeire suprimir el prefijo + si lo hay 'Escribir abs(+5); // 5
PRINT ABS(-5) 'Escribir abs(-5); // 5
PRINT ABS(5.0) 'Escribir abs(-5.); //5
PRINT LOG(x) ' Escribir ln(x); // logaritmo natural // 0.4054651081
PRINT EXP(x) ' Escribir exp(x); // exponencial, e^x

PRINT "pi/2 =", PI / 2
PRINT "sin(pi/2) =", SIN(PI / 2)
PRINT "ARCSIN(SIN(pi / 2))", ARCSIN(SIN(PI / 2)) ' Escribir asen(x);
PRINT "ARCSIN(1) =", ARCSIN(1)
PRINT "ARCSIN(1.1) =", ARCSIN(1.1)

PRINT "SIN(90) =", SIN(90)
PRINT "ARCSIN(SIN(90)) =", ARCSIN(SIN(90)) ' Escribir asen(x);

PRINT COS(PI / 2) ' Escribir cos(x); // coseno
PRINT TAN(PI / 2) ' Escribir tan(x); // tangente
PRINT ATN(PI / 2) ' Escribir atan(x); // arco-tangente

PRINT "REDONDEAR" ' Redondea a entero corto
PRINT CINT(1.5) ' Escribir redon(1.5); // 2
PRINT CINT(1.51) ' Escribir redon(1.51); // 2
PRINT CINT(1.49) ' Escribir redon(1.49); //  1
PRINT CINT(-1.01) ' Escribir redon(-1.01); //  2
PRINT CINT(-1.95) ' Escribir redon(-1.95); //  1

PRINT "TRUNCAR Pasa a entero qitando decimales";

PRINT INT(x) ' // truncamiento a entero
PRINT INT(1.51) ' Escribir trunc(1.51); 
PRINT INT(1.49) 'Escribir trunc(1.49); 
PRINT INT(-1.51) 'Escribir trunc(-1.51); 
PRINT INT(-1.51) 'Escribir trunc(-1.49); 
PRINT INT(0) 'Escribir trunc(0); 
	
PRINT "AZAR";
  RANDOMIZE TIMER
  PRINT "Random number from 0-9: ", INT(RND*10) ' azar(10) // numero al azar menor a 10 (entre 0 y 9 )
  PRINT "Random number from 0-10: ", INT(RND*10)+1 ' azar(10)+1
  PRINT "Random even integer from 50-100: ", INT(RND*25)+51 ' azar(25) * 2 + 51 
  
  PRINT "CADENAS"
  DIM S AS String
  s = "Hola"
  PRINT(s)
  PRINT LEN(S) ' Escribir Longitud(s); // cantidad de letras
  PRINT LCASE$(S)' Escribir Minusculas(s); // retorna una copia toda en minusculas
  PRINT UCASE$(S) ' Escribir Mayusculas(s); // retorna una copia toda en mayusculas 
  
  ' Escribir SubCadena(s,1,2); // retorna la cadena que desde la posicion 1 (segunda en base 0) a la 2 (tercera en base 0): "Ho"
  PRINT MID$(s,1,2) 
  PRINT MID$(s,1,1) 
  
  PRINT s + " Mundo" ' Escribir concatenar(s," Mundo");
  PRINT "Mundo " + s ' Escribir concatenar("Mundo ",s);
  PRINT VAL("15.5") ' Escribir ConvertirANumero("15.5"); // retorna un real 15.5
  PRINT STR$(15.5) ' Escribir ConvertirATexto(15.5); // retorna una cadena "15.5"
  PRINT "2" + "+3" ' escribir concatenar("2","+3") // => "2+3"
  PRINT "2" + "+(3" + "*4)" ' escribir concatenar( concatenar("2","+(3"), "*4)")  // => 2+(3*4)

  

 FUNCTION ARCSIN (x) 'Inverse Sine
    IF x <= 1 AND x >= -1 THEN
      ARCSIN = ATN(x / SQR(1 - (x * x)))
    ELSE
      PRINT "Domain Error"
      END ' finaliza la ejecucion del programa
    END IF
  END FUNCTION
  
 FUNCTION ARCCOS (x) 'Inverse Cosine
    IF x <= 1 AND x >= -1 THEN
      ARCCOS = (2 * ATN(1)) - ATN(x / SQR(1 - x * x))
    ELSE
      PRINT "Domain Error"
      END ' finaliza la ejecucion del programa
    END IF
  END FUNCTION  
  


/////////