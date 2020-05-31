#pragma once
#include <functional>
#include <cinttypes>
#include <memory>
#include <thread>
#include <vector>
#include <windows.h>
#include "occupancycontroller.h"

class CRingBufferManager
{
private:
    std::function<bool(const void* ptr, std::uint32_t size)> writeFunction;

    COccupancyController occupancy;

    std::uint32_t buffersize;
    std::vector<std::unique_ptr<void, void(*)(void*)>> buffers;

    std::uint32_t curReadBuf;
    std::uint32_t curWriteBuf;
    std::uint32_t writeBufOffset;

    std::thread threadobj;
    volatile bool threadTerminate;
    HANDLE hSem;
public:
    explicit CRingBufferManager(std::uint32_t buffersize, std::uint32_t buffercnt, std::function<bool(const void* ptr, std::uint32_t size)> writeFunct);
    ~CRingBufferManager();

    bool AddDataNonBlocking(const void* ptr, std::uint32_t size);
    bool CanAddNonBlocking(std::uint32_t size);

    void FinishAndShutdown();
private:
    void AddSpaceChecked(const void* ptr, std::uint32_t size);

    std::vector<std::unique_ptr<void, void(*)(void*)>> CreateBuffers(std::uint32_t buffersize, std::uint32_t buffercnt);
    static void FreeMem(void* p);

    static void ThreadFunction(CRingBufferManager* p);
    void OutputThread();
    void NotifyNewBufferAvailable();

    std::uint32_t IncBufIdx(std::uint32_t idx) const
    {
        return (idx + 1) % this->GetCountOfBuffers();
    }

    std::uint32_t GetCountOfBuffers() const
    {
        return static_cast<uint32_t>(this->buffers.size());
    }

    std::uint8_t* GetBuf(std::uint32_t idx)
    {
        return static_cast<std::uint8_t*>(this->buffers[idx].get());
    }
};
