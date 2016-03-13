using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace nes_managed_driver
{
    interface IGfx
    {
        IntPtr IntPtr { get; }
    }

    class SdlGfx : IGfx
    {
        public IntPtr IntPtr
        {
            get;
            private set;
        }

        private SdlGfx(IntPtr sdlGfxPtr)
        {
            IntPtr = sdlGfxPtr;
        }

        ~SdlGfx()
        {
            Internal.SdlGfx_Delete(IntPtr);
        }

        public static SdlGfx Create(int scale)
        {
            return new SdlGfx(Internal.SdlGfx_Create(scale));
        }

        private class Internal
        {
            [DllImport("nes.dll")]
            public static extern IntPtr SdlGfx_Create(int scale);

            [DllImport("nes.dll")]
            public static extern void SdlGfx_Delete(IntPtr sldGfx);
        }
    }
    class Nes
    {
        public IntPtr IntPtr { get; private set; }

        private Nes(IntPtr nesPtr)
        {
            IntPtr = nesPtr;
        }

        ~Nes()
        {
            Internal.Nes_Delete(IntPtr);
        }

        public static Nes Create(string romPath, IGfx gfx)
        {
            return new Nes(Internal.Nes_Create(romPath, gfx.IntPtr));
        }

        public void Run()
        {
            Internal.Nes_Run(IntPtr);
        }

        private class Internal
        {
            [DllImport("nes.dll")]
            public static extern IntPtr Nes_Create(string romPath, IntPtr IGfx);

            [DllImport("nes.dll")]
            public static extern IntPtr Nes_Delete(IntPtr nes);

            [DllImport("nes.dll")]
            public static extern void Nes_Run(IntPtr nes);
        }
    }

    class Program
    {
        static int Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("Must provide path to ROM");
                return 1;
            }

            SdlGfx gfx = SdlGfx.Create(3);

            Nes nes = Nes.Create(args[0], gfx);
            nes.Run();

            return 0;
        }
    }
}
