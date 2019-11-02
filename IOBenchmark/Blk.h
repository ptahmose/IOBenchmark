#pragma once
#include <cstdint>
#include <Windows.h>

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
		//this->FillWithIntegerCounting(startValue);
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
