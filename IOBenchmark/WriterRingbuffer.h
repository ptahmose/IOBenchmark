#pragma once
#include "IWriter.h"
#include <Windows.h>
#include "ringbufferwriter.h"

class WriterRingbuffer : public IWriter
{
private:
	HANDLE hFile;
	WriterOptions options;
	std::unique_ptr<RingBufferWriter> ringBuff;
public:
	WriterRingbuffer();

	virtual void Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions);

	virtual void DoIt();

	virtual ~WriterRingbuffer();
private:
	bool Write(const void* p, std::uint32_t s);
};