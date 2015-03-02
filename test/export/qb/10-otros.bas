  CLS ' Borrar Pantalla;
  SLEEP ' Esperar Tecla; ' // tambien se reemplaza por las dos siguientes sentencias
  WHILE INKEY$<>"": WEND
  DO: tecla$=INKEY$ : LOOP WHILE INKEY$=""
  
  
  ?? ' Esperar 100 Milisegundos;
  SLEEP 1 ' Esperar 1 Segundo;
  CONST PI = 3.141593
  print PI ' Escribir PI
