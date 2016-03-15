#pragma once

// External interface include file

#ifdef NES_EXPORTS
#define NES_API __declspec(dllexport)
#else
#define NES_API __declspec(dllimport)
#endif

class Nes;
class IGfx;
class IInput;
class SdlGfx;
class SdlInput;

extern "C"
{
    // Nes functions
    NES_API Nes* Nes_Create(const char* romPath, IGfx* gfx, IInput* input);
    NES_API void Nes_Run(Nes* nes);
    NES_API void Nes_Destroy(Nes* nes);

    // SdlGfx functions
    NES_API SdlGfx* SdlGfx_Create(int scale);
    NES_API void SdlGfx_Destroy(SdlGfx* sdlGfx);
}


// Graphics interface
class IGfx
{
public:
    virtual void Blit(u8 screen[]) = 0;

    //Optional
    virtual void BlitNameTable(u8 screen[], int i) { }
    virtual void BlitPatternTable(u8 left[], u8 right[]) { }
};

enum class InputResult
{
    Continue,
    SaveState,
    LoadState,
    Quit
};


struct JoypadState
{
    bool A;
    bool B;
    bool Select;
    bool Start;
    bool Up;
    bool Down;
    bool Left;
    bool Right;

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

// Input Interface
class IInput
{
public:
    virtual InputResult CheckInput(JoypadState* state) = 0;
};