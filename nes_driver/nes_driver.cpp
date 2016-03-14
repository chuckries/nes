// nes_driver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <nesapi.h>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.\n");
        return -1;
    }

    SdlGfx* sdlGfx = SdlGfx_Create(3);

    Nes* nes = Nes_Create(argv[1], (IGfx*)sdlGfx);
    if (nes != nullptr)
        Nes_Run(nes);

    SdlGfx_Destroy(sdlGfx);
    Nes_Destroy(nes);
}
