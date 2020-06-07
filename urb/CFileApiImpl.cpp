#include "pch.h"
#include "CFileApiImpl.h"

#include <cassert>

CFileApiImpl::CFileApiImpl() :
    hUnbuffered(INVALID_HANDLE_VALUE),
    hBuffered(INVALID_HANDLE_VALUE)
{
}

void CFileApiImpl::Create(const wchar_t* filename)
{
    HANDLE hUnbuffered = ::CreateFileW(
        filename,
        GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
        NULL);
    HANDLE hBuffered = ReOpenFile(
        hUnbuffered,
        GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        0);

    this->hUnbuffered = hUnbuffered;
    this->hBuffered = hBuffered;
}

void CFileApiImpl::WriteUnbuffered(std::uint64_t offset, const void* ptr, std::uint32_t size)
{
    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.Offset = (DWORD)offset;
    overlapped.OffsetHigh = (DWORD)(offset >> 32);
    DWORD bytesWritten;
    BOOL B = WriteFile(
        this->hUnbuffered,
        ptr,
        size,
        &bytesWritten,
        &overlapped);
    assert(B == TRUE);
}

void CFileApiImpl::WriteBuffered(std::uint64_t offset, const void* ptr, std::uint32_t size)
{
    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.Offset = (DWORD)offset;
    overlapped.OffsetHigh = (DWORD)(offset >> 32);
    DWORD bytesWritten;
    BOOL B = WriteFile(
        this->hBuffered,
        ptr,
        size,
        &bytesWritten,
        &overlapped);
    assert(B == TRUE);
}

void CFileApiImpl::Close()
{
    if (this->hUnbuffered != INVALID_HANDLE_VALUE)
    {
        CloseHandle(this->hUnbuffered);
        this->hUnbuffered = INVALID_HANDLE_VALUE;
    }

    if (this->hBuffered != INVALID_HANDLE_VALUE)
    {
        CloseHandle(this->hBuffered);
        this->hBuffered = INVALID_HANDLE_VALUE;
    }
}

CFileApiImpl::~CFileApiImpl()
{
    this->Close();
}
