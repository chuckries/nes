#include "stdafx.h"
#include "util.h"

void Util::WriteBytes(bool val, IWriteStream* ostream)
{
    u8 buf = val ? 0x01 : 0x00;
    ostream->WriteBytes((u8*)&buf, sizeof(buf));
}

void Util::WriteBytes(u8 val, IWriteStream* ostream)
{
    ostream->WriteBytes((u8*)&val, sizeof(val));
}

void Util::WriteBytes(u16 val, IWriteStream* ostream)
{
    ostream->WriteBytes((u8*)&val, sizeof(val));
}

void Util::WriteBytes(u32 val, IWriteStream* ostream)
{
    ostream->WriteBytes((u8*)&val, sizeof(val));
}

void Util::WriteBytes(i32 val, IWriteStream* ostream)
{
    ostream->WriteBytes((u8*)&val, sizeof(val));
}

void Util::ReadBytes(bool& val, IReadStream* istream)
{
    u8 buf;
    istream->ReadBytes((u8*)&buf, sizeof(buf));
    val = buf != 0 ? true : false;
}

void Util::ReadBytes(u8& val, IReadStream* istream)
{
    istream->ReadBytes((u8*)&val, sizeof(val));
}

void Util::ReadBytes(u16& val, IReadStream* istream)
{
    istream->ReadBytes((u8*)&val, sizeof(val));
}

void Util::ReadBytes(u32& val, IReadStream* istream)
{
    istream->ReadBytes((u8*)&val, sizeof(val));
}

void Util::ReadBytes(i32& val, IReadStream* istream)
{
    istream->ReadBytes((u8*)&val, sizeof(val));
}