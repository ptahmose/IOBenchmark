#include "pch.h"
#include "RingBufferManager.h"
#include <algorithm>

using namespace std;

CRingBufferManager::CRingBufferManager(std::uint32_t buffersize, std::uint32_t buffercnt, std::function<bool(const void* ptr, std::uint32_t size)> writeFunct)
    : buffersize(buffersize), writeFunction(writeFunct), occupancy(buffercnt),
    curReadBuf(0), curWriteBuf(0), writeBufOffset(0), threadTerminate(false)
{
    this->buffers = this->CreateBuffers(buffersize, buffercnt);

    this->hSem = CreateSemaphore(
        NULL,           // default security attributes
        0,              // initial count
        1 + buffercnt,  // maximum count
        NULL);          // unnamed semaphore
    std::thread t(&CRingBufferManager::ThreadFunction, this);
    this->threadobj = move(t);
}

CRingBufferManager::~CRingBufferManager()
{
    this->FinishAndShutdown();
    CloseHandle(this->hSem);
}

std::vector<std::unique_ptr<void, void(*)(void*)>> CRingBufferManager::CreateBuffers(std::uint32_t buffersize, std::uint32_t buffercnt)
{
    std::vector<std::unique_ptr<void, void(*)(void*)>> buffers;
    buffers.reserve(buffercnt);

    for (std::uint32_t i = 0; i < buffercnt; ++i)
    {
        std::unique_ptr<void, void(*)(void*)> upBuf{ _aligned_malloc(buffersize,4096), CRingBufferManager::FreeMem };
        buffers.emplace_back(std::move(upBuf));
    }

    return buffers;
}

void CRingBufferManager::FinishAndShutdown()
{
    if (this->threadobj.joinable())
    {
        this->threadTerminate = true;
        ReleaseSemaphore(this->hSem, 1, NULL);
        this->threadobj.join();
    }
}

/*static*/void CRingBufferManager::FreeMem(void* p)
{
    _aligned_free(p);
}

bool CRingBufferManager::AddDataNonBlocking(const void* ptr, std::uint32_t size)
{
    const bool enoughSpace = this->CanAddNonBlocking(size);
    if (!enoughSpace)
    {
        return false;
    }

    this->AddSpaceChecked(ptr, size);
    return true;
}

void CRingBufferManager::AddSpaceChecked(const void* ptr, std::uint32_t size)
{
    uint32_t bytesCopied = 0;

    // first deal with what fits into the current write-buf
    uint32_t bytesFreeInCurrentWriteBuf = this->buffersize - this->writeBufOffset;
    uint32_t bytesToCopy = (std::min)(bytesFreeInCurrentWriteBuf, size);
    memcpy(
        this->GetBuf(this->curWriteBuf) + this->writeBufOffset,
        ptr,
        bytesToCopy);
    bytesCopied += bytesToCopy;
    if (bytesFreeInCurrentWriteBuf == bytesToCopy)
    {
        this->occupancy.SetOccupied(this->curWriteBuf);
        this->curWriteBuf = this->IncBufIdx(this->curWriteBuf);
        this->writeBufOffset = 0;
        this->NotifyNewBufferAvailable();
    }
    else
    {
        this->writeBufOffset += bytesToCopy;
    }

    if (bytesCopied >= size)
    {
        return;
    }

    // ok, now fill the next buffers
    for (;;)
    {
        bytesToCopy = (std::min)(size - bytesCopied, this->buffersize);
        memcpy(
            this->GetBuf(this->curWriteBuf),
            static_cast<const uint8_t*>(ptr) + bytesCopied,
            bytesToCopy);
        bytesCopied += bytesToCopy;
        if (bytesToCopy == this->buffersize)
        {
            // the whole buffer was filled
            this->occupancy.SetOccupied(this->curWriteBuf);
            this->curWriteBuf = this->IncBufIdx(this->curWriteBuf);
            this->NotifyNewBufferAvailable();
        }
        else
        {
            // the buffer was only partially filed
            this->writeBufOffset = bytesToCopy;
        }

        if (bytesCopied >= size)
        {
            break;
        }
    }
}

void CRingBufferManager::NotifyNewBufferAvailable()
{
    ReleaseSemaphore(this->hSem, 1, NULL);
}

bool CRingBufferManager::CanAddNonBlocking(std::uint32_t size)
{
    uint64_t bytesFree = 0;
    // for the current write-buf, we have to consider the "position"
    if (this->occupancy.IsFree(this->curWriteBuf))
    {
        bytesFree += (this->buffersize - this->writeBufOffset);
    }
    else
    {
        return false;
    }

    if (bytesFree >= size)
    {
        return true;
    }

    uint32_t idx = this->curWriteBuf;
    for (;;)
    {
        idx = this->IncBufIdx(idx);
        if (idx == this->writeBufOffset)
        {
            break;
        }

        if (this->occupancy.IsOccupied(idx))
        {
            break;
        }

        bytesFree += this->buffersize;
        if (bytesFree >= size)
        {
            break;
        }
    }

    if (bytesFree >= size)
    {
        return true;
    }

    return false;
}

/*static*/void CRingBufferManager::ThreadFunction(CRingBufferManager* p)
{
    p->OutputThread();
}

void CRingBufferManager::OutputThread()
{
    for (;;)
    {
        DWORD dwWaitResult = WaitForSingleObject(
            this->hSem,   // handle to semaphore
            INFINITE);    // time-out interval

        if (this->threadTerminate == true)
        {
            // now, write out everything we have
            for (;;)
            {
                if (!this->occupancy.IsOccupied(this->curReadBuf))
                {
                    return;
                }

                this->writeFunction(
                    this->GetBuf(this->curReadBuf),
                    this->buffersize);
                this->occupancy.SetFree(this->curReadBuf);
                this->curReadBuf = this->IncBufIdx(this->curReadBuf);
            }
        }

        this->writeFunction(
            this->GetBuf(this->curReadBuf),
            this->buffersize);

        this->occupancy.SetFree(this->curReadBuf);
        this->curReadBuf = this->IncBufIdx(this->curReadBuf);
    }
}