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
            int n;
            string[] a = new string[10];
            string[,] b = new string[20, 30];
            string[] c = new string[40];
            string[,] d = new string[50, 5];
            n = int.Parse(Console.ReadLine());
            string[] e = new string[n];
            a[1] = Console.ReadLine();
            // El subindice del arreglo llega hasta el 9 ya que comienza en Cero 0 el arreglo a[] comienza con un subindice 0 y termina en 10 cuando se declara de tamaño 10
            a[9] = a[1];
            Console.WriteLine(a[9]);
            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }
    }
}
