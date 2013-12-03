Module SIN_TITULO
	Sub Main()
		Dim a(10) As String
		Dim b(3,4) As String
		Dim c As Double
		Console.WriteLine(funciondoble(5))
		noretornanada(3,9)
		c = 0
		porreferencia(c)
		Console.WriteLine(c)
		recibevector(a)
		recibematriz(b)
	End Sub
	Public Function funciondoble(ByVal b As Double)
		a = 2*b
		Return a
	End Function
	Public Sub noretornanada(ByVal a As Double, ByVal b As Double)
		Console.WriteLine(a+b)
	End Sub
	Public Sub porreferencia(ByRef b As Double)
		b = 7
	End Sub
	Public Sub recibevector(ByVal v() As String)
		v(1) = Console.ReadLine()
	End Sub
	Public Sub recibematriz(ByVal m(,) As String)
		m(1,1) = Console.ReadLine()
	End Sub
End Module
