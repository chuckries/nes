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

class GdiGfx : public IGfx
{
public:
    GdiGfx()
    {
        GdiplusStartup(&_gdiplusToken, &_gdiplusStartupInput, NULL);

        HINSTANCE hInstance = GetModuleHandle(NULL);

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
            256,            // initial x size
            240,            // initial y size
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

        MSG message;
        while (PeekMessage(&message, _hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

private:
    ULONG_PTR _gdiplusToken;
    GdiplusStartupInput _gdiplusStartupInput;

    HWND _hWnd;

    static Bitmap* _pBitmap;

    static void OnPaint(HDC hdc)
    {
        if (_pBitmap != nullptr)
        {
            Graphics graphics(hdc);
            graphics.DrawImage(_pBitmap, 0, 0);
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
            OnPaint(hdc);
            EndPaint(hWnd, &ps);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
};

Bitmap* GdiGfx::_pBitmap = nullptr;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.\n");
        return -1;
    }

    //SdlGfx* sdlGfx = SdlGfx_Create(3);
    GdiGfx gdiGfx;

    Nes* nes = Nes_Create(argv[1], (IGfx*)&gdiGfx);
    if (nes != nullptr)
        Nes_Run(nes);

    //SdlGfx_Destroy(sdlGfx);
    Nes_Destroy(nes);
}
