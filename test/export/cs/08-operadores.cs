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
            double a;
            double b;
            bool c;
            bool d;
            string e;
            string f;
            a = 11;
            b = 2;
            c = true;
            d = false;
            e = "Hola";
            f = "Mundo";
            Console.WriteLine(a + b);
            Console.WriteLine(a * b);
            Console.WriteLine(a / b);
            Console.WriteLine(a - b);
            // en C# no se puede realizar esta operacion 
            Console.WriteLine(Math.Pow(a,b));
            Console.WriteLine(a % b); // % Módulo (sólo sobre tipos de números enteros)
            Console.WriteLine(a = b);
            Console.WriteLine(a != b);
            Console.WriteLine(a < b);
            Console.WriteLine(a > b);
            Console.WriteLine(a <= b);
            Console.WriteLine(a >= b);
            Console.WriteLine(c && d);
            Console.WriteLine(c || d);
            Console.WriteLine(!(c && d));
            Console.WriteLine(e + " " + f);
            Console.WriteLine(e.Equals(f));
            Console.WriteLine(!e.Equals(f));
            Console.WriteLine(e.CompareTo(f) < 0);
            Console.WriteLine(e.CompareTo(f) > 0);
            Console.WriteLine(e.CompareTo(f) <= 0);
            Console.WriteLine(e.CompareTo(f) >= 0);
            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }
    }
}
