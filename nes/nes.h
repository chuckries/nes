#pragma once

class Rom;
class Cpu;
class IGfx;

class Nes
{
public:
    Nes(std::shared_ptr<Rom> rom, IGfx* gfx);
    __declspec(dllexport) ~Nes();

    static Nes* Create(const char* romPath, IGfx* gfx);
    //static std::unique_ptr<Nes> Create(std::shared_ptr<Rom> rom, std::shared_ptr<IGfx> gfx);

    void Run();
public:

private:
    void SaveState();
    void LoadState();
    std::unique_ptr<fs::path> GetSavePath();

private:
    std::shared_ptr<Rom> _rom;
    std::unique_ptr<Cpu> _cpu;
    IGfx* _gfx;
};

// External Interface
// External Interface
extern "C" __declspec(dllexport) Nes* Nes_Create(const char* romPath, IGfx* gfx);
extern "C" __declspec(dllexport) void Nes_Delete(Nes* nes);
extern "C" __declspec(dllexport) void Nes_Run(Nes* nes);