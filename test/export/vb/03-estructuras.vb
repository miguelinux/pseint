' Este codigo ha sido generado por el modulo psexport 20131129-l64 de PSeInt
' dado que dicho modulo se encuentra aun en desarrollo y en etapa experimental
' puede que el codigo generado no sea completamente correcto. Si encuentra
' errores por favor reportelos en el foro (http://pseint.sourceforge.net).

Module SIN_TITULO

	Sub Main()
		Dim c As Double;
		c = 0
		While c<=10
			c = c+1
			Console.WriteLine(c)
		End While
		c = 0
		Do 
			c = c+1
			Console.WriteLine(c)
		Loop Until !(c=10)
		c = 0
		Do 
			c = c+1
			Console.WriteLine(c)
		Loop While c<10
		If c=10 Then
			Console.WriteLine("Si")
		End If
		If c=10 Then
			Console.WriteLine("Si")
		Else
			Console.WriteLine("No")
		End If
	End Sub

End Module
