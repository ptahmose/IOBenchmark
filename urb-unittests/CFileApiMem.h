#pragma once

#include "include_urb.h"

class CFileApiMemImpl : public IFileApi
{
private:
    std::uint8_t* ptr;
    std::uint64_t size;
public:
    CFileApiMemImpl();
    virtual void Create(const wchar_t* filename);
    virtual void WriteUnbuffered(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void WriteBuffered(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void Close();
    virtual ~CFileApiMemImpl();

    const std::uint8_t* GetDataPtr() const { return this->ptr; }
    std::uint64_t GetDataSize() const { return this->size; }
private:
    void ResizeTo(std::uint64_t s);
};