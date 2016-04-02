using Microsoft.Graphics.Canvas;
using Microsoft.Graphics.Canvas.UI;
using Microsoft.Graphics.Canvas.UI.Xaml;
using NesRuntimeComponent;
using System;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Storage.Pickers;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

using Microsoft.OneDrive.Sdk;
using System.Diagnostics;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Storage.Streams;
using System.IO;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace nesUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            this.Loaded += MainPage_Loaded;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            var args = e.Parameter as Windows.ApplicationModel.Activation.IActivatedEventArgs;
            if (args != null)
            {
                if (args.Kind == Windows.ApplicationModel.Activation.ActivationKind.File)
                {
                    var fileArgs = args as Windows.ApplicationModel.Activation.IFileActivatedEventArgs;
                    _romFile = (Windows.Storage.StorageFile)fileArgs.Files[0];
                }
            }
        }

        private void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
        }

        private void canvas_Draw(ICanvasAnimatedControl sender, CanvasAnimatedDrawEventArgs args)
        {
            var bitmap = CanvasBitmap.CreateFromBytes(
                sender.Device,
                _bitmapBytes,
                256,
                240,
                Windows.Graphics.DirectX.DirectXPixelFormat.R8G8B8A8UIntNormalized,
                sender.Dpi,
                CanvasAlphaMode.Ignore
                );

            args.DrawingSession.DrawImage(
                bitmap,
                new Rect(0, 0, sender.Size.Width, sender.Size.Height),
                bitmap.Bounds,
                1,
                CanvasImageInterpolation.NearestNeighbor
                );
        }

        private void canvas_Update(ICanvasAnimatedControl sender, CanvasAnimatedUpdateEventArgs args)
        {
            _nes.DoFrame(_bitmapBytes);
        }

        private void canvas_CreateResources(CanvasAnimatedControl sender, CanvasCreateResourcesEventArgs args)
        {
            args.TrackAsyncAction(CreateResourcesAsync(sender).AsAsyncAction());
        }

        private async Task CreateResourcesAsync(CanvasAnimatedControl sender)
        {
            OneDriveClient client = null;

            //try
            //{
            //    client = await OneDriveClientExtensions.GetAuthenticatedUniversalClient(new string[] { "onedrive.readonly", "wl.offline_access", "wl.signin", "onedrive.appfolder"}) as OneDriveClient;
            //}
            //catch(OneDriveException e)
            //{
            //    Debug.WriteLine(e.Error.Message);
            //    client.Dispose();
            //    App.Current.Exit();
            //}

            // var romContent = await client.Drive.Root.Children["smb.nes"].Content.Request().GetAsync();

            //var romContent = await client.Drive.Special.AppRoot.ItemWithPath("roms/smb.nes").Content.Request().GetAsync();

            //Windows.Storage.StreamedFileDataRequestedHandler handler = new Windows.Storage.StreamedFileDataRequestedHandler(async (Windows.Storage.StreamedFileDataRequest request) =>
            //{
            //    byte[] buf = new byte[romContent.Length];
            //    romContent.Read(buf, 0, (int)romContent.Length);
            //    await request.WriteAsync(buf.AsBuffer());
            //});

            //var file = await Windows.Storage.StorageFile.CreateStreamedFileAsync("smb.nes", handler, null);


            _bitmapBytes = new byte[256 * 240 * 4];

            //var file = _romFile;
            //if (file == null)
            //{
            //    var picker = new FileOpenPicker();
            //    picker.FileTypeFilter.Add(".nes");
            //    file = await picker.PickSingleFileAsync();
            //}

            //IRandomAccessStream stream = romContent.AsRandomAccessStream();

            var inMemoryStream = new InMemoryRandomAccessStream();
            await inMemoryStream.WriteAsync(new byte[0x100].AsBuffer());

            _nes = await Nes.Create(inMemoryStream);
            _controller0 = _nes.GetStandardController(0);
            Window.Current.CoreWindow.KeyDown += HandleKey;
            Window.Current.CoreWindow.KeyUp += HandleKey;
            sender.Width = 256 * 2;
            sender.Height = 240 * 2;
        }

        private void HandleKey(CoreWindow sender, KeyEventArgs args)
        {
            if (args.KeyStatus.RepeatCount == 1)
            {
                bool state = !args.KeyStatus.IsKeyReleased;
                switch (args.VirtualKey)
                {
                    case VirtualKey.S:
                    case VirtualKey.GamepadA:
                        _controller0.A(state);
                        break;
                    case VirtualKey.A:
                    case VirtualKey.GamepadX:
                        _controller0.B(state);
                        break;
                    case VirtualKey.Shift:
                    case (VirtualKey)0xde:
                    case VirtualKey.GamepadView:
                        _controller0.Select(state);
                        break;
                    case VirtualKey.Enter:
                    case VirtualKey.GamepadMenu:
                        _controller0.Start(state);
                        break;
                    case VirtualKey.Up:
                    case VirtualKey.GamepadDPadUp:
                        _controller0.Up(state);
                        break;
                    case VirtualKey.Down:
                    case VirtualKey.GamepadDPadDown:
                        _controller0.Down(state);
                        break;
                    case VirtualKey.Left:
                    case VirtualKey.GamepadDPadLeft:
                        _controller0.Left(state);
                        break;
                    case VirtualKey.Right:
                    case VirtualKey.GamepadDPadRight:
                        _controller0.Right(state);
                        break;
                }
            }
        }

        private Nes _nes;
        private StandardController _controller0;
        private byte[] _bitmapBytes;
        private Windows.Storage.StorageFile _romFile;
    }
}
