// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "nes.h"
#include "new-cpu.h"
#include "debug.h"
#include "mem.h"
#include "ppu.h"
#include "input.h"
#include "rom.h"
#include "apu.h"
#include "mapper.h"

Nes::Nes(Rom* rom, IMapper* mapper, IAudioProvider* audioProvider)
    : _rom(rom)
{
    //_debugger = new DebugService();
    _ppu = new Ppu(mapper);
    _apu = new Apu(false, audioProvider);
    _input = new Input();
    _mem = new MemoryMap(_ppu, _apu, _input, mapper);
    //_cpu = new Cpu(_mem, _debugger);
    _newCpu.Attach(new NewCpu(_mem));

    // TODO: Move these to an init method
    _newCpu->Reset(true);
    _apu->StartAudio(_mem); 
}

Nes::~Nes()
{
}

bool Nes::Create(const char* romPath, IAudioProvider* audioProvider, Nes** nes)
{
    NPtr<StdStreamRomFile> rom(new StdStreamRomFile(romPath));
    return Nes::Create(static_cast<IRomFile*>(rom), audioProvider, nes);
}

bool Nes::Create(IRomFile* romFile, IAudioProvider* audioProvider, Nes** nes)
{
    NPtr<Rom> rom;
    if (Rom::Create(romFile, &rom))
    {
        NPtr<IMapper> mapper;
        if (IMapper::CreateMapper(rom, &mapper))
        {
            *nes = new Nes(rom, mapper, audioProvider);
            return true;
        }
    }
    return false;
}

void Nes::Dispose()
{
    _apu->StopAudio();

    // Release smart pointers to avoid problems with circular references.
    _debugger.Release();
    _ppu.Release();
    _apu.Release();
    _input.Release();
    _mem.Release();
    //_cpu.Release();
}

void Nes::DoFrame(u8 screen[])
{
    PpuStepResult ppuResult;
    ApuStepResult apuResult;
    do
    {
        ppuResult.Reset();
        apuResult.Reset();

        _newCpu->Step();
        u32 stealCycles = 0;
        _apu->Step(_newCpu->IsDmaRunning(), apuResult, stealCycles);
        _newCpu->StealCycles(stealCycles);
        _ppu->Step(3, screen, ppuResult);
        
        
        if (ppuResult.WantNmi)
        {
            _newCpu->Nmi();
        }
        else if (apuResult.Irq || ppuResult.WantIrq)
        {
            _newCpu->Irq();
        }
    } while (!ppuResult.VBlank);
}

IStandardController* Nes::GetStandardController(unsigned int port)
{
    return _input->GetStandardController(port);
}

void Nes::SaveState()
{
    //std::ofstream ofs(GetSavePath()->c_str(), std::fstream::binary | std::fstream::trunc);
    //_cpu->SaveState(ofs);
    //ofs.close();

    //printf("State Saved!\n");
}

void Nes::LoadState()
{
    //auto savePath = GetSavePath();

    //if (!fs::exists(*savePath))
    //{
    //    printf("No save state for this ROM.\n");
    //    return;
    //}

    //std::ifstream ifs(GetSavePath()->c_str(), std::fstream::binary);
    //_cpu->LoadState(ifs);
    //ifs.close();

    //printf("State Loaded!\n");
}

//std::unique_ptr<fs::path> Nes::GetSavePath()
//{
//    fs::path savePath(_rom->Path());
//    return std::make_unique<fs::path>(savePath.replace_extension("ns"));
//}

void Nes::Reset(bool hard)
{
    _newCpu->Reset(hard);
    _mem->Reset(hard);
}