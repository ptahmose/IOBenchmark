#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <Windows.h>

template <class t>
class AsyncWriter2
{
private:
	struct WriteOperationData
	{
		ULONGLONG fileOffset;
		std::shared_ptr<t> data;
		//const void* ptrData;
		//DWORD dataSize;
		//std::function<void(const void*)> deleteFunctor;
	};
private:
	HANDLE hFile;
	int maxNoOfPendingWrites;
	std::vector< WriteOperationData> writeData;
	std::vector<OVERLAPPED> overlapped;
	std::vector<HANDLE> events;
	std::vector<bool> activeWrites;
	int noOfActiveWrites;
public:
	AsyncWriter2(HANDLE h, int maxNoOfPendingWrites) :
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

	bool AddWrite(ULONGLONG offset, std::shared_ptr<t> data)
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

		// does not seem to be necessary, the event is reset automatically (-> https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile )
		//ResetEvent(this->overlapped[idxOfEmptySlot].hEvent);

		this->writeData[idxOfEmptySlot].fileOffset = offset;
		this->writeData[idxOfEmptySlot].data = data;

		BOOL B = WriteFile(
			this->hFile,
			data->operator()(),
			data->size(),
			NULL,
			&this->overlapped[idxOfEmptySlot]);

		this->activeWrites[idxOfEmptySlot] = true;
		this->noOfActiveWrites++;

		return true;
	}

	void WaitUntilSlotsAreAvailable()
	{
		if (this->noOfActiveWrites < this->maxNoOfPendingWrites)
		{
			return;
		}

		DWORD dw = WaitForMultipleObjects(this->noOfActiveWrites, &this->events[0], FALSE, INFINITE);

		int idxOfWriteOperationCompleted = dw - WAIT_OBJECT_0;

		this->writeData[idxOfWriteOperationCompleted].data.reset();

		this->activeWrites[idxOfWriteOperationCompleted] = false;
		this->noOfActiveWrites--;
	}

	void WaitUntilNoPendingWrites();
private:
	int GetFirstEmptySlot()
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
};