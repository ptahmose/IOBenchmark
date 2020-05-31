#include "WriterRingbuffer.h"
#include "Blk.h"
#include "utf8convert.h"
#include <cstdint>
#include <sstream>

using namespace std;

WriterRingbuffer::WriterRingbuffer() : hFile(INVALID_HANDLE_VALUE)
{
}

/*virtual*/void WriterRingbuffer::Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> /*writerSpecificOptions*/)
{
    const auto filenameW = Utf8ToUtf16(options.filename);
    HANDLE h = CreateFileW(
        filenameW.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
        NULL);

    if (h == INVALID_HANDLE_VALUE)
    {
        stringstream ss;
        ss << "Error when calling \"CreateFile\" with filename \"" << options.filename << ".";
        auto excp = WriterException(WriterException::ErrorType::APIError, ss.str());
        excp.SetLastError(GetLastError());
        throw excp;
    }

    this->options = options;
    this->hFile = h;

    auto rb = std::make_unique<RingBufferWriter>(options.blkSize, 16ul, std::bind(&WriterRingbuffer::Write, this, placeholders::_1, placeholders::_2));
    this->ringBuff = std::move(rb);
}

/*virtual*/void WriterRingbuffer::DoIt()
{
    for (uint64_t totalBytesWritten = 0; totalBytesWritten < this->options.fileSize;)
    {
        this->ringBuff->AddData(nullptr, this->options.blkSize);
        totalBytesWritten += this->options.blkSize;
    }
}

/*virtual*/WriterRingbuffer::~WriterRingbuffer()
{
    if (this->hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(this->hFile);
    }
}

bool WriterRingbuffer::Write(const void* p, std::uint32_t s)
{
    BOOL B;
    //s -= 1;
    /*LARGE_INTEGER pos{ 1,0 };
    BOOL B = SetFilePointerEx(this->hFile, pos, NULL, FILE_CURRENT);
    pos.QuadPart = 4095;
    B = SetFilePointerEx(this->hFile, pos, NULL, FILE_CURRENT);
    pos.QuadPart = 4096;
    B = SetFilePointerEx(this->hFile, pos, NULL, FILE_CURRENT);*/
    DWORD bytesWritten;
    B = WriteFile(
        this->hFile,
        p,
        s,
        &bytesWritten,
        nullptr);
    if (B == FALSE)
    {
        DWORD lastErr = GetLastError();
    }
    return true;
}