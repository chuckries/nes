#include "stdafx.h"
#include "diassembler.h"
#include "decode.h"

#if defined(TRACE)

Disassembler::Disassembler(u16 PC, IMem* mem)
    : _PC(PC)
    , _mem(mem)
    , _pInstr(nullptr)
{
}

Disassembler::~Disassembler()
{
}

void Disassembler::Disassemble(DisassembledInstruction& disassembledInstruction)
{
    disassembledInstruction.Reset();

    u8 op = LoadBBumpPC();
    disassembledInstruction._bytes.push_back(op);

    _pInstr = &disassembledInstruction;
    DECODE(op)

}

#endif