// nes_driver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef unsigned char u8;
#include <nesapi.h>

#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

class GdiGfx : public IGfx, public IInput
{
public:
    GdiGfx()
    {
        GdiplusStartup(&_gdiplusToken, &_gdiplusStartupInput, NULL);

        HINSTANCE hInstance = GetModuleHandle(NULL);

        RECT clientRect = {};
        clientRect.bottom = 240;
        clientRect.right = 256;
        AdjustWindowRect(&clientRect, (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX), false);

        WNDCLASS wndclass = {};
        wndclass.style = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc = WndProc;
        wndclass.lpszClassName = L"GDI RENDERER";
        RegisterClass(&wndclass);

        _hWnd = CreateWindow(
            L"GDI RENDERER",   // window class name
            L"NES",  // window caption
            WS_OVERLAPPEDWINDOW,      // window style
            CW_USEDEFAULT,            // initial x position
            CW_USEDEFAULT,            // initial y position
            clientRect.right - clientRect.left,            // initial x size
            clientRect.bottom - clientRect.top,            // initial y size
            NULL,                     // parent window handle
            NULL,                     // window menu handle
            nullptr,                // program instance handle
            NULL);

        ShowWindow(_hWnd, SW_SHOW);
        UpdateWindow(_hWnd);
    }

    ~GdiGfx()
    {
        GdiplusShutdown(_gdiplusToken);
    }

    void GdiGfx::Blit(u8 screen[])
    {
        if (_pBitmap != nullptr)
        {
            delete _pBitmap;
        }
        _pBitmap = new Bitmap(256, 240, 3 * 256, PixelFormat24bppRGB, screen);

        RedrawWindow(_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    InputResult GdiGfx::CheckInput(JoypadState* state)
    {
        // Process message queue
        MSG message;
        while (PeekMessage(&message, _hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        state->A = _joypadState.A;
        state->B = _joypadState.B;
        state->Select = _joypadState.Select;
        state->Start = _joypadState.Start;
        state->Up = _joypadState.Up;
        state->Down = _joypadState.Down;
        state->Left = _joypadState.Left;
        state->Right = _joypadState.Right;

        return InputResult::Continue;
    }

private:
    ULONG_PTR _gdiplusToken;
    GdiplusStartupInput _gdiplusStartupInput;

    HWND _hWnd;

    static Bitmap* _pBitmap;
    static JoypadState _joypadState;

    static void OnPaint(HWND hWnd, HDC hdc)
    {
        if (_pBitmap != nullptr)
        {
            Graphics graphics(hdc);

            RECT rect;
            GetClientRect(hWnd, &rect);

            graphics.DrawImage(_pBitmap, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
            //Pen      pen(Color(255, 0, 0, 255));
            //graphics.DrawLine(&pen, 0, 0, 200, 100);
        }
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
        WPARAM wParam, LPARAM lParam)
    {
        HDC hdc;
        PAINTSTRUCT ps;

        switch (message)
        {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            OnPaint(hWnd, hdc);
            EndPaint(hWnd, &ps);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            switch (wParam)
            {
            case 0x53: _joypadState.A = true; break;
            case 0x41: _joypadState.B = true; break;
            case VK_RSHIFT: _joypadState.Select = true; break;
            case VK_RETURN: _joypadState.Start = true; break;
            case VK_UP: _joypadState.Up = true; break;
            case VK_DOWN: _joypadState.Down = true; break;
            case VK_LEFT: _joypadState.Left = true; break;
            case VK_RIGHT: _joypadState.Right = true; break;
            default: return DefWindowProc(hWnd, message, wParam, lParam);
            }
            return 0;
        case WM_KEYUP:
            switch (wParam)
            {
            case 0x53: _joypadState.A = false; break;
            case 0x41: _joypadState.B = false; break;
            case VK_RSHIFT: _joypadState.Select = false; break;
            case VK_RETURN: _joypadState.Start = false; break;
            case VK_UP: _joypadState.Up = false; break;
            case VK_DOWN: _joypadState.Down = false; break;
            case VK_LEFT: _joypadState.Left = false; break;
            case VK_RIGHT: _joypadState.Right = false; break;
            default: return DefWindowProc(hWnd, message, wParam, lParam);
            }
            return 0;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
};

Bitmap* GdiGfx::_pBitmap = nullptr;
JoypadState GdiGfx::_joypadState;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.\n");
        return -1;
    }

    SdlGfx* sdlGfx = SdlGfx_Create(3);
    GdiGfx gdiGfx;

    Nes* nes = Nes_Create(argv[1], (IGfx*)&gdiGfx, (IInput*)&gdiGfx);
    if (nes != nullptr)
        Nes_Run(nes);

    SdlGfx_Destroy(sdlGfx);
    Nes_Destroy(nes);
}
