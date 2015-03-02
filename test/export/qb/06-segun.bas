' No es obligatorio el caso por defecto.
' variable de control puede comparar cadenas.
' no se requiere dos puntos para finalizar la comparacion.
' no requiere de BREAK para salir.

DIM a AS INTEGER
PRINT "entre c como ENTERO y entre 1 y 9"
INPUT "", c
SELECT CASE c
    CASE 1
        PRINT "1"
    CASE 3, 5, 9 ' // lista de casos
        PRINT "3, 5 o 9"
    CASE 7
        PRINT "7"
    CASE ELSE
        PRINT "ni 1, ni 3, ni 5, ni 7, ni 9"
END SELECT
