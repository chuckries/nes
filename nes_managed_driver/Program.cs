using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using nes_cs;

namespace nes_managed_driver
{
    class Program
    {
        static int Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("Must provide path to ROM");
                return 1;
            }

            SdlGfx gfx = SdlGfx.Create(IntPtr.Zero);

            Nes nes = Nes.Create(args[0], gfx);
            nes.Run();

            return 0;
        }
    }
}
