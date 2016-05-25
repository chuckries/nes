#include "stdafx.h"
#include "new-cpu.h"
#include "mem.h"
#include "diassembler.h"

NewCpu::NewCpu(
    MemoryMap* memoryMap
)
    : _mem(memoryMap)
{
    Reset(true);
}

NewCpu::~NewCpu()
{

}

u8 NewCpu::loadb(u16 addr)
{
    return _mem->loadb(addr);
}

void NewCpu::storeb(u16 addr, u8 val)
{
    if (addr == 0x4014)
    {
        _dmaCycles = 512;
        _dmaAddr = ((u16)val) << 8;
    }
    else
    {
        _mem->storeb(addr, val);
    }
}

void NewCpu::SaveState(std::ofstream& ofs)
{
    // TODO
}

void NewCpu::LoadState(std::ifstream& ifs)
{
    // TODO
}

void NewCpu::Reset(bool hard)
{
    if (hard)
    {
        _cycle = 0;
        _op = 0;
        _type = InstrType::NONE;
        _action = nullptr;
        _value = 0;
        _addr.W = 0;
        _doBranch = false;
        _pageCross = false;
        _pIdxReg = nullptr;

        _wantNmi = false;
        _wantIrq = false;
        _dmaCycles = 0;
        _dmaAddr = 0;
        _stealCycles = 0;

        _regs.Reset(hard);
        _regs.PC.W = loadw(RESET_VECTOR);
    }
}

void NewCpu::Step()
{
    if (_stealCycles > 0)
    {
        _stealCycles--;
    }
    else if (_dmaCycles > 0)
    {
        if (_dmaCycles % 2 == 0)
        {
            _value = loadb(_dmaAddr++);
        }
        else
        {
            storeb(0x2004, _value);
        }
        _dmaCycles--;
        if (_dmaCycles == 0)
        {
            _dmaAddr = 0;
            EndInstr();
        }
    }
    else if (_cycle == 0)
    {
        if (_wantNmi)
        {
            _type = InstrType::NMI;
            _wantNmi = false;
            _cycle = 1;
        }
        else if (_wantIrq && !_regs.GetFlag(Flag::IRQ))
        {
            _type = InstrType::IRQ;
            _wantIrq = false;
            _cycle = 1;
        }
        else
        {
#if defined(TRACE)
            Trace();
#endif
            _op = LoadBBumpPC();
            Decode();
            _cycle = 1;
        }
    }
    else
    {
        switch (_type)
        {
        case InstrType::BRK:
            switch (_cycle)
            {
            case 1: LoadBBumpPC(); _cycle++; break;
            case 2: PushB(_regs.PC.B.H); _cycle++; break;
            case 3: PushB(_regs.PC.B.L); _cycle++; break;
            case 4: PushB(_regs.P | (u8)Flag::Break | (u8)Flag::Unused); _cycle++; break;
            case 5: _regs.PC.B.L = loadb(IRQ_VECTOR); _cycle++; break;
            case 6: _regs.PC.B.H = loadb(IRQ_VECTOR + 1); _regs.SetFlag(Flag::IRQ, true); EndInstr(); break;
            }
            break;
        case InstrType::NMI:
            switch (_cycle)
            {
            case 1: _cycle++; break;
            case 2: PushB(_regs.PC.B.H); _cycle++; break;
            case 3: PushB(_regs.PC.B.L); _cycle++; break;
            case 4: PushB(_regs.P); _cycle++; break;
            case 5: _regs.PC.B.L = loadb(NMI_VECTOR); _cycle++; break;
            case 6: _regs.PC.B.H = loadb(NMI_VECTOR + 1); EndInstr(); break;
            }
            break;
        case InstrType::IRQ:
            switch (_cycle)
            {
            case 1: _cycle++; break;
            case 2: PushB(_regs.PC.B.H); _cycle++; break;
            case 3: PushB(_regs.PC.B.L); _cycle++; break;
            case 4: PushB(_regs.P); _cycle++; break;
            case 5: _regs.PC.B.L = loadb(IRQ_VECTOR); _cycle++; break;
            case 6: _regs.PC.B.H = loadb(IRQ_VECTOR + 1); _regs.SetFlag(Flag::IRQ, true); EndInstr(); break;
            }
            break;
        case InstrType::RTI:
            switch (_cycle)
            {
            case 1: loadb(_regs.PC.W); _cycle++; break;
            case 2: _regs.SP++; _cycle++; break;
            case 3: _regs.P = loadb(0x100 | _regs.SP++); _cycle++; break;
            case 4: _regs.PC.B.L = loadb(0x100 | _regs.SP++); _cycle++; break;
            case 5: _regs.PC.B.H = loadb(0x100 | _regs.SP); EndInstr(); break;
            }
            break;
        case InstrType::RTS:
            switch (_cycle)
            {
            case 1: loadb(_regs.PC.W); _cycle++; break;
            case 2: _regs.SP++; _cycle++; break;
            case 3: _regs.PC.B.L = loadb(0x100 | _regs.SP++); _cycle++; break;
            case 4: _regs.PC.B.H = loadb(0x100 | _regs.SP); _cycle++; break;
            case 5: _regs.PC.W++; EndInstr(); break;
            }
            break;
        case InstrType::PHAorPHP:
            switch (_cycle)
            {
            case 1: loadb(_regs.PC.W); _cycle++; break;
            case 2: 
                if (_pIdxReg == &_regs.P)
                {
                    PushB(*_pIdxReg | (u8)Flag::Break | (u8)Flag::Unused);
                }
                else
                {
                    PushB(*_pIdxReg);
                }
                EndInstr();
                break;
            }
            break;
        case InstrType::PLAorPLP:
            switch (_cycle)
            {
            case 1: loadb(_regs.PC.W); _cycle++; break;
            case 2: _regs.SP++; _cycle++; break;
            case 3:
                *_pIdxReg = loadb(0x100 | _regs.SP);
                if (_pIdxReg == &_regs.A)
                {
                    _regs.SetZN(_regs.A);
                }
                EndInstr();
                break;
            }
            break;
        case InstrType::JSR:
            switch (_cycle)
            {
            case 1: _value = LoadBBumpPC(); _cycle++; break;
            case 2: _cycle++; break;
            case 3: PushB(_regs.PC.B.H); _cycle++; break;
            case 4: PushB(_regs.PC.B.L); _cycle++; break;
            case 5: _regs.PC.B.H = loadb(_regs.PC.W); _regs.PC.B.L = _value; EndInstr(); break;
            }
            break;
        case InstrType::Implied:
            loadb(_regs.PC.W); // throwaway read
            (*this.*_action)();
            EndInstr();
            break;
        case InstrType::Accumulator:
            loadb(_regs.PC.W);
            _value = _regs.A;
            (*this.*_action)();
            _regs.A = _value;
            EndInstr();
            break;
        case InstrType::Immediate:
            _value = LoadBBumpPC();
            (*this.*_action)();
            EndInstr();
            break;
        case InstrType::AbsoluteJmp:
            switch (_cycle)
            {
            case 1: _value = LoadBBumpPC(); _cycle++; break;
            case 2: _regs.PC.B.H = LoadBBumpPC(); _regs.PC.B.L = _value; EndInstr(); break;
            }
            break;
        case InstrType::AbsoluteRead:
            switch (_cycle)
            {
            case 1: _addr.B.L = LoadBBumpPC(); _cycle++; break;
            case 2: _addr.B.H = LoadBBumpPC(); _cycle++; break;
            case 3:
                _value = loadb(_addr.W);
                (*this.*_action)();
                EndInstr();
                break;
            }
            break;
        case InstrType::AbsoluteRMW:
            switch (_cycle)
            {
            case 1: _addr.B.L = LoadBBumpPC(); _cycle++; break;
            case 2: _addr.B.H = LoadBBumpPC(); _cycle++; break;
            case 3: _value = loadb(_addr.W); _cycle++; break;
            case 4: storeb(_addr.W, _value); (*this.*_action)(); _cycle++; break;
            case 5: storeb(_addr.W, _value); EndInstr(); break;
            }
            break;
        case InstrType::AbsoluteWrite:
            switch (_cycle)
            {
            case 1: _addr.B.L = LoadBBumpPC(); _cycle++; break;
            case 2: _addr.B.H = LoadBBumpPC(); _cycle++; break;
            case 3: 
                (*this.*_action)();
                EndInstr();
                break;
            }
            break;
        case InstrType::ZPRead:
            switch (_cycle)
            {
            case 1: _addr.W = (u16)LoadBBumpPC(); _cycle++; break;
            case 2: _value = loadb(_addr.W); (*this.*_action)(); EndInstr(); break;
            }
            break;
        case InstrType::ZPRMW:
            switch (_cycle)
            {
            case 1: _addr.W = (u16)LoadBBumpPC(); _cycle++; break;
            case 2: _value = loadb(_addr.W); _cycle++; break;
            case 3: storeb(_addr.W, _value); (*this.*_action)(); _cycle++; break;
            case 4: storeb(_addr.W, _value); EndInstr(); break;
            }
            break;
        case InstrType::ZPWRite:
            switch (_cycle)
            {
            case 1: _addr.W = (u16)LoadBBumpPC(); _cycle++; break;
            case 2: (*this.*_action)(); EndInstr(); break;
            }
            break;
        case InstrType::ZPIndexedRead:
            switch (_cycle)
            {
            case 1: _addr.W = (u16)LoadBBumpPC(); _cycle++; break;
            case 2: loadb(_addr.W); _addr.B.L = (u8)(_addr.B.L + *_pIdxReg); _cycle++; break;
            case 3: _value = loadb(_addr.W); (*this.*_action)(); EndInstr(); break;
            }
            break;
        case InstrType::ZPIndexedRMW:
            switch (_cycle)
            {
            case 1: _addr.W = (u16)LoadBBumpPC(); _cycle++; break;
            case 2: loadb(_addr.W); _addr.B.L = (u8)(_addr.B.L + _regs.X); _cycle++; break;
            case 3: _value = loadb(_addr.W); _cycle++; break;
            case 4: storeb(_addr.W, _value); (*this.*_action)(); _cycle++; break;
            case 5: storeb(_addr.W, _value); EndInstr(); break;
            }
            break;
        case InstrType::ZPIndexedWrite:
            switch (_cycle)
            {
            case 1: _addr.W = (u16)LoadBBumpPC(); _cycle++; break;
            case 2: loadb(_addr.W); _addr.B.L = (u8)(_addr.B.L + *_pIdxReg); _cycle++; break;
            case 3: (*this.*_action)(); EndInstr(); break;
            }
            break;
        case InstrType::AbsoluteIndexedRead:
            switch (_cycle)
            {
            case 1: _addr.B.L = LoadBBumpPC(); _cycle++; break;
            case 2:
                _addr.B.H = LoadBBumpPC();
                Pair newAddr;
                newAddr.W = (u16)_addr.B.L + (u16)*_pIdxReg;
                _addr.B.L = newAddr.B.L;
                _pageCross = newAddr.B.H != 0;
                _cycle++;
                break;
            case 3:
                _value = loadb(_addr.W);
                if (_pageCross)
                {
                    _addr.B.H++;
                    _cycle++;
                }
                else
                {
                    (*this.*_action)();
                    EndInstr();
                }
                break;
            case 4:
                _value = loadb(_addr.W);
                (*this.*_action)();
                EndInstr();
                break;
            }
            break;
        case InstrType::AbsoluteIndexedRMW:
            switch (_cycle)
            {
            case 1: _addr.B.L = LoadBBumpPC(); _cycle++; break;
            case 2:
                _addr.B.H = LoadBBumpPC();
                Pair newAddr;
                newAddr.W = (u16)_addr.B.L + (u16)_regs.X;
                _addr.B.L = newAddr.B.L;
                _pageCross = newAddr.B.H != 0;
                _cycle++;
                break;
            case 3:
                loadb(_addr.W);
                if (_pageCross)
                {
                    _addr.B.H++;
                }
                _cycle++;
                break;
            case 4: _value = loadb(_addr.W); _cycle++; break;
            case 5: storeb(_addr.W, _value); (*this.*_action)(); _cycle++; break;
            case 6: storeb(_addr.W, _value); EndInstr(); break;
            }
            break;
        case InstrType::AbsoluteIndexedWrite:
            switch (_cycle)
            {
            case 1: _addr.B.L = LoadBBumpPC(); _cycle++; break;
            case 2: 
                _addr.B.H = LoadBBumpPC();
                Pair newAddr;
                newAddr.W = (u16)_addr.B.L + (u16)*_pIdxReg;
                _addr.B.L = newAddr.B.L;
                _pageCross = newAddr.B.H != 0;
                _cycle++;
                break;
            case 3:
                loadb(_addr.W);
                if (_pageCross)
                {
                    _addr.B.H++;
                }
                _cycle++;
                break;
            case 4:
                (*this.*_action)();
                EndInstr();
                break;
            }
            break;
        case InstrType::Relative:
            switch (_cycle)
            {
            case 1: _value = LoadBBumpPC(); _cycle++; break;
            case 2: 
                {
                    u8 op = loadb(_regs.PC.W);
                    (*this.*_action)();
                    if (_doBranch)
                    {
                        Pair newPC;
                        newPC.W = (u16)((i32)(i16)_regs.PC.W + (i32)(i8)_value);
                        _regs.PC.B.L = newPC.B.L;
                        _pageCross = newPC.B.H != _regs.PC.B.H;
                        _value = newPC.B.H;
                        _cycle++;
                    }
                    else
                    {
                        EndInstr();
#if defined(TRACE)
                        Trace();
#endif
                        _op = op;
                        _regs.PC.W++;
                        Decode();
                        _cycle = 1;
                    }
                }
                break;
            case 3:
                {
                    u8 op = loadb(_regs.PC.W);
                    if (_pageCross)
                    {
                        _regs.PC.B.H = _value;
                        _cycle++;
                    }
                    else
                    {
                        EndInstr();
#if defined(TRACE)
                        Trace();
#endif
                        _op = op;
                        _regs.PC.W++;
                        Decode();
                        _cycle = 1;
                    }
                }
                break;
            case 4:
                {
#if defined(TRACE)
                    Trace();
#endif
                    _op = LoadBBumpPC();
                    Decode();
                    _cycle = 1;
                }
                break;
            }
            break;
        case InstrType::IndexedIndirectRead:
            switch (_cycle)
            {
            case 1: _value = LoadBBumpPC(); _cycle++; break;
            case 2: loadb(_value); _value += _regs.X; _cycle++; break;
            case 3: _addr.B.L = loadb(_value); _cycle++; break;
            case 4: _addr.B.H = loadb((u8)(_value + 1)); _cycle++; break;
            case 5: _value = loadb(_addr.W); (*this.*_action)();  EndInstr(); break;
            }
            break;
        case InstrType::IndexedIndirectRMW:
            switch (_cycle)
            {
            case 1: _value = LoadBBumpPC(); _cycle++; break;
            case 2: loadb(_value); _value += _regs.X; _cycle++; break;
            case 3: _addr.B.L = loadb(_value); _cycle++; break;
            case 4: _addr.B.H = loadb((u8)(_value + 1)); _cycle++; break;
            case 5: _value = loadb(_addr.W); _cycle++; break;
            case 6: storeb(_addr.W, _value); (*this.*_action)(); _cycle++; break;
            case 7: storeb(_addr.W, _value); EndInstr(); break;
            }
            break;
        case InstrType::IndexedIndirectWrite:
            switch (_cycle)
            {
            case 1: _value = LoadBBumpPC(); _cycle++; break;
            case 2: loadb(_value); _value += _regs.X; _cycle++; break;
            case 3: _addr.B.L = loadb(_value); _cycle++; break;
            case 4: _addr.B.H = loadb((u8)(_value + 1)); _cycle++; break;
            case 5: (*this.*_action)(); EndInstr(); break;
            }
            break;
        case InstrType::IndirectIndexedRead:
            switch (_cycle)
            {
            case 1: _value = LoadBBumpPC(); _cycle++; break;
            case 2: _addr.B.L = loadb(_value); _cycle++; break;
            case 3: 
                _addr.B.H = loadb((u8)(_value + 1));
                Pair newAddr;
                newAddr.W = (u16)_addr.B.L + (u16)_regs.Y;
                _addr.B.L = newAddr.B.L;
                _pageCross = newAddr.B.H != 0;
                _cycle++;
                break;
            case 4:
                _value = loadb(_addr.W);
                if (_pageCross)
                {
                    _addr.B.H++;
                    _cycle++;
                }
                else
                {
                    (*this.*_action)();
                    EndInstr();
                }
                break;
            case 5:
                _value = loadb(_addr.W);
                (*this.*_action)();
                EndInstr();
                break;
            }
            break;
        case InstrType::IndirectIndexedRMW:
            __debugbreak();
            break;
        case InstrType::IndirectIndexedWrite:
            switch (_cycle)
            {
            case 1: _value = LoadBBumpPC(); _cycle++; break;
            case 2: _addr.B.L = loadb(_value); _cycle++; break;
            case 3:
                _addr.B.H = loadb((u8)(_value + 1));
                Pair newAddr;
                newAddr.W = (u16)_addr.B.L + (u16)_regs.Y;
                _addr.B.L = newAddr.B.L;
                _pageCross = newAddr.B.H != 0;
                _cycle++;
                break;
            case 4:
                loadb(_addr.W);
                if (_pageCross)
                {
                    _addr.B.H++;
                }
                _cycle++;
                break;
            case 5:
                (*this.*_action)();
                EndInstr();
                break;
            }
            break;
        case InstrType::AbsoluteIndirectJmp:
            switch (_cycle)
            {
            case 1: _addr.B.L = LoadBBumpPC(); _cycle++; break;
            case 2: _addr.B.H = LoadBBumpPC(); _cycle++; break;
            case 3: _value = loadb(_addr.W); _cycle++; break;
            case 4: _addr.B.L = u8(_addr.B.L + 1); _regs.PC.B.L = _value; _regs.PC.B.H = loadb(_addr.W); EndInstr(); break;
            }
            break;
        default:
            __debugbreak();
        }
    }
}

void NewCpu::EndInstr()
{
    _cycle = 0;
    _op = 0;
    _type = InstrType::NONE;
    _action = nullptr;
    _value = 0;
    _addr.W = 0;
    _doBranch = false;
    _pageCross = false;
    _pIdxReg = nullptr;
}

void NewCpu::Nmi()
{
    _wantNmi = true;
}

void NewCpu::Irq()
{
    _wantIrq = !_regs.GetFlag(Flag::IRQ);
}

#if defined(TRACE)
void NewCpu::Trace()
{
    Disassembler disassembler(_regs.PC.W, _mem);

    DisassembledInstruction instruction;
    disassembler.Disassemble(instruction);

    printf("%04X %-10s %-12s A:%02x X:%02X Y:%02X P:%02X S:%02X\n",
        _regs.PC.W,
        instruction.GetFormattedBytes().c_str(),
        instruction.GetDisassemblyString().c_str(),
        _regs.A,
        _regs.X,
        _regs.Y,
        _regs.P,
        _regs.SP
    );
}
#endif

void NewCpu::Decode()
{
    switch (_op)
    {
        //loads
    case 0xa9: _type = InstrType::Immediate;            _action = &NewCpu::lda; break;
    case 0xa5: _type = InstrType::ZPRead;               _action = &NewCpu::lda; break;
    case 0xb5: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::lda; _pIdxReg = &_regs.X; break;
    case 0xad: _type = InstrType::AbsoluteRead;         _action = &NewCpu::lda; break;
    case 0xbd: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::lda; _pIdxReg = &_regs.X; break;
    case 0xb9: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::lda; _pIdxReg = &_regs.Y; break;
    case 0xa1: _type = InstrType::IndexedIndirectRead;  _action = &NewCpu::lda; break;
    case 0xb1: _type = InstrType::IndirectIndexedRead;  _action = &NewCpu::lda; break;

    case 0xa2: _type = InstrType::Immediate;            _action = &NewCpu::ldx; break;
    case 0xa6: _type = InstrType::ZPRead;               _action = &NewCpu::ldx; break;
    case 0xb6: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::ldx; _pIdxReg = &_regs.Y; break;
    case 0xae: _type = InstrType::AbsoluteRead;         _action = &NewCpu::ldx; break;
    case 0xbe: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::ldx; _pIdxReg = &_regs.Y; break;

    case 0xa0: _type = InstrType::Immediate;            _action = &NewCpu::ldy; break;
    case 0xa4: _type = InstrType::ZPRead;               _action = &NewCpu::ldy; break;
    case 0xb4: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::ldy; _pIdxReg = &_regs.X; break;
    case 0xac: _type = InstrType::AbsoluteRead;         _action = &NewCpu::ldy; break;
    case 0xbc: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::ldy; _pIdxReg = &_regs.X; break;

        //stores
    case 0x85: _type = InstrType::ZPWRite;              _action = &NewCpu::sta; break;
    case 0x95: _type = InstrType::ZPIndexedWrite;       _action = &NewCpu::sta; _pIdxReg = &_regs.X; break;
    case 0x8d: _type = InstrType::AbsoluteWrite;        _action = &NewCpu::sta; break;
    case 0x9d: _type = InstrType::AbsoluteIndexedWrite; _action = &NewCpu::sta; _pIdxReg = &_regs.X; break;
    case 0x99: _type = InstrType::AbsoluteIndexedWrite; _action = &NewCpu::sta; _pIdxReg = &_regs.Y; break;
    case 0x81: _type = InstrType::IndexedIndirectWrite; _action = &NewCpu::sta; break;
    case 0x91: _type = InstrType::IndirectIndexedWrite; _action = &NewCpu::sta; break;

    case 0x86: _type = InstrType::ZPWRite;          _action = &NewCpu::stx; break;
    case 0x96: _type = InstrType::ZPIndexedWrite;   _action = &NewCpu::stx; _pIdxReg = &_regs.Y; break;
    case 0x8e: _type = InstrType::AbsoluteWrite;    _action = &NewCpu::stx; break;

    case 0x84: _type = InstrType::ZPWRite;          _action = &NewCpu::sty; break;
    case 0x94: _type = InstrType::ZPIndexedWrite;   _action = &NewCpu::sty; _pIdxReg = &_regs.X; break;
    case 0x8c: _type = InstrType::AbsoluteWrite;    _action = &NewCpu::sty; break;

        // Arithmetic
    case 0x69: _type = InstrType::Immediate;            _action = &NewCpu::adc; break;
    case 0x65: _type = InstrType::ZPRead;               _action = &NewCpu::adc; break;
    case 0x75: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::adc; _pIdxReg = &_regs.X; break;
    case 0x6d: _type = InstrType::AbsoluteRead;         _action = &NewCpu::adc; break;
    case 0x7d: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::adc; _pIdxReg = &_regs.X; break;
    case 0x79: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::adc; _pIdxReg = &_regs.Y; break;
    case 0x61: _type = InstrType::IndexedIndirectRead;  _action = &NewCpu::adc; break;
    case 0x71: _type = InstrType::IndirectIndexedRead;  _action = &NewCpu::adc; break;

    case 0xe9: _type = InstrType::Immediate;            _action = &NewCpu::sbc; break;
    case 0xe5: _type = InstrType::ZPRead;               _action = &NewCpu::sbc; break;
    case 0xf5: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::sbc; _pIdxReg = &_regs.X; break;
    case 0xed: _type = InstrType::AbsoluteRead;         _action = &NewCpu::sbc; break;
    case 0xfd: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::sbc; _pIdxReg = &_regs.X; break;
    case 0xf9: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::sbc; _pIdxReg = &_regs.Y; break;
    case 0xe1: _type = InstrType::IndexedIndirectRead;  _action = &NewCpu::sbc; break;
    case 0xf1: _type = InstrType::IndirectIndexedRead;  _action = &NewCpu::sbc; break;

        // Comparisons
    case 0xc9: _type = InstrType::Immediate;            _action = &NewCpu::cmp; break;
    case 0xc5: _type = InstrType::ZPRead;               _action = &NewCpu::cmp; break;
    case 0xd5: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::cmp; _pIdxReg = &_regs.X; break;
    case 0xcd: _type = InstrType::AbsoluteRead;         _action = &NewCpu::cmp; break;
    case 0xdd: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::cmp; _pIdxReg = &_regs.X; break;
    case 0xd9: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::cmp; _pIdxReg = &_regs.Y; break;
    case 0xc1: _type = InstrType::IndexedIndirectRead;  _action = &NewCpu::cmp; break;
    case 0xd1: _type = InstrType::IndirectIndexedRead;  _action = &NewCpu::cmp; break;

    case 0xe0: _type = InstrType::Immediate;            _action = &NewCpu::cpx; break;
    case 0xe4: _type = InstrType::ZPRead;               _action = &NewCpu::cpx; break;
    case 0xec: _type = InstrType::AbsoluteRead;         _action = &NewCpu::cpx; break;

    case 0xc0: _type = InstrType::Immediate;            _action = &NewCpu::cpy; break;
    case 0xc4: _type = InstrType::ZPRead;               _action = &NewCpu::cpy; break;
    case 0xcc: _type = InstrType::AbsoluteRead;         _action = &NewCpu::cpy; break;

        //bitwise
    case 0x29: _type = InstrType::Immediate;            _action = &NewCpu::and; break;
    case 0x25: _type = InstrType::ZPRead;               _action = &NewCpu::and; break;
    case 0x35: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::and; _pIdxReg = &_regs.X; break;
    case 0x2d: _type = InstrType::AbsoluteRead;         _action = &NewCpu::and; break;
    case 0x3d: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::and; _pIdxReg = &_regs.X; break;
    case 0x39: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::and; _pIdxReg = &_regs.Y; break;
    case 0x21: _type = InstrType::IndexedIndirectRead;  _action = &NewCpu::and; break;
    case 0x31: _type = InstrType::IndirectIndexedRead;  _action = &NewCpu::and; break;

    case 0x09: _type = InstrType::Immediate;            _action = &NewCpu::ora; break;
    case 0x05: _type = InstrType::ZPRead;               _action = &NewCpu::ora; break;
    case 0x15: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::ora; _pIdxReg = &_regs.X; break;
    case 0x0d: _type = InstrType::AbsoluteRead;         _action = &NewCpu::ora; break;
    case 0x1d: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::ora; _pIdxReg = &_regs.X; break;
    case 0x19: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::ora; _pIdxReg = &_regs.Y; break;
    case 0x01: _type = InstrType::IndexedIndirectRead;  _action = &NewCpu::ora; break;
    case 0x11: _type = InstrType::IndirectIndexedRead;  _action = &NewCpu::ora; break;

    case 0x49: _type = InstrType::Immediate;            _action = &NewCpu::eor; break;
    case 0x45: _type = InstrType::ZPRead;               _action = &NewCpu::eor; break;
    case 0x55: _type = InstrType::ZPIndexedRead;        _action = &NewCpu::eor; _pIdxReg = &_regs.X; break;
    case 0x4d: _type = InstrType::AbsoluteRead;         _action = &NewCpu::eor; break;
    case 0x5d: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::eor; _pIdxReg = &_regs.X; break;
    case 0x59: _type = InstrType::AbsoluteIndexedRead;  _action = &NewCpu::eor; _pIdxReg = &_regs.Y; break;
    case 0x41: _type = InstrType::IndexedIndirectRead;  _action = &NewCpu::eor; break;
    case 0x51: _type = InstrType::IndirectIndexedRead;  _action = &NewCpu::eor; break;

    case 0x24: _type = InstrType::ZPRead;       _action = &NewCpu::bit; break;
    case 0x2c: _type = InstrType::AbsoluteRead; _action = &NewCpu::bit; break;

        // Shifts and Rotates
    case 0x2a: _type = InstrType::Accumulator;          _action = &NewCpu::rol; break;
    case 0x26: _type = InstrType::ZPRMW;                _action = &NewCpu::rol; break;
    case 0x36: _type = InstrType::ZPIndexedRMW;         _action = &NewCpu::rol; break;
    case 0x2e: _type = InstrType::AbsoluteRMW;          _action = &NewCpu::rol; break;
    case 0x3e: _type = InstrType::AbsoluteIndexedRMW;   _action = &NewCpu::rol; break;

    case 0x6a: _type = InstrType::Accumulator;          _action = &NewCpu::ror; break;
    case 0x66: _type = InstrType::ZPRMW;                _action = &NewCpu::ror; break;
    case 0x76: _type = InstrType::ZPIndexedRMW;         _action = &NewCpu::ror; break;
    case 0x6e: _type = InstrType::AbsoluteRMW;          _action = &NewCpu::ror; break;
    case 0x7e: _type = InstrType::AbsoluteIndexedRMW;   _action = &NewCpu::ror; break;

    case 0x0a: _type = InstrType::Accumulator;          _action = &NewCpu::asl; break;
    case 0x06: _type = InstrType::ZPRMW;                _action = &NewCpu::asl; break;
    case 0x16: _type = InstrType::ZPIndexedRMW;         _action = &NewCpu::asl; break;
    case 0x0e: _type = InstrType::AbsoluteRMW;          _action = &NewCpu::asl; break;
    case 0x1e: _type = InstrType::AbsoluteIndexedRMW;   _action = &NewCpu::asl; break;

    case 0x4a: _type = InstrType::Accumulator;          _action = &NewCpu::lsr; break;
    case 0x46: _type = InstrType::ZPRMW;                _action = &NewCpu::lsr; break;
    case 0x56: _type = InstrType::ZPIndexedRMW;         _action = &NewCpu::lsr; break;
    case 0x4e: _type = InstrType::AbsoluteRMW;          _action = &NewCpu::lsr; break;
    case 0x5e: _type = InstrType::AbsoluteIndexedRMW;   _action = &NewCpu::lsr; break;

        // increments and decrements
    case 0xe6: _type = InstrType::ZPRMW;                _action = &NewCpu::inc; break;
    case 0xf6: _type = InstrType::ZPIndexedRMW;         _action = &NewCpu::inc; break;
    case 0xee: _type = InstrType::AbsoluteRMW;          _action = &NewCpu::inc; break;
    case 0xfe: _type = InstrType::AbsoluteIndexedRMW;   _action = &NewCpu::inc; break;

    case 0xc6: _type = InstrType::ZPRMW;                _action = &NewCpu::dec; break;
    case 0xd6: _type = InstrType::ZPIndexedRMW;         _action = &NewCpu::dec; break;
    case 0xce: _type = InstrType::AbsoluteRMW;          _action = &NewCpu::dec; break;
    case 0xde: _type = InstrType::AbsoluteIndexedRMW;   _action = &NewCpu::dec; break;

    case 0xe8: _type = InstrType::Implied; _action = &NewCpu::inx; break;
    case 0xca: _type = InstrType::Implied; _action = &NewCpu::dex; break;
    case 0xc8: _type = InstrType::Implied; _action = &NewCpu::iny; break;
    case 0x88: _type = InstrType::Implied; _action = &NewCpu::dey; break;

        // Register Moves
    case 0xaa: _type = InstrType::Implied; _action = &NewCpu::tax; break;
    case 0xa8: _type = InstrType::Implied; _action = &NewCpu::tay; break;
    case 0x8a: _type = InstrType::Implied; _action = &NewCpu::txa; break;
    case 0x98: _type = InstrType::Implied; _action = &NewCpu::tya; break;
    case 0x9a: _type = InstrType::Implied; _action = &NewCpu::txs; break;
    case 0xba: _type = InstrType::Implied; _action = &NewCpu::tsx; break;

        // Flag Operations
    case 0x18: _type = InstrType::Implied; _action = &NewCpu::clc; break;
    case 0x38: _type = InstrType::Implied; _action = &NewCpu::sec; break;
    case 0x58: _type = InstrType::Implied; _action = &NewCpu::cli; break;
    case 0x78: _type = InstrType::Implied; _action = &NewCpu::sei; break;
    case 0xb8: _type = InstrType::Implied; _action = &NewCpu::clv; break;
    case 0xd8: _type = InstrType::Implied; _action = &NewCpu::cld; break;
    case 0xf8: _type = InstrType::Implied; _action = &NewCpu::sed; break;

        // Branches
    case 0x10: _type = InstrType::Relative; _action = &NewCpu::bpl; break;
    case 0x30: _type = InstrType::Relative; _action = &NewCpu::bmi; break;
    case 0x50: _type = InstrType::Relative; _action = &NewCpu::bvc; break;
    case 0x70: _type = InstrType::Relative; _action = &NewCpu::bvs; break;
    case 0x90: _type = InstrType::Relative; _action = &NewCpu::bcc; break;
    case 0xb0: _type = InstrType::Relative; _action = &NewCpu::bcs; break;
    case 0xd0: _type = InstrType::Relative; _action = &NewCpu::bne; break;
    case 0xf0: _type = InstrType::Relative; _action = &NewCpu::beq; break;

    case 0x4c: _type = InstrType::AbsoluteJmp; break;
    case 0x6c: _type = InstrType::AbsoluteIndirectJmp; break;

        // Procdeure Class
    case 0x20: _type = InstrType::JSR; break;
    case 0x60: _type = InstrType::RTS; break;
    case 0x00: _type = InstrType::BRK; break;
    case 0x40: _type = InstrType::RTI; break;

        // Stack Operations
    case 0x48: _type = InstrType::PHAorPHP; _pIdxReg = &_regs.A; break;
    case 0x68: _type = InstrType::PLAorPLP; _pIdxReg = &_regs.A; break;
    case 0x08: _type = InstrType::PHAorPHP; _pIdxReg = &_regs.P; break;
    case 0x28: _type = InstrType::PLAorPLP; _pIdxReg = &_regs.P; break;

        // No Operation
    case 0xea: _type = InstrType::Implied; _action = &NewCpu::nop; break;

    default:
        __debugbreak();
    }
}
