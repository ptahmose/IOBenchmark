#include "WriterAsync.h"
#include "Blk.h"
#include "utf8convert.h"
#include <cstdint>
#include <sstream>

using namespace std;

//AsyncWriter::AsyncWriter(HANDLE h, int maxNoOfPendingWrites) :
//	hFile(h),
//	writeData(maxNoOfPendingWrites),
//	overlapped(maxNoOfPendingWrites),
//	activeWrites(maxNoOfPendingWrites, false),
//	events(maxNoOfPendingWrites),
//	noOfActiveWrites(0),
//	maxNoOfPendingWrites(maxNoOfPendingWrites)
//{
//	for (int i = 0; i < maxNoOfPendingWrites; ++i)
//	{
//		ZeroMemory(&(this->overlapped[i]), sizeof(OVERLAPPED));
//		this->events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
//	}
//}
//
//bool AsyncWriter::AddWrite(ULONGLONG offset, const void* ptrData, DWORD dataSize, std::function<void(const void*)> deleteFunctor)
//{
//	if (this->noOfActiveWrites == this->maxNoOfPendingWrites)
//	{
//		return false;
//	}
//
//	int idxOfEmptySlot = this->GetFirstEmptySlot();
//
//	ZeroMemory(&(this->overlapped[idxOfEmptySlot]), sizeof(OVERLAPPED));
//	this->overlapped[idxOfEmptySlot].hEvent = this->events[idxOfEmptySlot];
//	this->overlapped[idxOfEmptySlot].Offset = (DWORD)offset;
//	this->overlapped[idxOfEmptySlot].OffsetHigh = (DWORD)(offset >> 32);
//
//	// does not seem to be necessary, the event is reset automatically (-> https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile )
//	//ResetEvent(this->overlapped[idxOfEmptySlot].hEvent);
//
//	this->writeData[idxOfEmptySlot].fileOffset = offset;
//	this->writeData[idxOfEmptySlot].ptrData = ptrData;
//	this->writeData[idxOfEmptySlot].dataSize = dataSize;
//	this->writeData[idxOfEmptySlot].deleteFunctor = deleteFunctor;
//
//	BOOL B = WriteFile(
//		this->hFile,
//		ptrData,
//		dataSize,
//		NULL,
//		&this->overlapped[idxOfEmptySlot]);
//
//	this->activeWrites[idxOfEmptySlot] = true;
//	this->noOfActiveWrites++;
//
//	return true;
//}
//
//int AsyncWriter::GetFirstEmptySlot()
//{
//	for (int i = 0; i < maxNoOfPendingWrites; ++i)
//	{
//		if (this->activeWrites[i] == false)
//		{
//			return i;
//		}
//	}
//
//	return -1;
//}
//
//void AsyncWriter::WaitUntilSlotsAreAvailable()
//{
//	if (this->noOfActiveWrites < this->maxNoOfPendingWrites)
//	{
//		return;
//	}
//
//	DWORD dw = WaitForMultipleObjects(this->noOfActiveWrites, &this->events[0], FALSE, INFINITE);
//
//	int idxOfWriteOperationCompleted = dw - WAIT_OBJECT_0;
//
//	this->writeData[idxOfWriteOperationCompleted].deleteFunctor(this->writeData[idxOfWriteOperationCompleted].ptrData);
//
//	this->activeWrites[idxOfWriteOperationCompleted] = false;
//	this->noOfActiveWrites--;
//}

//---------------------------------------------------------------------------------------

WriterAsync::WriterAsync() :
	hFile(INVALID_HANDLE_VALUE)
{
}

/*virtual*/void WriterAsync::Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions)
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

            if (maxPendingOperationsCount > MAXIMUM_WAIT_OBJECTS)
            {
                stringstream ss;
                ss << "Parameter 'MaxPendingOperations' must be less than or equal to " << MAXIMUM_WAIT_OBJECTS << ".";
                throw invalid_argument(ss.str());
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

	this->writer = make_unique<AsyncWriter2<Data>>(h, maxPendingOperationsCount);
}

/*virtual*/void WriterAsync::DoIt()
{
	int startValue = 0;
	for (uint64_t totalBytesWritten = 0; totalBytesWritten < this->options.fileSize;)
	{
		auto blk = make_shared<Data>(this->options.blkGenHashCode, this->options.blkSize, startValue);

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
        startValue = blk->BlkGen().NextState();
	}

	this->writer->WaitUntilNoPendingWrites();

	//for (;;)
	//{


	//	for (;;)
	//	{
	//		bool b = this->StartWrite();
	//		if (b == false)
	//		{
	//			break;
	//		}
	//	}

	//	DWORD dw = WaitForMultipleObjects(MaxPendingOperationCount, &this->events[0], FALSE, INFINITE);

	//}


	//for (uint64_t totalBytesWritten = 0; totalBytesWritten < this->options.fileSize;)
	//{
	//	for (;;)
	//	{
	//		bool 
	//	}


	//	DWORD bytesWritten;
	//	CBlk blk(this->options.blkSize);
	//	WriteFile(
	//		this->hFile,
	//		blk.GetData(),
	//		blk.GetDataSize(),
	//		&bytesWritten,
	//		NULL);

	//	totalBytesWritten += bytesWritten;
	//}
}

//bool WriterAsync::StartWrite()
//{
//
//}

/*virtual*/WriterAsync::~WriterAsync()
{
	this->writer.reset();

	if (this->hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(this->hFile);
	}
}