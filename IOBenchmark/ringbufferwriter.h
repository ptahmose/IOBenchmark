#pragma once
#include <memory>
#include <cinttypes>
#include <functional>
#include <vector>

class RingBufferWriter
{
private:
    std::function<bool(const void* ptr, std::uint32_t size)> writeFunction;

    std::uint64_t occupancy;

    std::uint32_t buffersize;
    std::vector<std::unique_ptr<void, void(*)(void*)>> buffers;

    std::uint32_t curReadPos;
    std::uint32_t curWritePos;
    std::uint64_t position;
public:
    explicit RingBufferWriter(std::uint32_t buffersize, std::uint32_t buffercnt, std::function<bool(const void* ptr, std::uint32_t size)> writeFunct);

    void AddData(const void* ptr, std::uint32_t size);

private:
    std::vector<std::unique_ptr<void, void(*)(void*)>> CreateBuffers(std::uint32_t buffersize, std::uint32_t buffercnt);
    static void FreeMem(void* p);

    bool IsBufferFree(std::uint32_t idx);
};
