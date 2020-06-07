#pragma once

#include <memory>
#include <thread>
#include "IFileApi.h"
#include "IUnbufferedFileWriter.h"
#include "readerwriterqueue.h"
#include "ringbuffer.h"

class CUnbufferedFileWriter2 : public IUnbufferedFileWriter2
{
private:
    struct Command
    {
        enum class Cmd
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
    /// The parameters used to initialize an instance - the memory sizes and alignment requirements are
    /// specified here.
    struct InitParameters
    {
        /// Size of the ring buffer in bytes - this must be a multiple of "unbufferedWriteOutBlockSize".
        std::uint32_t ringBufferSize;

        /// Size of the chunks we write out to disk - must be a multiple of "unbufferedWriteOutBlockSize".
        std::uint32_t unbufferedWriteOutSize;

        /// The block written to disk must be a multiple of this.
        std::uint32_t unbufferedWriteOutBlockSize;
    };

    static InitParameters defaultInitParameters;
public:
    CUnbufferedFileWriter2();
    CUnbufferedFileWriter2(const InitParameters& initparams);
    CUnbufferedFileWriter2(std::unique_ptr<IFileApi> fileApi);
    CUnbufferedFileWriter2(std::unique_ptr<IFileApi> fileApi, const InitParameters& initparams);

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
