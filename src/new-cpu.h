#pragma once

#include "interfaces.h"

class MemoryMap;

// CPU Interrupt Vectors
const u16 NMI_VECTOR = 0xfffa;
const u16 RESET_VECTOR = 0xfffc;
const u16 IRQ_VECTOR = 0xfffe;

// CPU Status Register Flags
enum class Flag : u8
{
    Carry = 1 << 0,
    Zero = 1 << 1,
    IRQ = 1 << 2,
    Decimal = 1 << 3,
    Break = 1 << 4,
    Unused = 1 << 5,
    Overflow = 1 << 6,
    Negative = 1 << 7
};

union Pair
{
    u16 W;
    struct
    {
        u8 L;
        u8 H;
    } B;
};


struct CpuRegs
{
    u8 A;    // Accumulator
    u8 X;    // Index Register
    u8 Y;    // Index Register
    u8 P;    // Process Status
    u8 SP;   // Stack Pointer

    Pair PC; // Program Counter

    CpuRegs()
    {
        Reset(true);
    }

    void Reset(bool hard)
    {
        if (hard)
        {
            A = 0;
            X = 0;
            Y = 0;
            P = ((u8)Flag::Unused | (u8)Flag::IRQ); // DECIMAL_FLAG is always set on nes and bit 5 is unused, always set
            SP = 0xfd; // Startup value according to http://wiki.nesdev.com/w/index.php/CPU_power_up_state
            PC.W = 0x8000;
        }
        else
        {
            // TODO
        }
    }

    bool GetFlag(Flag flag)
    {
        return (P & (u8)flag) != 0;
    }

    void SetFlag(Flag flag, bool on)
    {
        if (on)
        {
            P |= (u8)flag;
        }
        else
        {
            P &= ~((u8)flag);
        }
    }
    u8 SetZN(u8 val)
    {
        SetFlag(Flag::Zero, val == 0);
        SetFlag(Flag::Negative, (val & 0x80) != 0);
        return val;
    }
};

enum class InstrType
{
    BRK,
    NMI,
    IRQ,
    RTI,
    RTS,
    PHAorPHP,
    PLAorPLP,
    JSR,
    Implied,
    Accumulator,
    Immediate,
    AbsoluteJmp,
    AbsoluteRead,
    AbsoluteRMW,
    AbsoluteWrite,
    ZPRead,
    ZPRMW,
    ZPWRite,
    ZPIndexedRead,
    ZPIndexedRMW,
    ZPIndexedWrite,
    AbsoluteIndexedRead,
    AbsoluteIndexedRMW,
    AbsoluteIndexedWrite,
    Relative,
    IndexedIndirectRead,
    IndexedIndirectRMW,
    IndexedIndirectWrite,
    IndirectIndexedRead,
    IndirectIndexedRMW,
    IndirectIndexedWrite,
    AbsoluteIndirectJmp,

    NONE
};

class NewCpu : public IMem, public NesObject
{
public:
    NewCpu(MemoryMap* memoryMap);
    virtual ~NewCpu();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

    DELEGATE_NESOBJECT_REFCOUNTING();

    void Reset(bool hard);
    void Step();
    void Nmi();
    void Irq();
    void StealCycles(u32 cycles) { _stealCycles += cycles; }
    bool IsDmaRunning() { return _dmaCycles > 0; }

private:
    void Decode();
    void EndInstr();

    // Memory Acess Helpers
    u8 LoadBBumpPC() { return loadb(_regs.PC.W++); }

    // Stack Helpers
    void PushB(u8 val)
    {
        storeb(0x100 | (u16)_regs.SP, val);
        _regs.SP--;
    }

private:
    CpuRegs _regs;
    NPtr<MemoryMap> _mem;

    // state machine
    u8 _cycle;
    u8 _op;
    InstrType _type;
    void(NewCpu::*_action)();
    u8 _value;
    Pair _addr;
    bool _doBranch;
    bool _pageCross;
    u8* _pIdxReg;

    bool _wantNmi;
    bool _wantIrq;
    u32 _dmaCycles;
    u16 _dmaAddr;
    u32 _stealCycles;

private:
    // Actions

    // loads
    void lda() { _regs.A = _regs.SetZN(_value); }
    void ldx() { _regs.X = _regs.SetZN(_value); }
    void ldy() { _regs.Y = _regs.SetZN(_value); }

    // stores
    void sta() { storeb(_addr.W, _regs.A); }
    void stx() { storeb(_addr.W, _regs.X); }
    void sty() { storeb(_addr.W, _regs.Y); }

    // Arithemtic
    void adc()
    {
        u8 val = _value;
        u32 result = (u32)_regs.A + (u32)val;
        if (_regs.GetFlag(Flag::Carry)) result += 1;
        _regs.SetFlag(Flag::Carry, (result & 0x100) != 0);

        u8 resultByte = result & 0xff;
        u8 a = _regs.A;
        _regs.SetFlag(Flag::Overflow, (((a ^ val) & 0x80) == 0) && (((a ^ resultByte) & 0x80) == 0x80));
        _regs.A = _regs.SetZN(resultByte);
    }

    void sbc()
    {
        u8 val = _value;
        u32 result = (u32)_regs.A - (u32)val;
        if (!_regs.GetFlag(Flag::Carry)) result -= 1;
        _regs.SetFlag(Flag::Carry, (result & 0x100) == 0);

        u8 resultByte = result & 0xff;
        u8 a = _regs.A;
        _regs.SetFlag(Flag::Overflow, (((a ^ resultByte) & 0x80) != 0) && ((a ^ val) & 0x80) == 0x80);
        _regs.A = _regs.SetZN(resultByte);
    }

    // Comparisons
    void cmp_base(u8 val)
    {
        u32 result = (u32)val - (u32)_value;
        _regs.SetFlag(Flag::Carry, (result & 0x100) == 0);
        _regs.SetZN((u8)result);
    }
    void cmp() { cmp_base(_regs.A); }
    void cpx() { cmp_base(_regs.X); }
    void cpy() { cmp_base(_regs.Y); }

    // Bitwise Operations
    void and() { _regs.A = _regs.SetZN(_regs.A & _value); }
    void ora() { _regs.A = _regs.SetZN(_regs.A | _value); }
    void eor() { _regs.A = _regs.SetZN(_regs.A ^ _value); }
    void bit()
    {
        u8 val = _value;
        _regs.SetFlag(Flag::Zero, (val &_regs.A) == 0);
        _regs.SetFlag(Flag::Negative, (val & (1 << 7)) != 0);
        _regs.SetFlag(Flag::Overflow, (val & (1 << 6)) != 0);
    }

    // Shifts and Rotates
    void shl_base(bool lsb)
    {
        u8 val = _value;
        bool newCarry = (val & 0x80) != 0;
        u8 result = (val << 1) | (lsb ? 1 : 0);
        _regs.SetFlag(Flag::Carry, newCarry);
        _value = _regs.SetZN(result);
    }
    void shr_base(bool msb)
    {
        u8 val = _value;
        bool newCarry = (val & 0x01) != 0;
        u8 result = (val >> 1) | (msb ? 0x80 : 0);
        _regs.SetFlag(Flag::Carry, newCarry);
        _value = _regs.SetZN(result);
    }
    void rol()
    {
        bool oldCarry = _regs.GetFlag(Flag::Carry);
        shl_base(oldCarry);
    }
    void ror()
    {
        bool oldCarry = _regs.GetFlag(Flag::Carry);
        shr_base(oldCarry);
    }
    void asl() { shl_base(false); }
    void lsr() { shr_base(false); }

    // Increments and Decrements
    void inc() { _value = _regs.SetZN(_value + 1); }
    void dec() { _value = _regs.SetZN(_value - 1); }
    void inx() { _regs.SetZN(++_regs.X); }
    void dex() { _regs.SetZN(--_regs.X); }
    void iny() { _regs.SetZN(++_regs.Y); }
    void dey() { _regs.SetZN(--_regs.Y); }

    // Register Moves
    void tax() { _regs.X = _regs.SetZN(_regs.A); }
    void tay() { _regs.Y = _regs.SetZN(_regs.A); }
    void txa() { _regs.A = _regs.SetZN(_regs.X); }
    void tya() { _regs.A = _regs.SetZN(_regs.Y); }
    void txs() { _regs.SP = _regs.X; }
    void tsx() { _regs.X = _regs.SetZN(_regs.SP); }

    // flags
    void clc() { _regs.SetFlag(Flag::Carry, false); }
    void sec() { _regs.SetFlag(Flag::Carry, true); }
    void cli() { _regs.SetFlag(Flag::IRQ, false); }
    void sei() { _regs.SetFlag(Flag::IRQ, true); }
    void clv() { _regs.SetFlag(Flag::Overflow, false); }
    void cld() { _regs.SetFlag(Flag::Decimal, false); }
    void sed() { _regs.SetFlag(Flag::Decimal, true); }

    // branches
    void bpl() { _doBranch = !_regs.GetFlag(Flag::Negative); }
    void bmi() { _doBranch = _regs.GetFlag(Flag::Negative); }
    void bvc() { _doBranch = !_regs.GetFlag(Flag::Overflow); }
    void bvs() { _doBranch = _regs.GetFlag(Flag::Overflow); }
    void bcc() { _doBranch = !_regs.GetFlag(Flag::Carry); }
    void bcs() { _doBranch = _regs.GetFlag(Flag::Carry); }
    void bne() { _doBranch = !_regs.GetFlag(Flag::Zero); }
    void beq() { _doBranch = _regs.GetFlag(Flag::Zero); }

    // no operation
    void nop() {}

#if defined(TRACE)
    void Trace();
#endif
};