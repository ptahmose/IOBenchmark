#pragma once
#include <vector>
#include <functional>
#include "IWriter.h"
#include <Windows.h>
#include "asyncwriter2.h"
#include "Blk.h"
#include "StorageAlignmentInfo.h"

class WriterAsyncUnbuffered : public IWriter
{
private:
    class Data
    {
    private:
        std::unique_ptr<CBlkGenBase> blk;
    public:
        //Data(std::uint32_t blkSize, int startValue) :blk(blkSize, startValue) {}
        Data(size_t type_hashcode, std::uint32_t blkSize, int startValue) : blk(CreateBlkGenUniquePtr(type_hashcode, blkSize, startValue)) {}

        const CBlkGenBase& BlkGen() const { return *this->blk.get(); }
        const void* operator()(void) const { return this->blk->GetData(); }
        std::uint32_t size() const { return this->blk->GetDataSize(); }
    };
private:
    HANDLE hFile;
    StorageAlignmentInfo storageAlignmentInfo;

    WriterOptions options;
    std::unique_ptr<AsyncWriter3<Data>> writer;
public:
    WriterAsyncUnbuffered();

    virtual void Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions);

    virtual void DoIt();

    virtual ~WriterAsyncUnbuffered();

private:

    void DetermineAlignmentInformation(HANDLE hFile, LPCWSTR szFilename);

};