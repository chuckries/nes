#pragma once

class RandomAccessReadStream: public IReadStream, public NesObject
{
protected:
    RandomAccessReadStream(DataReader^ dataReader)
        : _dataReader(dataReader)
    {
    }

public:
    virtual ~RandomAccessReadStream()
    {
    }

public:
    static IAsyncOperation<bool>^ Create(IRandomAccessStream^ stream, RandomAccessReadStream** randomAccessReadStream)
    {
        return create_async([=]() {
            DataReader^ dataReader = ref new DataReader(stream);
            return create_task(dataReader->LoadAsync(stream->Size)).then([=](unsigned int size) {
                *randomAccessReadStream = new RandomAccessReadStream(dataReader);
                return true;
            });
        });
    }

    // IReadStream
public:
    void ReadBytes(unsigned char* buf, long long count)
    {
        _dataReader->ReadBytes(ArrayReference<unsigned char>(buf, count));
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

private:
    DataReader^ _dataReader;
};

class RandomAccessWriteStream: public IWriteStream, public NesObject
{
protected:
    RandomAccessWriteStream(DataWriter^ dataWriter)
        : _dataWriter(dataWriter)
    {
    }

public:
    virtual ~RandomAccessWriteStream()
    {
    }

public:
    static IAsyncOperation<bool>^ Create(IRandomAccessStream^ stream, RandomAccessWriteStream** randomAccessWriteStream)
    {
        return create_async([=]() {
            DataWriter^ writer = ref new DataWriter(stream);
            *randomAccessWriteStream = new RandomAccessWriteStream(writer);
            return true;
        });
    }

public:
    void WriteBytes(unsigned char* buf, long long count)
    {
        _dataWriter->WriteBytes(ArrayReference<unsigned char>(buf, count));
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

private:
    DataWriter^ _dataWriter;
};

class RandomAccessStreamRomFile: public IRomFile, public NesObject
{
public:
    RandomAccessStreamRomFile(IRandomAccessStream^ romStream)
        : _romStream(romStream)
    {
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    // IRomFile
public:
    bool GetRomFileStream(IReadStream** stream) {
        NPtr<RandomAccessReadStream> randomAccessReadStream;
        if (create_task(::RandomAccessReadStream::Create(_romStream, &randomAccessReadStream)).get())
        {
            *stream = static_cast<IReadStream*>(randomAccessReadStream.Detach());
            return true;
        }
        return false;
    }

    bool GetSaveGameStream(IWriteStream** stream)
    {
        try
        {
            IRandomAccessStream^ randomAccessStream = create_task(ApplicationData::Current->RoamingFolder->CreateFileAsync(L"save.sav", CreationCollisionOption::ReplaceExisting)).then([](StorageFile^ file)
            {
                return file->OpenAsync(FileAccessMode::ReadWrite);
            }).get();
            NPtr<RandomAccessWriteStream> randomAccessWriteStream;
            if (create_task(::RandomAccessWriteStream::Create(randomAccessStream, &randomAccessWriteStream)).get())
            {
                *stream = static_cast<IWriteStream*>(randomAccessWriteStream.Detach());
                return true;;
            }
            return false;
        }
        catch (Exception^ e)
        {
            return false;
        }
    }

    bool GetLoadGameStream(IReadStream** stream)
    {
        try {
            IRandomAccessStream^ randomAccessStream = create_task(ApplicationData::Current->RoamingFolder->GetFileAsync(L"save.sav")).then([](StorageFile^ file)
            {
                return file->OpenReadAsync();
            }).get();
            NPtr<RandomAccessReadStream> randomAccessReadStream;
            if (create_task(::RandomAccessReadStream::Create(randomAccessStream, &randomAccessReadStream)).get())
            {
                *stream = static_cast<IReadStream*>(randomAccessReadStream.Detach());
                return true;
            }
            return false;
        }
        catch (Exception^ e)
        {
            return false;
        }
    }

private:
    IRandomAccessStream^ _romStream;
};

namespace NesRuntimeComponent
{
    public ref class RomFile sealed
    {
    public:
        RomFile(String^ name, IRandomAccessStream^ romStream)
            : _name(name)
        {
            _romFile.Attach(new ::RandomAccessStreamRomFile(romStream));
        }

    internal:
        property ::RandomAccessStreamRomFile* Native
        {
            ::RandomAccessStreamRomFile* get()
            {
                return _romFile;
            }
        }
    private:
        Platform::String^ _name;
        NPtr<::RandomAccessStreamRomFile> _romFile;
    };
}