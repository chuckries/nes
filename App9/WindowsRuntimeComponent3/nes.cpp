// nes.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "nes.h"
#include "cpu.h"
#include "new_ppu.h"
#include "mem.h"
//#include "ppu.h"
#include "new_ppu.h"
#include "input.h"
#include "rom.h"
#include "apu.h"
//#include "sdlGfx.h"
#include "mapper.h"

#include <ppltasks.h>
using namespace concurrency;


using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

Nes::Nes(std::shared_ptr<Rom> rom, std::shared_ptr<IMapper> mapper)
    : _rom(rom)
    , _mapper(mapper)
{
    _ppu = std::make_shared<Ppu>(_mapper);
    _apu = std::make_shared<Apu>(false);
    _input = std::make_shared<Input>();
    _mem = std::make_shared<MemoryMap>(_ppu, _apu, _input, _mapper);
    _cpu = std::make_unique<Cpu>(_mem);
    _cpu->Reset();
    //_apu->StartAudio(_mem.get(), 44100);
}

Nes::~Nes()
{
}

std::unique_ptr<Nes> Nes::Create(const char* romPath)
{
    auto rom = std::make_shared<Rom>();
    if (!rom->Load(romPath))
        return nullptr;

    return Nes::Create(rom);
}

std::unique_ptr<Nes> Nes::Create(std::shared_ptr<Rom> rom)
{
    auto mapper = IMapper::CreateMapper(rom);
    if (mapper == nullptr)
    {
        printf("Mapper not supported\n");
        return nullptr;
    }

    return std::make_unique<Nes>(rom, mapper);
}

void Nes::DoFrame(const JoypadState& joypadState, u8 screen[])
{
    _input->State.Set(joypadState);

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

void Nes::Run()
{
    //SdlGfx gfx(3);
    //SdlInput input;
    //u8 screen[256 * 240 * 3];
    //for (;;)
    //{
    //    ZeroMemory(screen, sizeof(screen));

    //    // TODO: get joypadState
    //    input.CheckInput(_input->State);
    //    DoFrame(_input->State, screen);
    //    gfx.Blit(screen);
    //}
}

//void Nes::Run()
//{
//    std::shared_ptr<SdlGfx> gfx = std::make_shared<SdlGfx>(3);
//
//    std::shared_ptr<IMapper> mapper = IMapper::CreateMapper(_rom);
//    if (mapper == nullptr)
//    {
//        return;
//    }
//
//    Ppu ppu(mapper, std::dynamic_pointer_cast<IGfx>(gfx));
//    Apu apu(false /* isPal */);
//    Input input;
//    MemoryMap mem(ppu, apu, input, mapper);
//    _cpu = std::make_unique<Cpu>(mem);
//
//    _cpu->Reset();
//    
//    apu.StartAudio(&mem, 44100);
//
//    InputResult inputResult;
//    ApuStepResult apuResult;
//    PpuStepResult ppuResult;
//    bool wantSaveState = false;
//    bool wantLoadState = false;
//    for (;;)
//    {
//        apuResult.Reset();
//        ppuResult.Reset();
//
//         inputResult = input.CheckInput();
//         if (inputResult == InputResult::SaveState)
//         {
//             wantSaveState = true;
//         }
//         else if (inputResult == InputResult::LoadState)
//         {
//             wantLoadState = true;
//         }
//         else if (inputResult == InputResult::Quit)
//         {
//             break;
//         }
//
//        _cpu->Step();
//        apu.Step(_cpu->Cycles, _cpu->IsDmaRunning(), apuResult);
//        ppu.Step(_cpu->Cycles * 3, ppuResult);
//
//        _cpu->Cycles = 0;
//
//        if (ppuResult.VBlankNmi)
//        {
//            _cpu->Nmi();
//            if (wantSaveState)
//            {
//                SaveState();
//                wantSaveState = false;
//            }
//            if (wantLoadState)
//            {
//                LoadState();
//                wantLoadState = false;
//            }
//        }
//        else if (apuResult.Irq || ppuResult.WantIrq)
//        {
//            _cpu->Irq();
//        }
//    }
//
//    apu.StopAudio();
//}

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

WindowsRuntimeComponent3::NesComponent::RefNes::RefNes(Windows::Storage::StorageFile^ romFile)
{
    create_task(romFile->OpenReadAsync()).then([this](IRandomAccessStreamWithContentType^ stream)
    {
        auto reader = ref new DataReader(stream);
        create_task(reader->LoadAsync(stream->Size)).then([this, reader](task<unsigned int> bytesLoaded)
        {
            std::shared_ptr<Rom> rom = std::make_shared<Rom>();


            Platform::ArrayReference<u8> arrayReference((u8*)&(rom->Header), 16);

            reader->ReadBytes(arrayReference);

            if (rom->Header.PrgRamSize == 0)
            {
                rom->PrgRam.resize(PRG_RAM_UNIT_SIZE);
            }
            else
            {
                rom->PrgRam.resize(rom->Header.PrgRamSize * PRG_RAM_UNIT_SIZE);
            }

            //if (rom->Header.HasSaveRam())
            //{
            //    LoadGame();
            //}

            if (rom->Header.PrgRomSize > 0)
            {
                rom->PrgRom.resize(rom->Header.PrgRomSize * PRG_ROM_BANK_SIZE);
                //stream.read((char*)&PrgRom[0], PRG_ROM_BANK_SIZE * Header.PrgRomSize);
                reader->ReadBytes(Platform::ArrayReference<u8>((u8*)&rom->PrgRom[0], PRG_ROM_BANK_SIZE * rom->Header.PrgRomSize));
            }

            if (rom->Header.ChrRomSize > 0)
            {
                rom->ChrRom.resize(rom->Header.ChrRomSize * CHR_ROM_BANK_SIZE);
                //stream.read((char*)&ChrRom[0], CHR_ROM_BANK_SIZE * Header.ChrRomSize);
                reader->ReadBytes(Platform::ArrayReference<u8>((u8*)&rom->ChrRom[0], CHR_ROM_BANK_SIZE * rom->Header.ChrRomSize));
            }

            _nes = Nes::Create(rom);
        }).wait();
    }).wait();
}

void WindowsRuntimeComponent3::NesComponent::RefNes::DoFrame(WindowsRuntimeComponent3::NesComponent::JoypadState^ state, Platform::WriteOnlyArray<byte>^ screen)
{
    ::JoypadState native_state;
    native_state.A = state->A;
    native_state.B = state->B;
    native_state.Select = state->Select;
    native_state.Start = state->Start;
    native_state.Up = state->Up;
    native_state.Down = state->Down;
    native_state.Left = state->Left;
    native_state.Right = state->Right;

    _nes->DoFrame(native_state, screen->Data);
}