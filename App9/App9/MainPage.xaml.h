//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace App9
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

    private:
        void canvas_Update(Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl^ sender, Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedDrawEventArgs^ args);
        void canvas_Draw(Microsoft::Graphics::Canvas::UI::Xaml::ICanvasAnimatedControl^ sender, Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedDrawEventArgs^ args);
        void canvas_CreateResources(Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedControl^ sender, Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesEventArgs^ args);
        void canvas_KeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
        void canvas_KeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);

        void page_Loaded(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);

        Windows::Foundation::IAsyncAction^ CreateResourcesAsync(Microsoft::Graphics::Canvas::UI::Xaml::CanvasAnimatedControl^ sender);

    private:
        WindowsRuntimeComponent3::NesComponent::RefNes^ _nes;
        WindowsRuntimeComponent3::NesComponent::JoypadState^ _joypadState;

        Microsoft::Graphics::Canvas::CanvasBitmap^ _bitmap;
        Platform::Array<byte>^ _bitmapBytes;
	};
}
