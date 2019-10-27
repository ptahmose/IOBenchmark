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

	const void* GetData() const
	{
		return this->ptrData;
	}

	std::uint32_t GetDataSize() const
	{
		return this->blkSize;
	}

	~CBlk()
	{
		HeapFree(GetProcessHeap(), 0, this->ptrData);
	}
};
