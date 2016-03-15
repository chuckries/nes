// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "nes.h"
#include "cpu.h"
#include "new_ppu.h"
#include "mem.h"
//#include "ppu.h"
#include "new_ppu.h"
#include "input.h"
#include "rom.h"
#include "apu.h"
#include "sdlGfx.h"
#include "mapper.h"

Nes::Nes(std::shared_ptr<Rom> rom, IGfx* gfx, IInput* input)
    : _rom(rom)
    , _gfx(gfx)
    , _input(input)
{
}

Nes::~Nes()
{
}

Nes* Nes::Create(const char* romPath, IGfx* gfx, IInput* input)
{
    auto rom = std::make_shared<Rom>();
    if (!rom->Load(romPath))
        return nullptr;

    return new Nes(rom, gfx, input);
}

//std::unique_ptr<Nes> Nes::Create(std::shared_ptr<Rom> rom, std::shared_ptr<IGfx> gfx)
//{
//    return std::make_unique<Nes>(rom, gfx);
//}

void Nes::Run()
{
    std::shared_ptr<IMapper> mapper = IMapper::CreateMapper(_rom);
    if (mapper == nullptr)
    {
        return;
    }

    Ppu ppu(mapper, _gfx);
    Apu apu(false /* isPal */);
    Input input;
    MemoryMap mem(ppu, apu, input, mapper);
    _cpu = std::make_unique<Cpu>(mem);

    _cpu->Reset();
    
    apu.StartAudio(&mem, 44100);

    InputResult inputResult;
    ApuStepResult apuResult;
    PpuStepResult ppuResult;
    bool wantSaveState = false;
    bool wantLoadState = false;
    for (;;)
    {
        apuResult.Reset();
        ppuResult.Reset();

        _cpu->Step();
        apu.Step(_cpu->Cycles, _cpu->IsDmaRunning(), apuResult);
        ppu.Step(_cpu->Cycles * 3, ppuResult);

        _cpu->Cycles = 0;

        if (ppuResult.VBlank)
        {
            //inputResult = input.CheckInput();
            inputResult = _input->CheckInput(&input.Joypad0.State);
            if (inputResult == InputResult::SaveState)
            {
                wantSaveState = true;
            }
            else if (inputResult == InputResult::LoadState)
            {
                wantLoadState = true;
            }
            else if (inputResult == InputResult::Quit)
            {
                break;
            }
        }

        if (ppuResult.WantNmi)
        {
            _cpu->Nmi();
            if (wantSaveState)
            {
                SaveState();
                wantSaveState = false;
            }
            if (wantLoadState)
            {
                LoadState();
                wantLoadState = false;
            }
        }
        else if (apuResult.Irq || ppuResult.WantIrq)
        {
            _cpu->Irq();
        }
    }

    apu.StopAudio();
}

void Nes::SaveState()
{
    std::ofstream ofs(GetSavePath()->c_str(), std::fstream::binary | std::fstream::trunc);
    _cpu->SaveState(ofs);
    ofs.close();

    printf("State Saved!\n");
}

void Nes::LoadState()
{
    auto savePath = GetSavePath();

    if (!fs::exists(*savePath))
    {
        printf("No save state for this ROM.\n");
        return;
    }

    std::ifstream ifs(GetSavePath()->c_str(), std::fstream::binary);
    _cpu->LoadState(ifs);
    ifs.close();

    printf("State Loaded!\n");
}

std::unique_ptr<fs::path> Nes::GetSavePath()
{
    fs::path savePath(_rom->Path());
    return std::make_unique<fs::path>(savePath.replace_extension("ns"));
}