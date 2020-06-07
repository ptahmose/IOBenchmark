#pragma once

#include <memory>
#include <thread>


#include "IUnbufferedFileWriter.h"
#include "readerwriterqueue.h"
#include "ringbuffer.h"
//#include "RingBufferManager.h"

class CUnbufferedFileWriter2 : public IUnbufferedFileWriter2
{
private:
    struct Command
    {
        enum class  Cmd
        {
            WriteUnbuffered,
            WriteBuffered,
            Terminate
        };

        Cmd     cmd;

        std::uint32_t unbufferedWriteSize;
        std::uint64_t unbufferedWriteOffset;
    };

    HANDLE hUnbuffered, hBuffered;

    std::thread threadobj;
    moodycamel::BlockingReaderWriterQueue<Command> prodConsQueue;
    //std::unique_ptr<CRingBufferManager> ringBufferManager;
    std::unique_ptr<CRingBuffer> ringBuffer;
    std::uint32_t unbufferedWriteOutSize;
private:
    struct RingBufferRun
    {
        std::uint64_t baseFileOffset;
        std::uint32_t size;
    };

    RingBufferRun ringBufRun;
    std::uint64_t fileSize;
public:
    CUnbufferedFileWriter2();
    virtual void InitializeFile(const wchar_t* filename);
    virtual bool TryAppendNoWait(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void OverwriteSync(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void Close();
private:
    bool WriteFunc(const void* ptr, std::uint32_t size);
    static void ThreadFunction(CUnbufferedFileWriter2* p);
    void WriteThreadFunction();
    void WriteUnbuffered(const Command& cmd);

    bool TryAppendAtEnd(const void* ptr, std::uint32_t size);
};
