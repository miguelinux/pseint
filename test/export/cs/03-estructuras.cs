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
            c = 0;

            while (c <= 10)
            {
                c = c + 1;
                Console.WriteLine(c);
            }
            c = 0;
            do
            {
                c = c + 1;
                Console.WriteLine(c);
            } while (c == 10);
            c = 0;
            do
            {
                c = c + 1;
                Console.WriteLine(c);
            } while (c < 10);
            if (c == 10)
            {
                Console.WriteLine("Si");
            }
            if (c == 10)
            {
                Console.WriteLine("Si");
            }
            else
            {
                Console.WriteLine("No");
            }

            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }
    }
}
