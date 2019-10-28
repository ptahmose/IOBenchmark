#pragma once

#pragma once
#include <vector>
#include <functional>
#include "IWriter.h"
#include <Windows.h>

class AsyncWriter
{
private:
	struct WriteOperationData
	{
		ULONGLONG fileOffset;
		const void* ptrData;
		DWORD dataSize;
		std::function<void(const void*)> deleteFunctor;
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
	AsyncWriter(HANDLE h, int maxNoOfPendingWrites);

	bool AddWrite(ULONGLONG offset, const void* ptrData, DWORD dataSize, std::function<void(const void*)> deleteFunctor);

	void WaitUntilSlotsAreAvailable();
private:
	int GetFirstEmptySlot();
};

class WriterAsync : public IWriter
{
private:
	static const int MaxPendingOperationCount = 5;

	HANDLE hFile;
	WriterOptions options;

	std::vector<bool> activeWrites;
	std::vector<HANDLE> events;
	OVERLAPPED overlapped[MaxPendingOperationCount];
public:
	WriterAsync();

	virtual void Init(const WriterOptions& options);

	virtual void DoIt();

	virtual ~WriterAsync();

private:
	bool StartWrite();
};