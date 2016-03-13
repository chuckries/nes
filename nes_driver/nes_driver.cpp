// nes_driver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <memory>
#include <experimental\filesystem>
namespace fs = std::experimental::filesystem;
#include "..\nes\types.h"
#include "..\nes\nes.h"
#include "..\nes\interfaces.h"
#include "..\nes\sdlGfx.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.");
        return -1;
    }

    SdlGfx* sdlGfx = SdlGfx_Create(3);

    Nes* nes = Nes_Create(argv[1], sdlGfx);
    if (nes != nullptr)
        Nes_Run(nes);

    SdlGfx_Delete(sdlGfx);
    Nes_Delete(nes);
}
