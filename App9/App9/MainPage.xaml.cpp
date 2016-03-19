//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <random>

using namespace App9;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::UI::Xaml;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
    this->Loaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &App9::MainPage::page_Loaded);
}

void MainPage::page_Loaded(Object^ sender, RoutedEventArgs^ args)
{
    Window::Current->CoreWindow->KeyDown += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &App9::MainPage::canvas_KeyDown);
    Window::Current->CoreWindow->KeyUp += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &App9::MainPage::canvas_KeyUp);
}

void MainPage::canvas_Draw(ICanvasAnimatedControl^ sender, CanvasAnimatedDrawEventArgs^ args)
{
    args->DrawingSession->DrawImage(_bitmap, Rect(0, 0, sender->Size.Width, sender->Size.Height));
}

void MainPage::canvas_Update(ICanvasAnimatedControl^ sender, CanvasAnimatedDrawEventArgs^ args)
{
    _nes->DoFrame(_joypadState, _bitmapBytes);
    _bitmap->SetPixelBytes(_bitmapBytes);
}

void MainPage::canvas_CreateResources(CanvasAnimatedControl^ sender, CanvasCreateResourcesEventArgs^ args)
{
    args->TrackAsyncAction(CreateResourcesAsync(sender));

    _joypadState = ref new WindowsRuntimeComponent3::NesComponent::JoypadState();
    _bitmapBytes = ref new Array<byte>(256 * 240 * 4);
    _bitmap = CanvasBitmap::CreateFromBytes(sender->Device, _bitmapBytes, 256, 240, Windows::Graphics::DirectX::DirectXPixelFormat::R8G8B8A8UIntNormalized, sender->Dpi, Microsoft::Graphics::Canvas::CanvasAlphaMode::Ignore);
}

IAsyncAction^ MainPage::CreateResourcesAsync(CanvasAnimatedControl^ sender)
{
    return create_async([this]()
    {
        create_task([this]() {
            return StorageFile::GetFileFromPathAsync(L"C:\\Users\\chuckr\\Documents\\Super Mario Bros. (Japan, USA).nes");
            //auto picker = ref new FileOpenPicker();
            //picker->FileTypeFilter->Append(L".nes");
            //return picker->PickSingleFileAsync();
        }).then([this](StorageFile^ file) {
            _nes = ref new WindowsRuntimeComponent3::NesComponent::RefNes(file);
        }).then([](task<void> t) {
            try
            {
                t.get();
            }
            catch (Platform::COMException^ e)
            {
                OutputDebugString(e->Message->Data());
            }
        }).wait();
    });
}

void MainPage::canvas_KeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    switch (args->VirtualKey)
    {
    case Windows::System::VirtualKey::S: _joypadState->A = true; break;
    case Windows::System::VirtualKey::A: _joypadState->B = true; break;
    case Windows::System::VirtualKey::RightShift: _joypadState->Select = true; break;
    case Windows::System::VirtualKey::Enter: _joypadState->Start = true; break;
    case Windows::System::VirtualKey::Up: _joypadState->Up = true; break;
    case Windows::System::VirtualKey::Down: _joypadState->Down = true; break;
    case Windows::System::VirtualKey::Left: _joypadState->Left = true; break;
    case Windows::System::VirtualKey::Right: _joypadState->Right = true; break;
    }
}

void MainPage::canvas_KeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    switch (args->VirtualKey)
    {
    case Windows::System::VirtualKey::S: _joypadState->A = false; break;
    case Windows::System::VirtualKey::A: _joypadState->B = false; break;
    case Windows::System::VirtualKey::RightShift: _joypadState->Select = false; break;
    case Windows::System::VirtualKey::Enter: _joypadState->Start = false; break;
    case Windows::System::VirtualKey::Up: _joypadState->Up = false; break;
    case Windows::System::VirtualKey::Down: _joypadState->Down = false; break;
    case Windows::System::VirtualKey::Left: _joypadState->Left = false; break;
    case Windows::System::VirtualKey::Right: _joypadState->Right = false; break;
    }
}