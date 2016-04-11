#include "stdafx.h"
#include "..\include\nes_api.h"
#include "nes.h"

bool Nes_Create(IRomFile* romFile, IAudioProvider* audioProvider, INes** ines)
{
    NPtr<Nes> nes;
    if (Nes::Create(romFile, audioProvider, &nes))
    {
        *ines = static_cast<INes*>(nes.Detach());
        return true;
    }
    else
    {
        return false;
    }
}
