#include "WriterAsync2.h"
#include "Blk.h"
#include "utf8convert.h"
#include <cstdint>
#include <sstream>

using namespace std;

//---------------------------------------------------------------------------------------

WriterAsync2::WriterAsync2() :
	hFile(INVALID_HANDLE_VALUE)
{
}

/*virtual*/void WriterAsync2::Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions)
{
    int maxPendingOperationsCount = DefaultMaxPendingOperationCount;
    if (writerSpecificOptions)
    {
        if (writerSpecificOptions->TryGetInt("MaxPendingOperations", &maxPendingOperationsCount) == true)
        {
            if (maxPendingOperationsCount <= 0)
            {
                throw invalid_argument("Parameter 'MaxPendingOperations' must be greater than zero.");
            }
        }
    }

	const auto filenameW = Utf8ToUtf16(options.filename);
	HANDLE h = CreateFileW(
		filenameW.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_NEW,
		FILE_FLAG_OVERLAPPED,
		NULL);
	if (h == INVALID_HANDLE_VALUE)
	{
		stringstream ss;
		ss << "Error when calling \"CreateFile\" with filename \"" << options.filename << "\".";
		auto excp = WriterException(WriterException::ErrorType::APIError, ss.str());;
		excp.SetLastError(GetLastError());
		throw excp;
	}

	this->options = options;
	this->hFile = h;

	this->writer = make_unique<AsyncWriter3<Data>>(h, maxPendingOperationsCount);
}

/*virtual*/void WriterAsync2::DoIt()
{
	int startValueForFill = 0;
	for (uint64_t totalBytesWritten = 0; totalBytesWritten < this->options.fileSize;)
	{
		auto blk = make_shared<Data>(this->options.blkSize, startValueForFill++);

		for (;;)
		{
			bool b;
			try
			{
				b = this->writer->AddWrite(
					totalBytesWritten,
					blk);
			}
			catch (AsyncWriterException awexcp)
			{
				stringstream ss;
				ss << "Error from AsyncWriter: \"" << awexcp.what() << "\".";
				auto excp = WriterException(WriterException::ErrorType::APIError, ss.str());
				throw excp;
			}

			if (b == false)
			{
				this->writer->WaitUntilSlotIsAvailable();

				this->writer->ClearAllFinishedSlots();
			}
			else
			{
				break;
			}
		}

		totalBytesWritten += this->options.blkSize;
	}

	this->writer->WaitUntilNoPendingWrites();
}

/*virtual*/WriterAsync2::~WriterAsync2()
{
	this->writer.reset();

	if (this->hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(this->hFile);
	}
}