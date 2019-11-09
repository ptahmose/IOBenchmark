#pragma once
#include <vector>
#include <functional>
#include "IWriter.h"
#include <Windows.h>
#include "asyncwriter2.h"
#include "Blk.h"

class WriterAsync2 : public IWriter
{
private:
	class Data
	{
	private:
		CBlk blk;
	public:
		Data(std::uint32_t blkSize, int startValue) :blk(blkSize, startValue) {}

		const void* operator()(void) const { return this->blk.GetData(); }
		std::uint32_t size() const { return this->blk.GetDataSize(); }
	};
private:
	static const int MaxPendingOperationCount = 1024;
	std::unique_ptr<AsyncWriter3<Data>> writer;
	HANDLE hFile;
	WriterOptions options;
public:
	WriterAsync2();

	virtual void Init(const WriterOptions& options);

	virtual void DoIt();

	virtual ~WriterAsync2();

private:

};