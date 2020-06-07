#include "pch.h"
#include "UnbufferedFileWriter.h"

#include <cassert>

CUnbufferedFileWriter::CUnbufferedFileWriter()
    : hUnbuffered(INVALID_HANDLE_VALUE), hBuffered(INVALID_HANDLE_VALUE)
{
}

/*virtual*/void CUnbufferedFileWriter::InitializeFile(const wchar_t* filename)
{
    HANDLE hUnbuffered = ::CreateFileW(
        filename,
        GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        CREATE_NEW,
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
        NULL);
    HANDLE hBuffered = ReOpenFile(
        hUnbuffered,
        GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        0);

    this->hUnbuffered = hUnbuffered;
    this->hBuffered = hBuffered;

    this->ringBufferManager = std::make_unique<CRingBufferManager>(
        1 * 1024 * 1024,
        8,
        [this](const void* ptr, std::uint32_t size)->bool {return this->WriteFunc(ptr, size); }/*
        std::bind(&CUnbufferedFileWriter::WriteFunc, this, std::placeholders::_1)*/
    );
}

/*virtual*/bool CUnbufferedFileWriter::TryAppendNoWait(const void* ptr, std::uint32_t size)
{
    return this->ringBufferManager->AddDataNonBlocking(ptr, size);
}

/*virtual*/void CUnbufferedFileWriter::Rewrite(std::uint64_t offset, const void* ptr, std::uint32_t size)
{
    DWORD bytesWritten;
    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.Offset = (DWORD)offset;
    overlapped.OffsetHigh = (DWORD)(offset >> 32);
    BOOL B = WriteFile(
        this->hBuffered,
        ptr,
        size,
        &bytesWritten,
        &overlapped);
    assert(B == TRUE);
}

/*virtual*/void CUnbufferedFileWriter::Close()
{
    this->ringBufferManager->FinishAndShutdown();
    CloseHandle(this->hUnbuffered);
    CloseHandle(this->hBuffered);
}

bool CUnbufferedFileWriter::WriteFunc(const void* ptr, std::uint32_t size)
{
    DWORD bytesWritten;
    BOOL B = WriteFile(
        this->hUnbuffered,
        ptr,
        size,
        &bytesWritten,
        nullptr);
    if (B == FALSE)
    {
        DWORD lastErr = GetLastError();
    }
    assert(B == TRUE);
    return true;
}