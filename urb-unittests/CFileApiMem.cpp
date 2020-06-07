#include "pch.h"
#include "CFileApiMem.h"

using namespace std;

CFileApiMemImpl::CFileApiMemImpl() :
    ptr(nullptr), size(0)
{
}

void CFileApiMemImpl::Create(const wchar_t* filename)
{
}

void CFileApiMemImpl::WriteUnbuffered(std::uint64_t offset, const void* ptr, std::uint32_t size)
{
    this->ResizeTo(offset + size);
    memcpy(this->ptr + offset, ptr, size);
}

void CFileApiMemImpl::WriteBuffered(std::uint64_t offset, const void* ptr, std::uint32_t size)
{
    this->ResizeTo(offset + size);
    memcpy(this->ptr + offset, ptr, size);
}

void CFileApiMemImpl::Close()
{
}

CFileApiMemImpl::~CFileApiMemImpl()
{
    if (this->ptr!=nullptr)
    {
        free(this->ptr);
    }
}

void CFileApiMemImpl::ResizeTo(std::uint64_t s)
{
    if (this->ptr == nullptr)
    {
        this->ptr = static_cast<uint8_t*>(malloc(s));
        this->size = s;
    }
    else
    {
        if (s > this->size)
        {
            this->ptr = static_cast<uint8_t*>(realloc(this->ptr, s));
            this->size = s;
        }
    }
}
