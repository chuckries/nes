#include "stdafx.h"
#include "nesapi.h"
#include "nes.h"
#include "sdlGfx.h"

// Nes Functions

Nes* Nes_Create(const char* romPath, IGfx* gfx)
{
    return Nes::Create(romPath, gfx);
}

void Nes_Run(Nes* nes)
{
    if (nes != nullptr)
    {
        nes->Run();
    }
}

void Nes_Destroy(Nes* nes)
{
    if (nes != nullptr)
    {
        delete nes;
    }
}

// SdlGfx Functions

SdlGfx* SdlGfx_Create(void* pNativeWindow)
{
    return new SdlGfx(pNativeWindow);
}

void SdlGfx_Destroy(SdlGfx* gfx)
{
    if (gfx != nullptr)
    {
        delete gfx;
    }
}

#include <Windows.h>
void* Gfx_CreateWindowsWindow(void* hwndParent)
{
    HWND parentWindow = (HWND)hwndParent;
    HINSTANCE hInstance = (HINSTANCE)&__ImageBase;

    GetWindowLongPtr(parentWindow, GWLP_WNDPROC);

    const wchar_t CLASS_NAME[] = L"Render Widow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = (WNDPROC)GetWindowLongPtr(parentWindow, GWLP_WNDPROC);
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    return (void*)CreateWindow(
        CLASS_NAME,
        L"Render Context",
        WS_CHILD,
        0,
        0,
        256,
        240,
        parentWindow,
        nullptr,
        hInstance,
        nullptr);
}