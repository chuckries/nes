// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "nes.h"
#include "cpu.h"
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
    _debugger.Attach(new DebugService());
    _ppu.Attach(new Ppu(mapper));
    _apu.Attach(new Apu(false, audioProvider));
    _input.Attach(new Input());
    _mem.Attach(new MemoryMap(_ppu, _apu, _input, mapper));
    _cpu.Attach(new Cpu(_mem, _debugger));

    // TODO: Move these to an init method
    _cpu->Reset();
    _apu->StartAudio(_mem); 
}

Nes::~Nes()
{
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
    _cpu.Release();
}

void Nes::DoFrame(u8 screen[])
{
    PpuStepResult ppuResult;
    ApuStepResult apuResult;
    do
    {
        ppuResult.Reset();
        apuResult.Reset();

        _cpu->Step();
        _apu->Step(_cpu->Cycles, _cpu->IsDmaRunning(), apuResult);
        _ppu->Step(_cpu->Cycles * 3, screen, ppuResult);
        
        _cpu->Cycles = 0;
        
        if (ppuResult.WantNmi)
        {
            _cpu->Nmi();
        }
        else if (apuResult.Irq || ppuResult.WantIrq)
        {
            _cpu->Irq();
        }
    } while (!ppuResult.VBlank);
}

IStandardController* Nes::GetStandardController(unsigned int port)
{
    return _input->GetStandardController(port);
}

void Nes::SaveState(IWriteStream* ostream)
{
    _cpu->SaveState(ostream);
}

void Nes::LoadState(IReadStream* istream)
{
    _cpu->LoadState(istream);
}
