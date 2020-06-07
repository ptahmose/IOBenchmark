#pragma once

#include <cinttypes>

class IFileApi
{
public:
    virtual void Create(const wchar_t* filename) = 0;
    virtual void WriteUnbuffered(std::uint64_t offset, const void* ptr, std::uint32_t size) = 0;
    virtual void WriteBuffered(std::uint64_t offset, const void* ptr, std::uint32_t size) = 0;
    virtual void Close() = 0;
    virtual ~IFileApi() {};
};