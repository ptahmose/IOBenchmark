#pragma once
#include <vector>
#include <functional>
#include "IWriter.h"
#include <Windows.h>
#include "asyncwriter2.h"
#include "Blk.h"

class WriterAsyncUnbuffered : public IWriter
{
private:
    HANDLE hFile;
public:
    WriterAsyncUnbuffered();

    virtual void Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions);

    virtual void DoIt();

    virtual ~WriterAsyncUnbuffered();

private:

    void DetermineAlignmentInformation(HANDLE hFile, LPCWSTR szFilename);

};