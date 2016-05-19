#pragma once

#if defined(TRACE)

#include "mem.h"

#include <vector>

class DisassembledInstruction 
{
public:
    DisassembledInstruction() 
        : _ss(std::ios_base::out | std::ios_base::ate)
    { 
    }
    ~DisassembledInstruction() { }

    std::string GetFormattedBytes()
    {
        std::stringstream ss;
        ss << std::uppercase << std::hex;

        std::vector<u8>::iterator it = _bytes.begin();
        do
        {
            ss << std::setfill('0') << std::setw(2) << (i32)*it << ' ';
            it++;
        } while (it != _bytes.end());

        return ss.str();
    }

    std::string GetDisassemblyString()
    {
        return _ss.str();
    }

    void Reset()
    {
        _ss.clear();
    }

private:
    friend class Disassembler;
    std::stringstream _ss;
    std::vector<u8> _bytes;
};


class Disassembler
{
public:
    Disassembler(u16 PC, IMem* mem);
    ~Disassembler();

    void Disassemble(DisassembledInstruction& disassembledInstruction);

private:
    u16 _PC;
    NPtr<IMem> _mem;
    DisassembledInstruction* _pInstr;

    // Helpers
    u8 LoadBBumpPC()
    {
        return _mem->loadb(_PC++);
    }

    u16 LoadWBumpPC()
    {
        u16 val = _mem->loadw(_PC);
        _PC += 2;
        return val;
    }

    u16 GetBranchTarget()
    {
        i8 disp = (i8)LoadBBumpPC();
        return (u16)((i32)_PC + (i32)disp);
    }

    std::string DisBBumpPC()
    {
        std::stringstream ss;
        ss << '$' << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)LoadBBumpPC();
        return ss.str();
    }

    std::string DisWBumpPC()
    {
        std::stringstream ss;
        ss << '$' << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << (int)LoadWBumpPC();
        return ss.str();
    }

    std::string DisBranchTarget()
    {
        std::stringstream ss;
        ss << '$' << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << (int)GetBranchTarget();
        return ss.str();
    }

    u8 PeekPC(int offset = 0)
    {
        return _mem->loadb(_PC + offset);
    }

    // Addressing Modes
    void Immediate() 
    { 
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_ss << '#' << DisBBumpPC();
    }

    void Accumulator() { }

    void ZeroPage() 
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_ss << DisBBumpPC();
    }

    void ZeroPageX() 
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_ss << DisBBumpPC() << ",X";
    }

    void ZeroPageY() 
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_ss << DisBBumpPC() << ",Y";
    }

    void Absolute() 
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_bytes.push_back(PeekPC(1));
        _pInstr->_ss << DisWBumpPC();
    }

    void AbsoluteX() 
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_bytes.push_back(PeekPC(1));
        _pInstr->_ss << DisWBumpPC() << ",X";
    }

    void AbsoluteY()
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_bytes.push_back(PeekPC(1));
        _pInstr->_ss << DisWBumpPC() << ",Y";
    }

    void IndexedIndirectX() 
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_ss << '(' << DisBBumpPC() << ",X)";
    }

    void IndirectIndexedY() 
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_ss << '(' << DisBBumpPC() << "),Y";
    }

    // Instructions
#define INSTRUCTION(codeName, displayName) \
void codeName() \
{ \
    std::string address = _pInstr->_ss.str(); \
    _pInstr->_ss.str(displayName); \
    _pInstr->_ss << ' ' << address; \
} 

#define IMPLIED(codeName, displayName) \
void codeName() { \
    _pInstr->_ss << displayName; \
}

#define BRANCH(codeName, displayName) \
void codeName() \
{ \
    _pInstr->_bytes.push_back(PeekPC()); \
    _pInstr->_ss << displayName << ' ' << DisBranchTarget(); \
}

    // Loads
    INSTRUCTION(lda, "LDA")
    INSTRUCTION(ldx, "LDX")
    INSTRUCTION(ldy, "LDY")

    // Stores
    INSTRUCTION(sta, "STA")
    INSTRUCTION(stx, "STX")
    INSTRUCTION(sty, "STY")

    // Arithmetic
    INSTRUCTION(adc, "ADC")
    INSTRUCTION(sbc, "SBD")

    // Comparisons
    INSTRUCTION(cmp, "CMP")
    INSTRUCTION(cpx, "CPX")
    INSTRUCTION(cpy, "CPY")

    // Bitwise Operations
    INSTRUCTION(and, "AND")
    INSTRUCTION(ora, "ORA")
    INSTRUCTION(eor, "EOR")
    INSTRUCTION(bit, "BIT")

    // Shifts and Rotates
    INSTRUCTION(rol, "ROL")
    INSTRUCTION(ror, "ROR")
    INSTRUCTION(asl, "ASL")
    INSTRUCTION(lsr, "LSR")

    // Increments and Decrements
    INSTRUCTION(inc, "INC")
    INSTRUCTION(dec, "DEC")
    IMPLIED(inx, "INX")
    IMPLIED(dex, "DEX")
    IMPLIED(iny, "INY")
    IMPLIED(dey, "DEY")

    // Register Moves
    IMPLIED(tax, "TAX")
    IMPLIED(tay, "TAY")
    IMPLIED(txa, "TXA")
    IMPLIED(tya, "TYA")
    IMPLIED(txs, "TXS")
    IMPLIED(tsx, "TSX")

    // Flag Operations
    IMPLIED(clc, "CLC")
    IMPLIED(sec, "SEC")
    IMPLIED(cli, "CLI")
    IMPLIED(sei, "SEI")
    IMPLIED(clv, "CLV")
    IMPLIED(cld, "CLD")
    IMPLIED(sed, "SED")

    // Branches
    BRANCH(bpl, "BPL")
    BRANCH(bmi, "BMI")
    BRANCH(bvc, "BVC")
    BRANCH(bvs, "BVS")
    BRANCH(bcc, "BCC")
    BRANCH(bcs, "BCS")
    BRANCH(bne, "BNE")
    BRANCH(beq, "BEQ")

    // Jumps
    void jmp()
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_bytes.push_back(PeekPC(1));

        _pInstr->_ss << "JMP " << DisWBumpPC();
    }

    void jmpi()
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_bytes.push_back(PeekPC(1));

        u16 val = _mem->loadw(LoadWBumpPC());

        _pInstr->_ss << "JMP ($" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << val << ')';
    }

    // Procedure Calls
    void jsr()
    {
        _pInstr->_bytes.push_back(PeekPC());
        _pInstr->_bytes.push_back(PeekPC(1));

        _pInstr->_ss << "JSR " << DisWBumpPC();
    }
    IMPLIED(rts, "RTS")
    IMPLIED(brk, "BRK")
    IMPLIED(rti, "RTI")

    // Stack Operations
    IMPLIED(pha, "PHA")
    IMPLIED(pla, "PLA")
    IMPLIED(php, "PHP")
    IMPLIED(plp, "PLP")

    // No Operation
    IMPLIED(nop, "NOP")
};

#endif