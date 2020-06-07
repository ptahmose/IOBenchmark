#include "pch.h"
#include "UnbufferedFileWriter2.h"

#include <cassert>

using namespace std;

CUnbufferedFileWriter2::CUnbufferedFileWriter2()
    : hUnbuffered(INVALID_HANDLE_VALUE),
    hBuffered(INVALID_HANDLE_VALUE),
    unbufferedWriteOutSize(1024 * 1024),
    prodConsQueue(100)
{
}

/*virtual*/void CUnbufferedFileWriter2::InitializeFile(const wchar_t* filename)
{
    HANDLE hUnbuffered = ::CreateFileW(
        filename,
        GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
        NULL);
    HANDLE hBuffered = ReOpenFile(
        hUnbuffered,
        GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        0);

    this->hUnbuffered = hUnbuffered;
    this->hBuffered = hBuffered;

    //this->ringBufferManager = std::make_unique<CRingBufferManager>(
    //    1 * 1024 * 1024,
    //    8,
    //    [this](const void* ptr, std::uint32_t size)->bool {return this->WriteFunc(ptr, size); }/*
    //    std::bind(&CUnbufferedFileWriter::WriteFunc, this, std::placeholders::_1)*/
    //);
    this->ringBuffer = make_unique<CRingBuffer>(2 * 1024 * 1024, 4096);

    std::thread t(&CUnbufferedFileWriter2::ThreadFunction, this);
    this->threadobj = move(t);
}

/*virtual*/bool CUnbufferedFileWriter2::TryAppendNoWait(std::uint64_t offset, const void* ptr, std::uint32_t size)
{
    // first check if we append (directly at the end of the file)
    if (offset == this->fileSize)
    {
        return this->TryAppendAtEnd(ptr, size);
    }

    /*Command cmd;
    cmd.cmd = Command::Cmd::WriteUnbuffered;
    this->prodConsQueue.enqueue(cmd);*/

    return false;
}

void CUnbufferedFileWriter2::OverwriteSync(std::uint64_t offset, const void* ptr, std::uint32_t size)
{

}

void CUnbufferedFileWriter2::Close()
{
    this->threadobj.join();
}

bool CUnbufferedFileWriter2::TryAppendAtEnd(const void* ptr, std::uint32_t size)
{
    uint32_t freeSize = this->ringBuffer->GetFreeSize();
    if (freeSize < size)
    {
        return false;
    }

    bool b = this->ringBuffer->Add(ptr, size);

    if (this->ringBuffer->GetUsedSize() >= this->unbufferedWriteOutSize)
    {
        Command cmd;
        cmd.cmd = Command::Cmd::WriteUnbuffered;
        cmd.unbufferedWriteSize = (this->ringBuffer->GetUsedSize() / this->unbufferedWriteOutSize) * this->unbufferedWriteOutSize;
        cmd.unbufferedWriteOffset = this->ringBufRun.baseFileOffset;
        this->prodConsQueue.enqueue(cmd);
    }

    this->fileSize += size;
    return true;
}

/*static*/void CUnbufferedFileWriter2::ThreadFunction(CUnbufferedFileWriter2* p)
{
    p->WriteThreadFunction();
}

void CUnbufferedFileWriter2::WriteThreadFunction()
{
    Command cmd;
    for (;;)
    {
        this->prodConsQueue.wait_dequeue(cmd);
        switch (cmd.cmd)
        {
        case Command::Cmd::WriteUnbuffered:
            this->WriteUnbuffered(cmd);
        }
    }
}

void CUnbufferedFileWriter2::WriteUnbuffered(const Command& cmd)
{
    CRingBuffer::ReadDataInfo readInfo;
    this->ringBuffer->Get(cmd.unbufferedWriteSize, readInfo);

    DWORD bytesWritten;
    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.Offset = (DWORD)cmd.unbufferedWriteOffset;
    overlapped.OffsetHigh = (DWORD)(cmd.unbufferedWriteOffset >> 32);
    BOOL B = WriteFile(
        this->hBuffered,
        readInfo.ptr1,
        readInfo.size1,
        &bytesWritten,
        &overlapped);
    assert(B == TRUE);

    if (readInfo.ptr2 != nullptr)
    {
        overlapped.Offset = (DWORD)(cmd.unbufferedWriteOffset+readInfo.size1);
        overlapped.OffsetHigh = (DWORD)((cmd.unbufferedWriteOffset + readInfo.size1) >> 32);
        BOOL B = WriteFile(
            this->hBuffered,
            readInfo.ptr2,
            readInfo.size2,
            &bytesWritten,
            &overlapped);
        assert(B == TRUE);
    }
}