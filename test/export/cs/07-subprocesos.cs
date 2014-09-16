using System;
namespace PSeInt {
	class sin_titulo {
		static void Main(string[] args) {
			double c;
			Console.WriteLine(FuncionDoble(5.0));
			noretornanNada("3", "9");
			c = 0;
			porreferencia(ref c);
			Console.WriteLine(c);
			string[] a = new string[10];
			string[,] b = new string[3, 4];
			recibevector(a);
			recibematriz(b);
			Console.WriteLine("a {0}, b {1} ", a[1], b[1, 1]);
		}
		static double funciondoble(double b) {
			double a;
			a = 2 * b;
			return a;
		}
		static void noretornanada(string a, string b) {
			Console.WriteLine(a + b);
		}
		static void porreferencia(ref double b) {
			b = 7;
		}
		static void recibevector(string[] v) {
			v[1] = Console.ReadLine();
		}
		static void recibematriz(string[,] m) {
			m[1, 1] = Console.ReadLine();
		}
	}
}
