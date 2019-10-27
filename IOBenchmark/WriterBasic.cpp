#include "WriterBasic.h"
#include "Blk.h"
#include "utf8convert.h"
#include <cstdint>

using namespace std;

WriterBasic::WriterBasic() : hFile(INVALID_HANDLE_VALUE)
{
}

/*virtual*/void WriterBasic::Init(const WriterOptions& options)
{
	const auto filenameW = Utf8ToUtf16(options.filename);
	HANDLE h = CreateFileW(
		filenameW.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	this->options = options;
	this->hFile = h;
}

/*virtual*/void WriterBasic::DoIt()
{
	for (uint64_t totalBytesWritten = 0; totalBytesWritten < this->options.fileSize;)
	{
		DWORD bytesWritten;
		CBlk blk(this->options.blkSize);
		WriteFile(
			this->hFile,
			blk.GetData(),
			blk.GetDataSize(),
			&bytesWritten,
			NULL);

		totalBytesWritten += bytesWritten;
	}
}

/*virtual*/WriterBasic::~WriterBasic()
{
	if (this->hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(this->hFile);
	}
}