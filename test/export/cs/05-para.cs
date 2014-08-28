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
            int i;
            int j;
            double[] a = new double[10];
            for (i = 0; i < 10; i++)
            {
                a[i] = i * 10;
            }

            foreach (int elemento in a)
            {
                Console.WriteLine(elemento);
            }
            double[,] b = new double[3, 6];
            c = 0;
            foreach (int x in b)
            {
                c = c + 1;
                // No se puede asignar el valor de c en x ya que x es la variable de iteracion 
                //x = c;
            }
            for (i = 2; i > 1; i--)
            {
                for (j = 1; j < 5; j = j + 2)
                {
                    Console.WriteLine(b[i, j]);
                }
            }

            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }
    }
}
