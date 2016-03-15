using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Interop;

namespace nes_ui
{
    class NativeWindow : HwndHost
    {
        public IntPtr hWnd { get; private set; }
        private Native.WndProc _wndProc;

        public NativeWindow()
        {
            _wndProc = CustomWndProc;
        }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            // Create custom window to be our render context
            // When the Nes is constructed the hWnd of this window will be given to SDL

            string className = "Nes Window";
            Native.WNDCLASS wndclass = new Native.WNDCLASS();
            wndclass.lpszClassName = className;
            wndclass.lpfnWndProc = Marshal.GetFunctionPointerForDelegate(_wndProc);
            Native.RegisterClassW(ref wndclass);

            hWnd = Native.CreateWindowExW(
                0,
                className,
                "",
                Native.WS_CHILD,
                0,
                0,
                0,
                0,
                hwndParent.Handle,
                IntPtr.Zero,
                IntPtr.Zero,
                IntPtr.Zero);

            return new HandleRef(null, hWnd);
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            Native.DestroyWindow(hwnd.Handle);
        }

        private static IntPtr CustomWndProc(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam)
        {
            return Native.DefWindowProcW(hWnd, msg, wParam, lParam);
        }

        public void OnSetFocus()
        {
            Native.SetFocus(hWnd);
        }
    }

    /// <summary>
    /// Class for creating a custom native win32 window.
    /// Adapted from http://stackoverflow.com/questions/128561/registering-a-custom-win32-window-class-from-c-sharp
    /// </summary>
    class CustomWindow
    {
        private const int ERROR_CLASS_ALREADY_EXISTS = 1410;

        public IntPtr HWnd { get; private set; }

        public CustomWindow(string class_name)
        {
            if (class_name == null) throw new System.Exception("class_name is null");
            if (class_name == String.Empty) throw new System.Exception("class_name is empty");

            m_wnd_proc_delegate = CustomWndProc;

            // Create WNDCLASS
            Native.WNDCLASS wind_class = new Native.WNDCLASS();
            wind_class.lpszClassName = class_name;
            wind_class.lpfnWndProc = Marshal.GetFunctionPointerForDelegate(m_wnd_proc_delegate);

            UInt16 class_atom = Native.RegisterClassW(ref wind_class);

            int last_error = Marshal.GetLastWin32Error();

            if (class_atom == 0 && last_error != ERROR_CLASS_ALREADY_EXISTS)
            {
                throw new Exception("Could not register window class");
            }

            // Create window
            HWnd = Native.CreateWindowExW(
                0,
                class_name,
                String.Empty,
                0,
                0,
                0,
                0,
                0,
                IntPtr.Zero,
                IntPtr.Zero,
                IntPtr.Zero,
                IntPtr.Zero
            );
        }

        private static IntPtr CustomWndProc(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam)
        {
            return Native.DefWindowProcW(hWnd, msg, wParam, lParam);
        }

        private Native.WndProc m_wnd_proc_delegate;
    }

    /// <summary>
    /// Calls into native window functions
    /// </summary>
    class Native
    {
        public const UInt32 WS_CHILD = 0x40000000;

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct WNDCLASS
        {
            public uint style;
            public IntPtr lpfnWndProc;
            public int cbClsExtra;
            public int cbWndExtra;
            public IntPtr hInstance;
            public IntPtr hIcon;
            public IntPtr hCursor;
            public IntPtr hbrBackground;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string lpszMenuName;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string lpszClassName;
        }

        [DllImport("user32.dll", SetLastError = true)]
        public static extern IntPtr CreateWindowExW(
           UInt32 dwExStyle,
           [MarshalAs(UnmanagedType.LPWStr)]
           string lpClassName,
           [MarshalAs(UnmanagedType.LPWStr)]
           string lpWindowName,
           UInt32 dwStyle,
           Int32 x,
           Int32 y,
           Int32 nWidth,
           Int32 nHeight,
           IntPtr hWndParent,
           IntPtr hMenu,
           IntPtr hInstance,
           IntPtr lpParam
        );

        [DllImport("user32.dll", SetLastError = true)]
        public static extern ushort RegisterClassW(
            [In] ref WNDCLASS lpWndClass
        );

        [DllImport("user32.dll", SetLastError = true)]
        public static extern IntPtr DefWindowProcW(
            IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam
        );

        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool DestroyWindow(
            IntPtr hWnd
        );

        public delegate IntPtr WndProc(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        public static extern IntPtr SetFocus(IntPtr hWnd);
    }
}
