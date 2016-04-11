#pragma once

class RandomAccessReadStream: public IReadStream, public NesObject
{
protected:
    RandomAccessReadStream(DataReader^ dataReader)
        : _dataReader(dataReader)
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

class RomFile: public IRomFile, public NesObject
{
public:
    RomFile(IRandomAccessStream^ romStream)
        : _romStream(romStream)
    {
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    // IRomFile
public:
    bool GetRomFileStream(IReadStream** stream) {
        NPtr<RandomAccessReadStream> randomAccessReadStream;
        bool succeeded = create_task(::RandomAccessReadStream::Create(_romStream, &randomAccessReadStream)).get();
        *stream = static_cast<IReadStream*>(randomAccessReadStream.Detach());
        return succeeded;
    }

    bool GetSaveGameStream(IWriteStream** stream) { return false; }
    bool GetLoadGameStream(IReadStream** stream) { return false; }

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
            _romFile.Attach(new ::RomFile(romStream));
        }

    internal:
        property ::RomFile* Native
        {
            ::RomFile* get()
            {
                return _romFile;
            }
        }
    private:
        Platform::String^ _name;
        NPtr<::RomFile> _romFile;
    };
}