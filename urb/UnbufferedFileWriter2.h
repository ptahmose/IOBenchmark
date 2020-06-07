#pragma once

#include <memory>
#include <thread>



#include "IFileApi.h"
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

        std::uint32_t unbufferedOrBufferedWriteSize;
        std::uint64_t unbufferedOrBufferedWriteOffset;
    };

    enum class RunState
    {
        NotStarted,
        RunStarted
    };

    RunState runstate;

    //HANDLE hUnbuffered, hBuffered;

    std::thread threadobj;
    moodycamel::BlockingReaderWriterQueue<Command> prodConsQueue;
    //std::unique_ptr<CRingBufferManager> ringBufferManager;
    std::unique_ptr<CRingBuffer> ringBuffer;
    std::uint32_t unbufferedWriteOutSize;
    std::uint32_t blkWriteSize;

    std::unique_ptr<IFileApi> fileApi;
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
    CUnbufferedFileWriter2(std::unique_ptr<IFileApi> fileApi);
    virtual void InitializeFile(const wchar_t* filename);
    virtual bool TryAppendNoWait(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void OverwriteSync(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void Close();
private:
    bool WriteFunc(const void* ptr, std::uint32_t size);
    static void ThreadFunction(CUnbufferedFileWriter2* p);
    void WriteThreadFunction();
    void WriteUnbuffered(const Command& cmd);
    void WriteBuffered(const Command& cmd);

    bool TryAppendAtEnd(const void* ptr, std::uint32_t size);
};
