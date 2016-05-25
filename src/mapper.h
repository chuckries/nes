#pragma once

#include "interfaces.h"

class NRom : public IMapper, public NesObject
{
public:
    NRom(Rom* rom);
    virtual ~NRom();
    
public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    virtual u8 prg_loadb(u16 addr);
    virtual void prg_storeb(u16 addr, u8 val);
    virtual u8 chr_loadb(u16 addr);
    virtual void chr_storeb(u16 addr, u8 val);

public:
    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

private:
    u8* _chrBuf;
    std::vector<u8> _chrRam; // If no ChrRom is provided we will give ChrRam
};

class SxRom : public IMapper, public NesObject
{
public:
    SxRom(Rom* rom);
    virtual ~SxRom();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    virtual void Reset(bool hard);

public:
    u8 prg_loadb(u16 addr);
    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);
    void chr_storeb(u16 addr, u8 val);

public:
    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

private:
    void SetSegmentAddresses();
    u32 GetChrAddress(u16 addr);

private:
    bool _swap256;
    bool _chrMode;
    bool _prgSize;
    bool _slotSelect;
    u8 _chrReg[2];
    u8 _prgReg;
    u8 _accumulator;

    u32 _prgSegmentAddr[2];
    u32 _chrSegmentAddr[2];

    u8 _writeCount;
    u8* _chrBuf;
    std::vector<u8> _chrRam;
};

class UxRom : public NRom
{
public:
    UxRom(Rom* rom);

    void Reset(bool hard);

    void prg_storeb(u16 addr, u8 val);
    u8 prg_loadb(u16 addr);

    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);
private:
    int _lastBankOffset;
    u8 _prgBank;
};

class CNRom : public NRom
{
public:
    CNRom(Rom* rom);

    void Reset(bool hard);

    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);

    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);
private:
    u8 _chrBank;
};

// MMC3
class TxRom : public IMapper, public NesObject
{
public:
    TxRom(Rom* rom);

    void Reset(bool hard);

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    u8 prg_loadb(u16 addr);
    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);
    void chr_storeb(u16 addr, u8 val);

    bool Scanline();

private:
    void SetSegmentAddresses();
    u32 GetChrAddress(u16 addr);

private:
    bool _chrMode;
    bool _prgMode;
    u8 _addr8001; // "address" to use when writing $80001
    u8 _chrReg[6];
    u8 _prgReg[2];

    u32 _prgSegmentAddr[4];
    u32 _chrSegmentAddr[8];

    u32 _lastPrgBankIndex;
    u32 _secondLastPrgBankIndex;
    u8 _prgRegMask;
    u8 _chrRegMask;

    u16 _irqCounter;
    u16 _irqReload;
    bool _irqEnable;
    bool _irqPending;

    u8* _chrBuf;
    u8 _chrRam[0x2000];
};

// AxRom, #7
class AxRom : public NRom 
{
public:
    AxRom(Rom* rom);

    void Reset(bool hard);

    u8 prg_loadb(u16 addr);
    void prg_storeb(u16 addr, u8 val);

private:
    u8 _prgReg;
};