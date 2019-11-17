#pragma once
#include <vector>
#include <functional>
#include "IWriter.h"
#include <Windows.h>

#include "asyncwriter.h"
#include "Blk.h"

//class AsyncWriter
//{
//private:
//	struct WriteOperationData
//	{
//		ULONGLONG fileOffset;
//		const void* ptrData;
//		DWORD dataSize;
//		std::function<void(const void*)> deleteFunctor;
//	};
//private:
//	HANDLE hFile;
//	int maxNoOfPendingWrites;
//	std::vector< WriteOperationData> writeData;
//	std::vector<OVERLAPPED> overlapped;
//	std::vector<HANDLE> events;
//	std::vector<bool> activeWrites;
//	int noOfActiveWrites;
//public:
//	AsyncWriter(HANDLE h, int maxNoOfPendingWrites);
//
//	bool AddWrite(ULONGLONG offset, const void* ptrData, DWORD dataSize, std::function<void(const void*)> deleteFunctor);
//
//	void WaitUntilSlotsAreAvailable();
//
//	void WaitUntilNoPendingWrites();
//private:
//	int GetFirstEmptySlot();
//};

class WriterAsync : public IWriter
{
private:
    class Data
    {
    private:
        //CBlk blk;
        std::unique_ptr<CBlkGenBase> blk;
    public:
        Data(size_t type_hashcode, std::uint32_t blkSize, int startValue) : blk(CreateBlkGenUniquePtr(type_hashcode, blkSize, startValue)) {}
        //blk(blkSize, startValue) {}

        const CBlkGenBase& BlkGen() const { return *this->blk.get(); }

        const void* operator()(void) const { return this->blk->GetData(); }
        std::uint32_t size() const { return this->blk->GetDataSize(); }
    };
private:
    static const int DefaultMaxPendingOperationCount = 5;

    std::unique_ptr<AsyncWriter2<Data>> writer;

    HANDLE hFile;
    WriterOptions options;

    /*std::vector<bool> activeWrites;
    std::vector<HANDLE> events;
    OVERLAPPED overlapped[MaxPendingOperationCount];*/
public:
    WriterAsync();

    virtual void Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions);

    virtual void DoIt();

    virtual ~WriterAsync();

private:
    //bool StartWrite();
};