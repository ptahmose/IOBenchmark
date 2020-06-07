#pragma once

#include "IFileApi.h"


class CFileApiImpl : public IFileApi
{
private:
    HANDLE hUnbuffered;
    HANDLE hBuffered;
public:
    CFileApiImpl();
    virtual void Create(const wchar_t* filename);
    virtual void WriteUnbuffered(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void WriteBuffered(std::uint64_t offset, const void* ptr, std::uint32_t size);
    virtual void Close();
    virtual ~CFileApiImpl();
};