#pragma once

#include "stdafx.h"
#include "input.h"

Joypad::Joypad()
    : _nextRead(0)
{
}

Joypad::~Joypad()
{
}

bool Joypad::load()
{
    u8 currentRead = _nextRead;
    _nextRead++;
    if (_nextRead == 8)
    {
        _nextRead = 0;
    }
    switch (currentRead)
    {
    case 0:
        return State.A;
    case 1:
        return State.B;
    case 2:
        return State.Select;
    case 3:
        return State.Start;
    case 4:
        return State.Up;
    case 5:
        return State.Down;
    case 6:
        return State.Left;
    case 7: 
        return State.Right;
    default:
        return false;
    }
}

void Joypad::store(u8 val)
{
    _nextRead = 0;
}

//void Joypad::HandleKeyPress(JoypadButton button, bool isDown)
//{
//    if (isDown)
//    {
//        _state |= (u8)button;
//    }
//    else
//    {
//        _state &= ~((u8)button);
//    }
//}

Input::Input()
{
    //SDL_InitSubSystem(SDL_INIT_EVENTS);
}

Input::~Input()
{
}

u8 Input::loadb(u16 addr)
{
    if (addr == 0x4016)
    {
        return (u8)Joypad0.load();
    }
    return 0;
}

void Input::storeb(u16 addr, u8 val)
{
    if (addr == 0x4016)
    {
        Joypad0.store(val);
    }
}

//void Input::HandleKeyPress(SDL_Keycode code, bool isDown)
//{
//    switch (code)
//    {
//    case SDLK_LALT:
//    case SDLK_s:
//        _joypad.HandleKeyPress(JoypadButton::A, isDown);
//        break;
//    case SDLK_LCTRL:
//    case SDLK_a:
//        _joypad.HandleKeyPress(JoypadButton::B, isDown);
//        break;
//    case SDLK_RSHIFT:
//    case SDLK_LSHIFT:
//    case SDLK_BACKSLASH:
//        _joypad.HandleKeyPress(JoypadButton::Select, isDown);
//        break;
//    case SDLK_RETURN:
//        _joypad.HandleKeyPress(JoypadButton::Start, isDown);
//        break;
//    case SDLK_UP:
//        _joypad.HandleKeyPress(JoypadButton::Up, isDown);
//        break;
//    case SDLK_DOWN:
//        _joypad.HandleKeyPress(JoypadButton::Down, isDown);
//        break;
//    case SDLK_LEFT:
//        _joypad.HandleKeyPress(JoypadButton::Left, isDown);
//        break;
//    case SDLK_RIGHT:
//        _joypad.HandleKeyPress(JoypadButton::Right, isDown);
//        break;
//    }
//}

//InputResult Input::CheckInput()
//{
//    SDL_Event event;
//    while (SDL_PollEvent(&event))
//    {
//        if (event.type == SDL_KEYDOWN)
//        {
//            if (event.key.keysym.sym == SDLK_F1)
//            {
//                return InputResult::SaveState;
//            }
//            else if (event.key.keysym.sym == SDLK_F2)
//            {
//                return InputResult::LoadState;
//            }
//            else if (event.key.keysym.sym == SDLK_ESCAPE)
//            {
//                return InputResult::Quit;
//            }
//            else
//            {
//                HandleKeyPress(event.key.keysym.sym, true);
//            }
//        }
//        else if (event.type == SDL_KEYUP)
//        {
//            HandleKeyPress(event.key.keysym.sym, false);
//        }
//        else if (event.type == SDL_WINDOWEVENT)
//        {
//            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
//            {
//                return InputResult::Quit;
//            }
//        }
//    }
//    return InputResult::Continue;
//}
