#include "stdafx.h"
#include "util.h"

void Util::WriteBytes(bool val, std::ofstream& ofs)
{
    u8 buf = val ? 0x01 : 0x00;
    ofs.write((char*)&buf, sizeof(buf));
}

void Util::WriteBytes(u8 val, std::ofstream& ofs)
{
    ofs.write((char*)&val, sizeof(val));
}

void Util::WriteBytes(u16 val, std::ofstream& ofs)
{
    ofs.write((char*)&val, sizeof(val));
}

void Util::WriteBytes(u32 val, std::ofstream& ofs)
{
    ofs.write((char*)&val, sizeof(val));
}

void Util::WriteBytes(i32 val, std::ofstream& ofs)
{
    ofs.write((char*)&val, sizeof(val));
}

void Util::WriteBytes(const std::vector<u8>& val, std::ofstream& ofs)
{
    //write the length of the vector, then the bytes
    size_t length = val.size();
    ofs.write((char*)&length, sizeof(length));
    ofs.write((char*)&val[0], length);
}

void Util::ReadBytes(bool& val, std::ifstream& ifs)
{
    u8 buf;
    ifs.read((char*)&buf, sizeof(buf));
    val = buf != 0 ? true : false;
}

void Util::ReadBytes(u8& val, std::ifstream& ifs)
{
    ifs.read((char*)&val, sizeof(val));
}

void Util::ReadBytes(u16& val, std::ifstream& ifs)
{
    ifs.read((char*)&val, sizeof(val));
}

void Util::ReadBytes(u32& val, std::ifstream& ifs)
{
    ifs.read((char*)&val, sizeof(val));
}

void Util::ReadBytes(i32& val, std::ifstream& ifs)
{
    ifs.read((char*)&val, sizeof(val));
}

void Util::ReadBytes(std::vector<u8>& val, std::ifstream& ifs)
{
    size_t length = 0;
    ifs.read((char*)&length, sizeof(length));
    val.resize(length);
    ifs.read((char*)&val[0], length);
}