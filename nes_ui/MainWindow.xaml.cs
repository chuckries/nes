using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using nes_cs;
using System.Threading;
using System.Windows.Interop;
using System.Runtime.InteropServices;

namespace nes_ui
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void MenuItem_Open_Click(object sender, RoutedEventArgs e)
        {
            SdlGfx gfx = SdlGfx.Create(this.RenderContext.Window.Handle);

            ThreadPool.QueueUserWorkItem(o =>
            {
                Nes nes = Nes.Create(@"\\Mac\Home\Downloads\Nintendo\Super Mario Bros. 3.nes", gfx);
                nes.Run();
            });
        }
    }

    public class RenderContext : HwndHost
    {
        public HandleRef Window { get; private set; }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            IntPtr window = Nes.Gfx_CreateWindowsWindow(hwndParent.Handle);
            HandleRef handleRef = new HandleRef(null, window);
            Window = handleRef;
            return handleRef;
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            // TODO
        }
    }
}
