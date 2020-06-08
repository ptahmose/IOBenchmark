#include "pch.h"
#include <cassert>
#include "CFileApiImpl.h"
#include "UnbufferedFileWriter2.h"
#include <iostream>
#include <algorithm>

using namespace std;

/*static*/CUnbufferedFileWriter2::InitParameters CUnbufferedFileWriter2::defaultInitParameters = { 4 * 1024 * 1024, 1 * 1024 * 1024, 512 };

CUnbufferedFileWriter2::CUnbufferedFileWriter2(std::unique_ptr<IFileApi> fileApi, const InitParameters& initparams) :
    fileApi(move(fileApi)),
    unbufferedWriteOutSize(initparams.unbufferedWriteOutSize),
    prodConsQueue(100),
    runstate(RunState::NotStarted),
    blkWriteSize(initparams.unbufferedWriteOutBlockSize),
    fileSize(0),
    ringBufferSize(initparams.ringBufferSize),
    setWriteFinishedEvent(false)
{
    memset(&this->ringBufRun, 0, sizeof(this->ringBufRun));
    this->hWriteFinishedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CUnbufferedFileWriter2::CUnbufferedFileWriter2(std::unique_ptr<IFileApi> fileApi) :
    CUnbufferedFileWriter2(move(fileApi), CUnbufferedFileWriter2::defaultInitParameters)
{
}

CUnbufferedFileWriter2::CUnbufferedFileWriter2() :
    CUnbufferedFileWriter2(make_unique<CFileApiImpl>())
{
}

CUnbufferedFileWriter2::CUnbufferedFileWriter2(const InitParameters& initparams) :
    CUnbufferedFileWriter2(make_unique<CFileApiImpl>(), initparams)
{
}

/*virtual*/CUnbufferedFileWriter2::~CUnbufferedFileWriter2()
{
    CloseHandle(this->hWriteFinishedEvent);
}

/*virtual*/void CUnbufferedFileWriter2::InitializeFile(const wchar_t* filename)
{
    this->fileApi->Create(filename);

    //this->ringBufferManager = std::make_unique<CRingBufferManager>(
    //    1 * 1024 * 1024,
    //    8,
    //    [this](const void* ptr, std::uint32_t size)->bool {return this->WriteFunc(ptr, size); }/*
    //    std::bind(&CUnbufferedFileWriter::WriteFunc, this, std::placeholders::_1)*/
    //);
    this->ringBuffer = make_unique<CRingBuffer>(
        this->ringBufferSize,
        this->blkWriteSize);
    //2 * 1024 * 1024, 4096);

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

/*virtual*/void CUnbufferedFileWriter2::AppendSync(std::uint64_t offset, const void* ptr, std::uint32_t size)
{
    if (offset != this->fileSize)
    {
        assert(false);
        throw runtime_error("not implemented");
    }

    // well, first check if we can add it immediately without need to chop the request into parts
    if (this->TryAppendAtEnd(ptr, size) == true)
    {
        return;
    }

    uint32_t bytesWritten = 0;
    uint8_t* ptrToWrite = (uint8_t*)ptr;
    for (;;)
    {
        if (bytesWritten == size)
        {
            break;
        }

        uint32_t freeSize = this->ringBuffer->GetFreeSize();
        if (freeSize > 0)
        {
            uint32_t bytesToWrite = (min)(size - bytesWritten, freeSize);
            bool b = this->TryAppendAtEnd(ptrToWrite, bytesToWrite);
            assert(b);
            ptrToWrite += bytesToWrite;
            bytesWritten += bytesToWrite;
        }
        else
        {
            ResetEvent(this->hWriteFinishedEvent);
            this->setWriteFinishedEvent = true;

            // Note: we use the "double-check-pattern" here
            freeSize = this->ringBuffer->GetFreeSize();
            if (freeSize == 0)
            {
                // we will now be notified whenever some memory is freed in the ringbufffer
                WaitForSingleObject(this->hWriteFinishedEvent, INFINITE);
                this->setWriteFinishedEvent = false;
            }
        }
    }
}

void CUnbufferedFileWriter2::OverwriteSync(std::uint64_t offset, const void* ptr, std::uint32_t size)
{

}

void CUnbufferedFileWriter2::Close()
{
    Command cmd;
    if (this->runstate == RunState::RunStarted && this->ringBufRun.size > 0)
    {
        cmd.cmd = Command::Cmd::WriteBuffered;
        cmd.unbufferedOrBufferedWriteSize = this->ringBufRun.size;
        cmd.unbufferedOrBufferedWriteOffset = this->ringBufRun.baseFileOffset;
        this->prodConsQueue.enqueue(cmd);
    }

    cmd.cmd = Command::Cmd::Terminate;
    this->prodConsQueue.enqueue(cmd);

    this->threadobj.join();
    this->fileApi->Close();
}

bool CUnbufferedFileWriter2::TryAppendAtEnd(const void* ptr, std::uint32_t size)
{
    const uint32_t freeSize = this->ringBuffer->GetFreeSize();
    if (freeSize < size)
    {
        return false;
    }

    if (this->runstate == RunState::NotStarted)
    {
        this->ringBufRun.baseFileOffset = this->fileSize;
        this->ringBufRun.size = 0;
        this->runstate = RunState::RunStarted;
    }

    bool b = this->ringBuffer->Add(ptr, size);
    assert(b);

    this->ringBufRun.size += size;

    if (this->ringBufRun.size >= this->unbufferedWriteOutSize)
    {
        uint32_t blkAlignedSize = (this->ringBufRun.size / this->blkWriteSize) * this->blkWriteSize;
        Command cmd;
        cmd.cmd = Command::Cmd::WriteUnbuffered;
        cmd.unbufferedOrBufferedWriteSize = blkAlignedSize;
        cmd.unbufferedOrBufferedWriteOffset = this->ringBufRun.baseFileOffset;// this->ringBufRun.baseFileOffset;
        this->prodConsQueue.enqueue(cmd);

        if (blkAlignedSize < this->ringBufRun.size)
        {
            this->ringBufRun.baseFileOffset += blkAlignedSize;
            this->ringBufRun.size = this->ringBufRun.size - blkAlignedSize;
        }
        else
        {
            this->runstate = RunState::NotStarted;
        }
    }

    //this->fileSize += size;
    //if (this->ringBuffer->GetUsedSize() >= this->unbufferedWriteOutSize)
    //{
    //    Command cmd;
    //    cmd.cmd = Command::Cmd::WriteUnbuffered;
    //    cmd.unbufferedOrBufferedWriteSize = (this->ringBuffer->GetUsedSize() / this->unbufferedWriteOutSize) * this->unbufferedWriteOutSize;
    //    cmd.unbufferedOrBufferedWriteOffset = this->ringBufRun.baseFileOffset;
    //    this->prodConsQueue.enqueue(cmd);
    //}

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
            break;
        case Command::Cmd::WriteBuffered:
            this->WriteBuffered(cmd);
            break;
        case Command::Cmd::Terminate:
            return;
        }
    }
}

void CUnbufferedFileWriter2::WriteUnbuffered(const Command& cmd)
{
    //cout << "Write: " << cmd.unbufferedOrBufferedWriteOffset << "  size: " << cmd.unbufferedOrBufferedWriteSize << endl;
    CRingBuffer::ReadDataInfo readInfo;
    bool b = this->ringBuffer->Get(cmd.unbufferedOrBufferedWriteSize, readInfo);
    if (b == false)
    {
        DebugBreak();
    }

    assert(b);

    this->fileApi->WriteUnbuffered(
        cmd.unbufferedOrBufferedWriteOffset,
        readInfo.ptr1,
        readInfo.size1);

    if (readInfo.ptr2 != nullptr)
    {
        this->fileApi->WriteUnbuffered(
            cmd.unbufferedOrBufferedWriteOffset + readInfo.size1,
            readInfo.ptr2,
            readInfo.size2);
    }

    this->ringBuffer->AdvanceRead(cmd.unbufferedOrBufferedWriteSize);
    if (this->setWriteFinishedEvent)
    {
        SetEvent(this->hWriteFinishedEvent);
    }
}

void CUnbufferedFileWriter2::WriteBuffered(const Command& cmd)
{
    CRingBuffer::ReadDataInfo readInfo;
    bool b = this->ringBuffer->Get(cmd.unbufferedOrBufferedWriteSize, readInfo);
    assert(b);

    this->fileApi->WriteBuffered(
        cmd.unbufferedOrBufferedWriteOffset,
        readInfo.ptr1,
        readInfo.size1);
    if (readInfo.ptr2 != nullptr)
    {
        this->fileApi->WriteBuffered(
            cmd.unbufferedOrBufferedWriteOffset + readInfo.size1,
            readInfo.ptr2,
            readInfo.size2);
    }

    this->ringBuffer->AdvanceRead(cmd.unbufferedOrBufferedWriteSize);
    if (this->setWriteFinishedEvent)
    {
        SetEvent(this->hWriteFinishedEvent);
    }
}