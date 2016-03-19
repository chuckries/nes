#include "pch.h"
#include "rom.h"

#include <fstream>

#include <ppltasks.h>
using namespace concurrency;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

Rom::Rom()
    : PrgRom(0)
    , ChrRom(0)
{
}

Rom::~Rom()
{
    // destructor is probably not the best place for IO
    // but here it goes for now
    if (Header.HasSaveRam())
    {
        SaveGame();
    }
}


bool Rom::Load(std::string romPath)
{
    _path = romPath;

    if (_path.is_relative())
    {
        _path = fs::current_path().append(_path);
    }

    if (!fs::exists(romPath))
    {
        printf("File does not exist\n");
        return false;
    }

    std::ifstream stream(romPath, std::ios::in | std::ios::binary);
    if (stream.is_open())
    {
        // read the header
        stream.read((char*)&Header, sizeof(Header));

        if (!Header.ValidateHeader())
        {
            printf("Invalid nes header\n");
            return false;
        }

        //TODO: Figure out how to handle a trainer
        if (Header.HasTrainer())
        {
            printf("ROMs with trainers are not supported\n");
            return false;
        }

        if (Header.PrgRamSize == 0)
        {
            PrgRam.resize(PRG_RAM_UNIT_SIZE);
        }
        else
        {
            PrgRam.resize(Header.PrgRamSize * PRG_RAM_UNIT_SIZE);
        }

        if (Header.HasSaveRam())
        {
            LoadGame();
        }

        if (Header.PrgRomSize > 0)
        {
            PrgRom.resize(Header.PrgRomSize * PRG_ROM_BANK_SIZE);
            stream.read((char*)&PrgRom[0], PRG_ROM_BANK_SIZE * Header.PrgRomSize);
        }

        if (Header.ChrRomSize > 0)
        {
            ChrRom.resize(Header.ChrRomSize * CHR_ROM_BANK_SIZE);
            stream.read((char*)&ChrRom[0], CHR_ROM_BANK_SIZE * Header.ChrRomSize);
        }

        stream.close();

        return true;
    }
    else 
    {
        printf("Unable to open file\n");
        return false;
    }
}

const fs::path& Rom::Path()
{
    return _path;
}

void Rom::SaveState(std::ofstream& ofs)
{
    ofs.write((char*)&PrgRam[0], PrgRam.size());
}

void Rom::LoadState(std::ifstream& ifs)
{
    ifs.read((char*)&PrgRam[0], PrgRam.size());
}

void Rom::SaveGame()
{
    std::ofstream ofs(GetSaveGamePath()->c_str(), std::ofstream::binary | std::ofstream::trunc);
    ofs.write((char*)&PrgRam[0], PrgRam.size());
    ofs.close();
}

void Rom::LoadGame()
{
    auto savePath = GetSaveGamePath();
    if (!fs::exists(*savePath))
    {
        return;
    }

    std::ifstream ifs(savePath->c_str(), std::ifstream::binary);
    ifs.read((char*)&PrgRam[0], PrgRam.size());
    ifs.close();
}

std::unique_ptr<fs::path> Rom::GetSaveGamePath()
{
    fs::path savePath(_path);
    return std::make_unique<fs::path>(savePath.replace_extension("sav"));
}

WindowsRuntimeComponent3::RefRom::RefRom(std::unique_ptr<Rom>& rom)
{
    _rom = std::move(rom);
}

WindowsRuntimeComponent3::RefRom::RefRom()
{
    _rom = std::make_unique<Rom>();
}

WindowsRuntimeComponent3::RefRom^ WindowsRuntimeComponent3::RefRom::FromFile(Windows::Storage::StorageFile^ file)
{
    RefRom^ refRom;
    create_task(file->OpenReadAsync()).then([refRom](IRandomAccessStreamWithContentType^ stream)
    {
        std::unique_ptr<Rom> rom;

        auto reader = ref new DataReader(stream);

        reader->ReadBytes(Platform::ArrayReference<u8>((u8*)&rom->Header, sizeof(INesHeader)));

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

        auto refRom = ref new RefRom(rom);
    }).wait();

    return refRom;
}