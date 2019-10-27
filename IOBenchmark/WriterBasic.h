#pragma once
#include "IWriter.h"
#include <Windows.h>

class WriterBasic : public IWriter
{
private:
	HANDLE hFile;
	WriterOptions options;
public:
	WriterBasic();

	virtual void Init(const WriterOptions& options);

	virtual void DoIt();

	virtual ~WriterBasic();
};