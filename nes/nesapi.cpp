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
