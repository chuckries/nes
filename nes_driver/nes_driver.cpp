// nes_driver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef unsigned char u8;
#include <nesapi.h>

#include <chrono>

#include <thread>

#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

class GdiGfx : public IGfx, public IInput
{
public:
    GdiGfx()
        : _pBitmap(nullptr)
        , _lastDrawTime(std::chrono::high_resolution_clock::now())
    {
        GdiplusStartup(&_gdiplusToken, &_gdiplusStartupInput, NULL);

        HINSTANCE hInstance = GetModuleHandle(NULL);

        RECT clientRect = {};
        clientRect.bottom = 240 * 3;
        clientRect.right = 256 * 3;
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
            this);

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

        std::chrono::time_point<std::chrono::steady_clock> now;
        long long duration;
        do
        {
            now = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - _lastDrawTime).count();
        } while (duration < 16666667); // 60 fps
        _lastDrawTime = now;

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

    Bitmap* _pBitmap;
    JoypadState _joypadState;
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastDrawTime;

    void OnPaint(HWND hWnd, HDC hdc)
    {
        if (_pBitmap != nullptr)
        {
            Gdiplus::Graphics graphics(hdc);

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

        GdiGfx* pThis;

        if (message == WM_CREATE)
        {
            pThis = static_cast<GdiGfx*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

            SetLastError(0);
            if (!SetWindowLongPtr(hWnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
            {
                if(GetLastError() != 0)
                {
                    return false;
                }
            }
        }
        else
        {
            pThis = reinterpret_cast<GdiGfx*>(GetWindowLongPtr(hWnd, GWL_USERDATA));
        }

        switch (message)
        {
        case WM_PAINT:
            if (pThis)
            {
                hdc = BeginPaint(hWnd, &ps);
                pThis->OnPaint(hWnd, hdc);
                EndPaint(hWnd, &ps);
                return 0;
            }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
        {
            JoypadState& joypad = pThis->_joypadState;
            switch (wParam)
            {
            case 0x53: joypad.A = true; break;
            case 0x41: joypad.B = true; break;
            case VK_LSHIFT: case VK_SHIFT: case VK_RSHIFT: joypad.Select = true; break;
            case VK_RETURN: joypad.Start = true; break;
            case VK_UP: joypad.Up = true; break;
            case VK_DOWN: joypad.Down = true; break;
            case VK_LEFT: joypad.Left = true; break;
            case VK_RIGHT: joypad.Right = true; break;
            default: return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
            return 0;
        case WM_KEYUP:
        {
            JoypadState& joypad = pThis->_joypadState;
            switch (wParam)
            {
            case 0x53: joypad.A = false; break;
            case 0x41: joypad.B = false; break;
            case VK_LSHIFT: case VK_SHIFT: case VK_RSHIFT: joypad.Select = false; break;
            case VK_RETURN: joypad.Start = false; break;
            case VK_UP: joypad.Up = false; break;
            case VK_DOWN: joypad.Down = false; break;
            case VK_LEFT: joypad.Left = false; break;
            case VK_RIGHT: joypad.Right = false; break;
            default: return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
            return 0;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
};

void RunNes(const char* romPath)
{
    //SdlGfx* sdlGfx = SdlGfx_Create(3);
    GdiGfx gdiGfx;

    Nes* nes = Nes_Create(romPath, (IGfx*)&gdiGfx, (IInput*)&gdiGfx);
    if (nes != nullptr)
        Nes_Run(nes);

    //SdlGfx_Destroy(sdlGfx);
    Nes_Destroy(nes);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.\n");
        return -1;
    }

    std::thread nesThread(RunNes, argv[1]);

    nesThread.join();

    //SdlGfx* sdlGfx = SdlGfx_Create(3);
    //GdiGfx gdiGfx1;
    //GdiGfx gdiGfx2;

    //Nes* nes1 = Nes_Create(argv[1], (IGfx*)&gdiGfx1, (IInput*)&gdiGfx1);
    //if (nes1 != nullptr)
    //    Nes_Run(nes1);

    ////SdlGfx_Destroy(sdlGfx);
    //Nes_Destroy(nes1);
}
