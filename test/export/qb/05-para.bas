' http://www.tek-tips.com/viewthread.cfm?qid=533873

DIM a(1 TO 10) AS INTEGER 

FOR i = 1 TO 10 
    a(i) = i * 10 
NEXT i

PRINT "" ' Escribir "";
PRINT "imprime los 10 elementos de a" 

FOR aux_index_1 = 1 TO UBOUND(a)
    PRINT a(aux_index_1)
NEXT aux_index_1

DIM b(1 TO 3, 1 TO 6) AS INTEGER
c = 0
FOR aux_index_1 = LBOUND(b) TO UBOUND(b)
    FOR aux_index_2 = LBOUND( b(aux_index_1,1) ) TO uBOUND( b(aux_index_1,2) )
        c = c + 1
        b(aux_index_1, aux_index_2) = c
    NEXT aux_index_2
NEXT aux_index_1

PRINT "imprime los 18 elementos de b" 
FOR i = 1 TO 3 
    FOR j = 1 TO 6
        PRINT b(i, j)
    NEXT j 
NEXT i


