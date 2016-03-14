#pragma once

// External interface include file

#ifdef NES_EXPORTS
#define NES_API __declspec(dllexport) __declspec(cdecl)
#else
#define NES_API __declspec(dllimport)
#endif

class Nes;
class IGfx;
class SdlGfx;

extern "C"
{
    // Nes functions
    NES_API Nes* Nes_Create(const char* romPath, IGfx* gfx);
    NES_API void Nes_Run(Nes* nes);
    NES_API void Nes_Destroy(Nes* nes);

    // SdlGfx functions
    NES_API SdlGfx* SdlGfx_Create(void* pNativeWindow);
    NES_API void SdlGfx_Destroy(SdlGfx* sdlGfx);

    // Native Windows
    NES_API void* Gfx_CreateWindowsWindow(void* hwndParent);
}