using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
// Se requiere la libreria using System.Threading; para utilizar el la funcion Thread.Sleep(100);
using System.Threading;

namespace PSeInt
{
    class otros
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hola Mundo");
            Console.Clear();
            Console.ReadKey();
            // Se requiere la libreria using System.Threading;
            Thread.Sleep(100);
            Thread.Sleep(1*1000);
            Console.WriteLine(Math.PI);
            // esta linea de comando es necesaria para esperar cualquier tecla para terminar el programa
            Console.ReadKey();
        }
    }
}