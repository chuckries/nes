#pragma once

class Rom;
class Cpu;
class IGfx;

class Nes
{
public:
    Nes(std::shared_ptr<Rom> rom, IGfx* gfx, IInput* input);
    ~Nes();

    static Nes* Create(const char* romPath, IGfx* gfx, IInput* input);
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
    IInput* _input;
};