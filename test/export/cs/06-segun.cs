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
            int c; // La variable c, solo puede ser declarada int, bool, char, string, integral, enum o NULL
            c = int.Parse(Console.ReadLine());
            switch (c) // La variable c, solo puede ser declarada int, bool, char, string, integral, enum o NULL
            {
                case 1:
                    Console.WriteLine("1");
                    break;
                case 3:
                case 5:
                case 9:
                    Console.WriteLine("3, 5 o 9");
                    break;
                case 7:
                    Console.WriteLine("7");
                    break;
                // Si el valor de c no es 1, 3, 5, 9 o 7, la
                // se ejecuta el caso predeterminado.
                default:
                    Console.WriteLine("ni 1, ni 3, ni 5, ni 9, ni 7");
                    break;
            }
            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }
    }
}
