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
            int a;
            double b;
            string c;
            bool d;
            a = int.Parse(Console.ReadLine());
            b = Double.Parse(Console.ReadLine());
            c = Console.ReadLine();
            d = Boolean.Parse(Console.ReadLine());
            Console.WriteLine(a);
            Console.WriteLine(b);
            Console.WriteLine(c);
            Console.WriteLine(d);
            a = int.Parse(Console.ReadLine());
            b = Double.Parse(Console.ReadLine());
            c = Console.ReadLine();
            d = Boolean.Parse(Console.ReadLine());
            Console.WriteLine(a.ToString(), b.ToString(), c, d.ToString());
            Console.Write("Esta linea no lleva enter al final");

            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }
    }
}
