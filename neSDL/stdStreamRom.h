#pragma once

#include "stdafx.h"
#include <fstream>

#include <experimental\filesystem>
namespace fs = std::experimental::filesystem;

// TODO: These std stream implementations will be used by save state as well, so move them somewhere more accessible
class StdReadStream : public IReadStream, public NesObject
{
private:
    StdReadStream(const char* path)
        : _stream(path, std::ios::binary)
    {
    }

public:
    virtual ~StdReadStream()
    {
        if (_stream.is_open())
        {
            _stream.close();
        }
    }

public:
    static bool Create(const char* path, IReadStream** stream)
    {
        NPtr<StdReadStream> newStream;
        newStream.Attach(new StdReadStream(path));
        if (newStream->_stream.is_open())
        {
            *stream = newStream.Detach();
            return true;
        }
        else
        {
            *stream = nullptr;
            return false;
        }
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    void ReadBytes(unsigned char* buf, long long count)
    {
        _stream.read((char*)buf, count);
    }

private:
    std::ifstream _stream;
};

class StdWriteStream : public IWriteStream, public NesObject
{
private:
    StdWriteStream(const char* path)
        : _stream(path, std::ios::binary | std::ios::trunc)
    {
    }

public:
    virtual ~StdWriteStream()
    {
        if (_stream.is_open())
        {
            _stream.close();
        }
    }

public:
    static bool Create(const char* path, IWriteStream** stream)
    {
        NPtr<StdWriteStream> newStream;
        newStream.Attach(new StdWriteStream(path));
        if (newStream->_stream.is_open())
        {
            *stream = newStream.Detach();
            return true;
        }
        else
        {
            *stream = nullptr;
            return false;
        }
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    void WriteBytes(unsigned char* buf, long long count)
    {
        _stream.write((char*)buf, count);
    }

private:
    std::ofstream _stream;
};

class StdStreamRomFile : public IRomFile, public NesObject
{
public:
    StdStreamRomFile(const char* romPath)
        : _romPath(romPath)
    {
        fs::path path(_romPath);
        _saveGamePath = path.replace_extension(".sav").generic_u8string().c_str();
        _saveStatePath = path.replace_extension(".ns").generic_u8string().c_str();
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    bool GetRomFileStream(IReadStream** stream)
    {
        return StdReadStream::Create(_romPath, stream);
    }

    bool GetSaveGameStream(IWriteStream** stream)
    {
        return StdWriteStream::Create(_saveGamePath.c_str(), stream);
    }

    bool GetLoadGameStream(IReadStream** stream)
    {
        return StdReadStream::Create(_saveGamePath.c_str(), stream);
    }

    bool GetSaveStateStream(IWriteStream** stream)
    {
        return StdWriteStream::Create(_saveStatePath.c_str(), stream);
    }

    bool GetLoadStateStream(IReadStream** stream)
    {
        return StdReadStream::Create(_saveStatePath.c_str(), stream);
    }

private:
    const char* _romPath;
    std::string _saveGamePath;
    std::string _saveStatePath;
};
