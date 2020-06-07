#pragma once

#include <memory>

#include "IUnbufferedFileWriter.h"
#include "RingBufferManager.h"

class CUnbufferedFileWriter : public IUnbufferedFileWriter
{
private:
    HANDLE hUnbuffered, hBuffered;

    std::unique_ptr<CRingBufferManager> ringBufferManager;
public:
    CUnbufferedFileWriter();
    virtual void InitializeFile(const wchar_t* filename);
    virtual bool TryAppendNoWait(const void* ptr, std::uint32_t size);
    virtual void Rewrite(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void Close();
private:
    bool WriteFunc(const void* ptr, std::uint32_t size);
};
