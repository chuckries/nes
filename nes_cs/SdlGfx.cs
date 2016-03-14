using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace nes_cs
{
    public class SdlGfx : IGfx
    {
        public IntPtr IntPtr { get; private set; } = IntPtr.Zero;

        private SdlGfx(IntPtr sdlGfx)
        {
            IntPtr = sdlGfx;
        }

        // TODO: This should potentially be done with a dispose pattern
        ~SdlGfx()
        {
            if (IntPtr != IntPtr.Zero)
            {
                Native.SdlGfx_Destroy(IntPtr);
            }
        }

        public static SdlGfx Create(IntPtr nativeWindow)
        {
            return new SdlGfx(Native.SdlGfx_Create(nativeWindow));
        }

        private class Native
        {
            [DllImport("nes.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr SdlGfx_Create(IntPtr nativeWindow);

            [DllImport("nes.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SdlGfx_Destroy(IntPtr sdlGfx);
        }
    }
}
