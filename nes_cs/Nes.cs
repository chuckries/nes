using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace nes_cs
{
    public class Nes
    {
        public IntPtr IntPtr { get; private set; } = IntPtr.Zero;

        private Nes(IntPtr nes)
        {
            IntPtr = nes;
        }

        // TODO: This should potentially be a dispose pattern.
        ~Nes()
        {
            if (IntPtr != IntPtr.Zero)
            {
                Native.Nes_Destroy(IntPtr);
            }
        }

        public static Nes Create(string romPath, IGfx gfx)
        {
            return new Nes(Native.Nes_Create(romPath, gfx.IntPtr));
        }

        public void Run()
        {
            Native.Nes_Run(IntPtr);
        }

        [DllImport("nes.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Gfx_CreateWindowsWindow(IntPtr parentWindow);

        private class Native
        {
            [DllImport("nes.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr Nes_Create(string romPath, IntPtr gfx);

            [DllImport("nes.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void Nes_Run(IntPtr nes);

            [DllImport("nes.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void Nes_Destroy(IntPtr nes);
        }
    }
}
