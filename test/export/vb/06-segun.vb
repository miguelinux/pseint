' Este codigo ha sido generado por el modulo psexport 20131130-l64 de PSeInt
' dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental
' puede que el codigo generado no sea completamente correcto. Si encuentra
' errores por favor reportelos en el foro (http://pseint.sourceforge.net).

Module SIN_TITULO

	Sub Main()
		Dim c As Double;
		c = Console.ReadLine()
		Select Case c
		Case 1
			Console.WriteLine("1")
		Case 3, 4, 9
			Console.WriteLine("3, 5 o 9")
		Case 7
			Console.WriteLine("7")
		Case Else
			Console.WriteLine("ni 1, ni 3, ni 5, ni 9, ni 7")
		End Select
	End Sub

End Module
