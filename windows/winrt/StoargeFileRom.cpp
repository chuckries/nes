#include "pch.h"
#include "StorageFileRom.h"

using namespace Platform;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

StorageFileReadStream::StorageFileReadStream(DataReader^ dataReader)
    : _dataReader(dataReader)
{
}

bool StorageFileReadStream::Create(IRandomAccessStream^ stream, IReadStream** outStream)
{
    *outStream = nullptr;
    DataReader^ reader = ref new DataReader(stream);
    return create_task(reader->LoadAsync(stream->Size)).then([=](unsigned int count)
    {
        *outStream = new StorageFileReadStream(reader);
        return true;
    }).get();
}

int StorageFileReadStream::ReadBytes(unsigned char* buf, int count)
{
    _dataReader->ReadBytes(ArrayReference<unsigned char>(buf, count));
    return count;
}

StorageFileRom::StorageFileRom(IRandomAccessStream^ stream)
    : _stream(stream)
{
}

bool StorageFileRom::GetRomFileStream(IReadStream** stream)
{
    return StorageFileReadStream::Create(_stream, stream);
}

bool StorageFileRom::GetSaveGameStream(IWriteStream** stream)
{
    return false;
}

bool StorageFileRom::GetLoadGameStream(IReadStream** stream)
{
    return false;
}