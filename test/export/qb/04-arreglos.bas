DIM z(0 TO 9) AS INTEGER ' en base cero
DIM a(1 TO 10) AS INTEGER ' en base uno
DIM b(1 TO 20, 1 TO 30) ' QBASIC asume como "single precision real number si no define tipo"
DIM c(1 TO 40) AS INTEGER, d(1 TO 50, 1 TO 5) AS INTEGER
INPUT "", n
DIM e(1 TO n)
INPUT "", a(1)
PRINT a(1)
a(10) = a(1)
PRINT a(10)
PRINT b(10, 10)
PRINT "impresion multiples"
PRINT b(9, 9); " "; b(10, 10)
PRINT "lecturas multiples a(1) y a(2)"
INPUT "", a(1), a(2)
PRINT a(1); " "; a(2)