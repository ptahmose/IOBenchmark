#include "ringbufferwriter.h"
#include <intrin.h>

RingBufferWriter::RingBufferWriter(std::uint32_t buffersize, std::uint32_t buffercnt, std::function<bool(const void* ptr, std::uint32_t size)> writeFunct)
    : buffersize(buffersize), writeFunction(writeFunct), occupancy(0)
{
    this->buffers = this->CreateBuffers(buffersize, buffercnt);
}


std::vector<std::unique_ptr<void, void(*)(void*)>> RingBufferWriter::CreateBuffers(std::uint32_t buffersize, std::uint32_t buffercnt)
{
    std::vector<std::unique_ptr<void, void(*)(void*)>> buffers;
    buffers.reserve(buffercnt);

    for (std::uint32_t i = 0; i < buffercnt; ++i)
    {
        std::unique_ptr<void, void(*)(void*)> upBuf{ _aligned_malloc(buffersize,4096), RingBufferWriter::FreeMem };
        buffers.emplace_back(std::move(upBuf));
    }

    return buffers;
}

void RingBufferWriter::AddData(const void* ptr, std::uint32_t size)
{
    // is the current buffer "free"
    if (this->IsBufferFree(this->curWritePos))
    {
        // how many bytes are available?
    }

    this->writeFunction(this->buffers[0].get(), this->buffersize);
}

/*static*/void RingBufferWriter::FreeMem(void* p)
{
    _aligned_free(p);
}

bool RingBufferWriter::IsBufferFree(std::uint32_t idx)
{
    return _bittest64((const __int64*)&this->occupancy, idx) == 0;
}

// _interlockedbittestandset64(
