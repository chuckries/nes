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

        public static SdlGfx Create(int scale)
        {
            return new SdlGfx(Native.SdlGfx_Create(scale));
        }

        private class Native
        {
            [DllImport("nes.dll")]
            public static extern IntPtr SdlGfx_Create(int scale);

            [DllImport("nes.dll")]
            public static extern IntPtr SdlGfx_Destroy(IntPtr sdlGfx);
        }
    }
}
