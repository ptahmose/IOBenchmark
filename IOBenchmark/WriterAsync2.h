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
		//CBlk blk;
        std::unique_ptr<CBlkGenBase> blk;
	public:
		//Data(std::uint32_t blkSize, int startValue) :blk(blkSize, startValue) {}
        Data(size_t type_hashcode, std::uint32_t blkSize, int startValue) : blk(CreateBlkGenUniquePtr(type_hashcode, blkSize, startValue)) {}

        const CBlkGenBase& BlkGen() const { return *this->blk.get(); }
		const void* operator()(void) const { return this->blk->GetData(); }
		std::uint32_t size() const { return this->blk->GetDataSize(); }
	};
private:
	const int DefaultMaxPendingOperationCount = 32;
	std::unique_ptr<AsyncWriter3<Data>> writer;
	HANDLE hFile;
	WriterOptions options;
public:
	WriterAsync2();

	virtual void Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions);

	virtual void DoIt();

	virtual ~WriterAsync2();

private:

};