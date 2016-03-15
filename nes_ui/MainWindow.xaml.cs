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
using Microsoft.Win32;

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
            var ofd = new OpenFileDialog();
            ofd.CheckFileExists = true;
            ofd.CheckPathExists = true;
            ofd.DefaultExt = ".nes";
            bool? result = ofd.ShowDialog();

            if (result == true)
            {
                SdlGfx gfx = SdlGfx.Create(this.RenderContext.hWnd);

                Nes nes = Nes.Create(ofd.FileName, gfx);
                FocusRenderContext();
                nes.Run();
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            FocusRenderContext();
        }

        private void FocusRenderContext()
        {
            Keyboard.Focus(this.RenderContext);
        }

        private void RenderContext_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.RenderContext.OnSetFocus();
        }
    }
}
