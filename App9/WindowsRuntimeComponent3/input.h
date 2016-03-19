#pragma once

#include "mem.h"

//#include <SDL_events.h>

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
        : A(false)
        , B(false)
        , Select(false)
        , Start(false)
        , Up(false)
        , Down(false)
        , Left(false)
        , Right(false)
    {
    }

    void Set(const JoypadState& state)
    {
        A = state.A;
        B = state.B;
        Select = state.Select;
        Start = state.Start;
        Up = state.Up;
        Down = state.Down;
        Left = state.Left;
        Right = state.Right;
    }
};

//class IInput
//{
//public:
//    virtual InputResult CheckInput(JoypadState& state) = 0;
//};
//
//class SdlInput : IInput
//{
//public:
//    SdlInput();
//    ~SdlInput();
//
//    InputResult CheckInput(JoypadState& state);
//
//private:
//    void HandleKey(SDL_Keycode key, JoypadState& state, bool pressed);
//};

class Input : public IMem
{
public:
    Input();
    ~Input();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    // ISaveState
    // don't bother saving the state of input as it will be read again after 
    // the save state is loaded and before the next cpu instruction
    void Save() {}
    void Load() {}

public:
    JoypadState State;

private:
    int _nextRead;
};
