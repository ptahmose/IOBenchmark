#include "pch.h"
#include "ringbuffer.h"
#include "exceptions.h"
#include <algorithm>
#include <sstream>

using namespace std;

CRingBuffer::CRingBuffer(std::uint32_t size, std::uint32_t alignment)
    : ptrBuffer(nullptr), readPtr(0), writerPtr(0)
{
    this->AllocateMemory(size, alignment);
    this->size = size;
}

CRingBuffer::~CRingBuffer()
{
    if (this->ptrBuffer != NULL)
    {
        VirtualFree(this->ptrBuffer, 0, MEM_RELEASE);
    }
}

void CRingBuffer::AllocateMemory(std::uint32_t size, std::uint32_t alignment)
{
    // "size" must be a multiple of "alignment"
    if ((size % alignment) != 0)
    {
        stringstream ss;
        ss << "\"size\" (=" << size << ") must be a multiple of \"alignment\" (=" << alignment << ")";
        throw UrbInvalidArgumentException(ss.str());
    }

    LPVOID pv = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pv == NULL)
    {
        stringstream ss;
        ss << "Error allocating " << size << " bytes of memory.";
        throw UrbMemoryAllocationException(ss.str());
    }

    // check if the alignment is sufficient
    if ((((std::uintptr_t)pv) % alignment) != 0)
    {
        VirtualFree(pv, 0, MEM_RELEASE);
        // now we would allocate a larger buffer, and then adjust the pointer
        // but... for the time being, we bail out here
        stringstream ss;
        ss << "The memory does not comply with the alignment requirement.";
        throw UrbMemoryAllocationException(ss.str());
    }

    this->ptrBuffer = static_cast<uint8_t*>(pv);
}

std::uint32_t CRingBuffer::GetFreeSize()
{
    uint32_t bytesFree;
    if (this->writerPtr >= this->readPtr)
    {
        bytesFree = this->size - (this->writerPtr - this->readPtr);
    }
    else
    {
        bytesFree = this->readPtr - this->writerPtr;
    }

    return bytesFree;
}

std::uint32_t CRingBuffer::GetUsedSize()
{
    return this->size - this->GetFreeSize();
}

bool CRingBuffer::Add(const void* ptr, std::uint32_t size)
{
    const uint32_t bytesFree = this->GetFreeSize();
    if (size > bytesFree)
    {
        return false;
    }

    // the size we have "right to the write-pointer"
    uint32_t copiedBytes = 0;
    const uint32_t forwardSize = this->size - this->writerPtr;
    if (forwardSize > 0)
    {
        copiedBytes = (std::min)(forwardSize, size);
        memcpy(this->ptrBuffer + this->writerPtr, ptr, copiedBytes);
    }

    if (copiedBytes < size)
    {
        memcpy(this->ptrBuffer, static_cast<const uint8_t*>(ptr) + copiedBytes, size - copiedBytes);
    }

    this->writerPtr = (this->writerPtr + size) % this->size;
    return true;
}

bool CRingBuffer::Get(std::uint32_t size, ReadDataInfo& info)
{
    uint32_t usedSize = this->GetUsedSize();
    if (usedSize < size)
    {
        return false;
    }

    info.ptr1 = this->ptrBuffer + this->readPtr;
    info.size1 = (min)(size, this->size - this->readPtr);

    if (info.size1 < size)
    {
        info.ptr2 = this->ptrBuffer;
        info.size2 = size - info.size1;
    }
    else
    {
        info.ptr2 = nullptr;
        info.size2 = 0;
    }

    return true;
}

bool CRingBuffer::AdvanceRead(std::uint32_t size)
{
    this->readPtr = (this->readPtr + size) % this->size;
    return true;
}