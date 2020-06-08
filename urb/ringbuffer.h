#pragma once

#include <cinttypes>

class CRingBuffer
{
private:
    std::uint8_t* ptrBuffer;
    std::uint32_t size;

    std::uint32_t readPtr;
    std::uint32_t writerPtr;
    bool isEmpty;
public:
    struct ReadDataInfo
    {
        const void* ptr1;
        std::uint32_t size1;
        const void* ptr2;
        std::uint32_t size2;
    };
public:
    CRingBuffer(std::uint32_t size, std::uint32_t alignment);
    ~CRingBuffer();

    std::uint32_t GetFreeSize();
    std::uint32_t GetUsedSize();
    bool Add(const void* ptr, std::uint32_t size);
    bool Get(std::uint32_t size, ReadDataInfo& info);
    bool AdvanceRead(std::uint32_t size);
private:
    void AllocateMemory(std::uint32_t size, std::uint32_t alignment);
};
