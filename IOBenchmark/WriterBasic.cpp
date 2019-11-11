#include "WriterBasic.h"
#include "Blk.h"
#include "utf8convert.h"
#include <cstdint>
#include <sstream>

using namespace std;

WriterBasic::WriterBasic() : hFile(INVALID_HANDLE_VALUE)
{
}

/*virtual*/void WriterBasic::Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> /*writerSpecificOptions*/)
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

	if (h == INVALID_HANDLE_VALUE)
	{
		stringstream ss;
		ss << "Error when calling \"CreateFile\" with filename \"" << options.filename << ".";
		auto excp = WriterException(WriterException::ErrorType::APIError, ss.str());
		excp.SetLastError(GetLastError());
		throw excp;
	}

	this->options = options;
	this->hFile = h;
}

/*virtual*/void WriterBasic::DoIt()
{
	int startValueForFill = 0;
	for (uint64_t totalBytesWritten = 0; totalBytesWritten < this->options.fileSize;)
	{
		DWORD bytesWritten;
		CBlk blk(this->options.blkSize, startValueForFill++);
		DWORD dw = WriteFile(
			this->hFile,
			blk.GetData(),
			blk.GetDataSize(),
			&bytesWritten,
			NULL);
		if (dw != TRUE)
		{
			auto excp = WriterException(WriterException::ErrorType::APIError, "Error when calling \"WriteFile\".");
			excp.SetLastError(GetLastError());
			throw excp;
		}

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