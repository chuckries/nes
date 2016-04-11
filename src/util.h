#pragma once

#include "interfaces.h"

class Util
{
public:
    static void WriteBytes(bool val, IWriteStream* ostream);
    static void WriteBytes(u8 val, IWriteStream* ostream);
    static void WriteBytes(u16 val, IWriteStream* ostream);
    static void WriteBytes(u32 val, IWriteStream* ostream);
    static void WriteBytes(i32 val, IWriteStream* ostream);

    static void ReadBytes(bool& val, IReadStream* istream);
    static void ReadBytes(u8& val, IReadStream* istream);
    static void ReadBytes(u16& val, IReadStream* istream);
    static void ReadBytes(u32& val, IReadStream* istream);
    static void ReadBytes(i32& val, IReadStream* istream);
};