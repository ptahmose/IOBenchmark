#include "WriterAsync.h"
#include "Blk.h"
#include "utf8convert.h"
#include <cstdint>

using namespace std;

AsyncWriter::AsyncWriter(HANDLE h, int maxNoOfPendingWrites) :
	hFile(h),
	writeData(maxNoOfPendingWrites),
	overlapped(maxNoOfPendingWrites),
	activeWrites(maxNoOfPendingWrites, false),
	events(maxNoOfPendingWrites),
	noOfActiveWrites(0),
	maxNoOfPendingWrites(maxNoOfPendingWrites)
{
	for (int i = 0; i < maxNoOfPendingWrites; ++i)
	{
		ZeroMemory(&(this->overlapped[i]), sizeof(OVERLAPPED));
		this->events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
}

bool AsyncWriter::AddWrite(ULONGLONG offset, const void* ptrData, DWORD dataSize, std::function<void(const void*)> deleteFunctor)
{
	if (this->noOfActiveWrites == this->maxNoOfPendingWrites)
	{
		return false;
	}

	int idxOfEmptySlot = this->GetFirstEmptySlot();

	ZeroMemory(&(this->overlapped[idxOfEmptySlot]), sizeof(OVERLAPPED));
	this->overlapped[idxOfEmptySlot].hEvent = this->events[idxOfEmptySlot];
	this->overlapped[idxOfEmptySlot].Offset = (DWORD)offset;
	this->overlapped[idxOfEmptySlot].OffsetHigh = (DWORD)(offset >> 32);

	this->writeData[idxOfEmptySlot].fileOffset = offset;
	this->writeData[idxOfEmptySlot].ptrData = ptrData;
	this->writeData[idxOfEmptySlot].dataSize = dataSize;
	this->writeData[idxOfEmptySlot].deleteFunctor = deleteFunctor;

	WriteFile(
		this->hFile,
		ptrData,
		dataSize,
		NULL,
		&this->overlapped[idxOfEmptySlot]);

	this->activeWrites[idxOfEmptySlot] = true;
	this->noOfActiveWrites++;

	return true;
}

int AsyncWriter::GetFirstEmptySlot()
{
	for (int i = 0; i < maxNoOfPendingWrites; ++i)
	{
		if (this->activeWrites[i] == false)
		{
			return i;
		}
	}

	return -1;
}

void AsyncWriter::WaitUntilSlotsAreAvailable()
{
	if (this->noOfActiveWrites < this->maxNoOfPendingWrites)
	{
		return;
	}

	DWORD dw = WaitForMultipleObjects(this->noOfActiveWrites, &this->events[0], FALSE, INFINITE);

	int idxOfWriteOperationCompleted = dw - WAIT_OBJECT_0;

	this->writeData[idxOfWriteOperationCompleted].deleteFunctor(this->writeData[idxOfWriteOperationCompleted].ptrData);
	
	this->activeWrites[idxOfWriteOperationCompleted] = false;
	this->noOfActiveWrites--;
}

//---------------------------------------------------------------------------------------

WriterAsync::WriterAsync() :
	hFile(INVALID_HANDLE_VALUE),
	activeWrites(MaxPendingOperationCount, false),
	events(MaxPendingOperationCount)
{
	for (int i = 0; i < MaxPendingOperationCount; ++i)
	{
		this->events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	for (int i = 0; i < sizeof(overlapped) / sizeof(overlapped[0]); ++i)
	{
		ZeroMemory(&(this->overlapped[i]), sizeof(overlapped[0]));
	}
}

/*virtual*/void WriterAsync::Init(const WriterOptions& options)
{
	const auto filenameW = Utf8ToUtf16(options.filename);
	HANDLE h = CreateFileW(
		filenameW.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_NEW,
		FILE_FLAG_OVERLAPPED,
		NULL);

	this->options = options;
	this->hFile = h;
}

/*virtual*/void WriterAsync::DoIt()
{
	for (;;)
	{
		for (;;)
		{
			bool b = this->StartWrite();
			if (b == false)
			{
				break;
			}
		}

		DWORD dw = WaitForMultipleObjects(MaxPendingOperationCount, &this->events[0], FALSE, INFINITE);

	}


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

bool WriterAsync::StartWrite()
{

}

/*virtual*/WriterAsync::~WriterAsync()
{
	if (this->hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(this->hFile);
	}
}