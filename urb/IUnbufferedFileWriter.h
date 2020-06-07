#pragma once

#include <cinttypes>

class IUnbufferedFileWriter
{
public:
    virtual void InitializeFile(const wchar_t* filename) = 0;
    virtual bool TryAppendNoWait(const void* ptr, std::uint32_t size) = 0;

    virtual void Rewrite(std::uint64_t offset, const void* ptr, std::uint32_t size) = 0;

    virtual void Close() = 0;
};


class IUnbufferedFileWriter2
{
public:
    virtual void InitializeFile(const wchar_t* filename) = 0;
    virtual bool TryAppendNoWait(std::uint64_t offset, const void* ptr, std::uint32_t size) = 0;
    virtual void OverwriteSync(std::uint64_t offset, const void* ptr, std::uint32_t size) = 0;

    virtual void Close() = 0;

    virtual ~IUnbufferedFileWriter2() {};
};
