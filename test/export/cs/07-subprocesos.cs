using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PSeInt
{
    class SIN_TITULO
    {
        static void Main(string[] args)
        {
            double c;

            Console.WriteLine(FuncionDoble(5.0));
            NoRetornaNada("3", "9");
            c = 0;
            PorReferencia(ref c);
            Console.WriteLine(c);
            string[] a = new string[10];
            string[,] b = new string[3, 4];
            RecibeVector(a);
            RecibeMatriz(b);
            Console.WriteLine("a {0}, b {1} ", a[1], b[1, 1]);
            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }

        static double FuncionDoble(double b)
        {
            double a;
            a = 2 * b;
            return a;
        }

        static void NoRetornaNada(string a, string b)
        {
            Console.WriteLine(a + b);
        }

        static void PorReferencia(ref double b)
        {
            b = 7;
        }

        static void RecibeVector(string[] v)
        {
            v[1] = Console.ReadLine();
        }

        static void RecibeMatriz(string[,] m)
        {
            m[1, 1] = Console.ReadLine();
        }
    }
}
