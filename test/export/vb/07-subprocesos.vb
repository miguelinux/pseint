' Este codigo ha sido generado por el modulo psexport 20131130-l64 de PSeInt
' dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental
' puede que el codigo generado no sea completamente correcto. Si encuentra
' errores por favor reportelos en el foro (http://pseint.sourceforge.net).

Module SIN_TITULO

	Sub Main()
		Dim c As Double;
		Dim a[0][ As String;
		Dim b[0][93][ As String;
		Console.WriteLine(funciondoble(5))
		noretornanada(3,9)
		c = 0
		porreferencia(c)
		Console.WriteLine(c)
		recibevector(a)
		recibematriz(b)
	End Sub

	Public Function funciondoble(ByVal Dim b As Double)
		a = 2*b
	Return Double
	End Function

	Public Sub noretornanada(ByVal Dim a As String, ByVal Dim b As String)
		Console.WriteLine(a+b)
	End Sub

	Public Sub porreferencia(ByRef Dim b As Double)
		b = 7
	End Sub

	Public Sub recibevector(ByVal Dim v As String)
		v(1) = Console.ReadLine()
	End Sub

	Public Sub recibematriz(ByVal Dim m As String)
		m(1,1) = Console.ReadLine()
	End Sub

End Module
