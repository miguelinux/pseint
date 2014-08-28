using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
// se requiere esta libreria para utilizar la funcion Math
//using System.Text.RegularExpressions;

namespace PSeInt
{
    class SIN_TITULO
    {
        static void Main(string[] args)
        {
            string s;
            double x ;
            x = 1.5;
            Console.WriteLine(Math.Sqrt(x));
            Console.WriteLine(Math.Abs(x));
            Console.WriteLine(Math.Log(x));
            Console.WriteLine(Math.Exp(x));
            Console.WriteLine(Math.Sin(x));
            Console.WriteLine(Math.Cos(x));
            Console.WriteLine(Math.Tan(x));
            Console.WriteLine(Math.Asin(x));
            Console.WriteLine(Math.Acos(x));
            Console.WriteLine(Math.Atan(x));
            Console.WriteLine(Math.Round(x));
            Console.WriteLine(Math.Truncate(x));
            int x1;
            x1 = 5;
            Console.WriteLine(new Random().Next(x1)); // x1 debe de er tipo entero
            s = "Hola";
            Console.WriteLine(s.Length);
            Console.WriteLine(s.ToLower());
            Console.WriteLine(s.ToUpper());
            Console.WriteLine(s.Substring(0,1));
            Console.WriteLine(s + " Mundo");
            Console.WriteLine(Convert.ToDouble("15.5"));
            Console.WriteLine(Convert.ToString(15.5));

            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }
    }
}
