#pragma once

class Rom;
class Cpu;
class MemoryMap;
class Ppu;
class Apu;
class Input;

struct JoypadState;

class Nes
{
public:
    Nes(std::shared_ptr<Rom> rom, std::shared_ptr<IMapper> mapper);
    ~Nes();

    static std::unique_ptr<Nes> Create(const char* romPath);
    static std::unique_ptr<Nes> Create(std::shared_ptr<Rom> rom);

    void Run();

    // DoFrame runs all nes components until the ppu hits VBlank
    // This means that one call to DoFrame will render scanlines 241 - 261 then 0 - 240
    // The joypad state provided will be used for the entirety of the frame
    // screen is the pixel data buffer that the ppu will write to
    void DoFrame(const JoypadState& joypadState, u8 screen[]);

private:
    void SaveState();
    void LoadState();
    std::unique_ptr<fs::path> GetSavePath();

private:
    std::shared_ptr<Rom> _rom;
    std::shared_ptr<IMapper> _mapper;
    std::shared_ptr<Apu> _apu;
    std::shared_ptr<Ppu> _ppu;
    std::shared_ptr<Input> _input;
    std::shared_ptr<MemoryMap> _mem;
    std::unique_ptr<Cpu> _cpu;
};

namespace WindowsRuntimeComponent3
{
    namespace NesComponent
    {
        public ref struct JoypadState sealed
        {
        public:
            property bool A;
            property bool B;
            property bool Select;
            property bool Start;
            property bool Up;
            property bool Down;
            property bool Left;
            property bool Right;

            JoypadState()
            {
                A = false;
                B = false;
                Select = false;
                Start = false;
                Up = false;
                Down = false;
                Left = false;
                Right = false;
            }
        };

        public ref class RefNes sealed
        {
        public:
            RefNes(Windows::Storage::StorageFile^ romFile);

            void DoFrame(JoypadState^ state, Platform::WriteOnlyArray<byte>^ screen);

        private:
            std::unique_ptr<Nes> _nes;
        };
    }
}