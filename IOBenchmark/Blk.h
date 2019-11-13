#pragma once
#include <cstdint>
#include <Windows.h>

class CBlkGenBase
{
protected:
    void* ptrData;
    std::uint32_t blkSize;
public:
    CBlkGenBase() {};
    virtual ~CBlkGenBase() {}

    virtual int NextState() const = 0;

    const void* GetData() const
    {
        return this->ptrData;
    }

    std::uint32_t GetDataSize() const
    {
        return this->blkSize;
    }
};

class CBlkGenHeapAlloc : public CBlkGenBase
{
public:
    CBlkGenHeapAlloc(std::uint32_t blkSize, int state)
    {}
protected:
    void Allocate(std::uint32_t blkSize, int state)
    {
        this->ptrData = HeapAlloc(
            GetProcessHeap(),
            0,
            blkSize);
        this->blkSize = blkSize;
    }

    virtual ~CBlkGenHeapAlloc()
    {
        HeapFree(GetProcessHeap(), 0, this->ptrData);
    }
};


class CBlkGenZero : public CBlkGenHeapAlloc
{
public:
    CBlkGenZero(std::uint32_t blkSize, int state) : CBlkGenHeapAlloc(blkSize, state)
    {
        this->Allocate(blkSize, state);
        ZeroMemory(this->ptrData, this->blkSize);
    }

    virtual int NextState() const { return 0; }
};

class CBlkGenCounterByte : public CBlkGenHeapAlloc
{
private:
    int nextState;
public:
    CBlkGenCounterByte(std::uint32_t blkSize, int state) : CBlkGenHeapAlloc(blkSize, state)
    {
        this->Allocate(blkSize, state);
        uint8_t cnt = (uint8_t)state;
        uint8_t* p = (uint8_t*)this->ptrData;
        for (uint32_t i = 0; i < this->blkSize; ++i)
        {
            p[i] = cnt++;
        }
        
        this->nextState = cnt;
    }

    virtual int NextState() const { return this->nextState; }
};


class CBlk
{
private:
    void* ptrData;
    std::uint32_t blkSize;
public:
    CBlk(std::uint32_t blkSize)
    {
        this->ptrData = HeapAlloc(
            GetProcessHeap(),
            0,
            blkSize);
        this->blkSize = blkSize;
    }

    CBlk(std::uint32_t blkSize, int startValue) : CBlk(blkSize)
    {
        this->FillWithIntegerCounting(startValue);
    }

    ~CBlk()
    {
        HeapFree(GetProcessHeap(), 0, this->ptrData);
    }

    const void* GetData() const
    {
        return this->ptrData;
    }

    std::uint32_t GetDataSize() const
    {
        return this->blkSize;
    }

private:
    void FillWithIntegerCounting(int startValue)
    {
        int* ptrDst = (int*)this->ptrData;
        for (std::uint32_t idx = 0; idx < this->blkSize / sizeof(int); ++idx)
        {
            *(ptrDst + idx) = startValue++;
        }
    }
};
