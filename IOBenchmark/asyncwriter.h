#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <Windows.h>
#include <stdexcept>
#include <string>
#include <limits>
#include <sstream>

class AsyncWriterException : public std::runtime_error
{
private:
	bool lastErrorValid;
	DWORD lastError;
public:
	enum class Type
	{
		Intialization,
		WriteError,
		WaitError,
		CancellationError
	};

	explicit AsyncWriterException(Type type, const std::string& what_arg) :
		std::runtime_error(what_arg),
		lastErrorValid(false)
	{
	}

	explicit AsyncWriterException(Type type, const char* what_arg) :
		std::runtime_error(what_arg),
		lastErrorValid(false)
	{
	}

	void SetLastError(DWORD lastError)
	{
		this->lastError = lastError;
		this->lastErrorValid = true;
	}

	bool TryGetLastError(DWORD* ptrLastError)
	{
		if (this->lastErrorValid)
		{
			if (ptrLastError != nullptr)
			{
				*ptrLastError = this->lastError;
			}
		}

		return this->lastErrorValid;
	}
};

struct AsyncWriterNullEnv
{
	void Assert(bool, const char* sz) {}
};

/// An asynchronous writer 2.
/// \tparam t The "data" generic type parameter. A shared pointer to an instance is representing the 
/// 		  data, and this type must support the operations "uint32_t size()" (giving the size of data in bytes)
/// 		  and "const void* operator()" (giving a pointer to the data).
template <class tData, class tEnv = AsyncWriterNullEnv>
class AsyncWriter2
{
private:
	struct WriteOperationData
	{
		ULONGLONG fileOffset;
		std::shared_ptr<tData> data;
	};

	struct ErrorState
	{
		ErrorState() : isInErrorState(false) {}
		bool isInErrorState;
		std::string information;
		DWORD lastErrorCode;
	};

	struct NullEnv
	{
		void Assert(bool, const char* sz) {}
	};
private:
	tEnv env;
	ErrorState errorState;
	HANDLE hFile;
	int maxNoOfPendingWrites;
	std::uint64_t maxPendingBytes;
	std::vector< WriteOperationData> writeData;
	std::vector<OVERLAPPED> overlapped;
	std::vector<HANDLE> events;
	std::vector<bool> activeWrites;
	int noOfActiveWrites;
	std::uint64_t pendingBytes;

public:
	AsyncWriter2(HANDLE h, int maxNoOfPendingWrites) :
		AsyncWriter2(tEnv(), h, maxNoOfPendingWrites)
	{
	}

	AsyncWriter2(tEnv env, HANDLE h, int maxNoOfPendingWrites) :
		AsyncWriter2(env, h, maxNoOfPendingWrites, (std::numeric_limits<uint64_t>::max)())
	{
	}

	AsyncWriter2(tEnv env, HANDLE h, int maxNoOfPendingWrites, std::uint64_t maxPendingBytes) :
		env(env),
		hFile(h),
		writeData(maxNoOfPendingWrites),
		overlapped(maxNoOfPendingWrites),
		activeWrites(maxNoOfPendingWrites, false),
		events(maxNoOfPendingWrites, NULL),
		noOfActiveWrites(0),
		maxNoOfPendingWrites(maxNoOfPendingWrites),
		maxPendingBytes(maxPendingBytes),
		pendingBytes(0)
	{
		for (int i = 0; i < maxNoOfPendingWrites; ++i)
		{
			ZeroMemory(&(this->overlapped[i]), sizeof(OVERLAPPED));
			this->events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);

			if (this->events[i] == NULL)
			{
				this->CloseEventHandles();
				AsyncWriterException excp(AsyncWriterException::Type::Intialization, "Error creating events");
				excp.SetLastError(GetLastError());
				throw excp;
			}
		}
	}

	~AsyncWriter2()
	{
		if (this->AreWritesPending())
		{
			this->CancelPendingWrites();
			this->WaitUntilNoPendingWrites();
		}

		this->ClearAllFinishedSlots();

		this->CloseEventHandles();
	}

	bool IsInErrorState() const
	{
		return this->errorState.isInErrorState;
	}

	/// Determine if we the are pending write operations.
	/// \returns True if there are pending write operations, false if not.
	bool AreWritesPending() const
	{
		return this->noOfActiveWrites > 0;
	}

	/// Gets number of pending write operations.
	/// \returns The number of pending write operations.
	int GetNumberOfPendingWrites() const
	{
		return this->noOfActiveWrites;
	}

	/// Gets the sum of bytes which are pending to be written.
	/// \returns The sum of pending bytes to be written.
	std::uint64_t GetPendingBytesToBeWritten() const
	{
		return this->pendingBytes;
	}

	/// Adds a 'data'-object to be written.
	/// \exception lastError Raised when a last error condition occurs.
	/// \param offset The file offset at which the data is to be written.
	/// \param data   The  'data'-object.
	/// \returns True if it succeeds, false if it fails.
	bool AddWrite(ULONGLONG offset, std::shared_ptr<tData> data)
	{
		if (this->noOfActiveWrites == this->maxNoOfPendingWrites)
		{
			return false;
		}

		if (this->maxNoOfPendingWrites != (std::numeric_limits<uint64_t>::max)())
		{
			auto size = data->size();
			if (size < this->maxPendingBytes)
			{
				if (this->pendingBytes + size > this->maxPendingBytes)
				{
					return false;
				}
			}
			else
			{
				// if the size of this Write by itself exceeds the "maxPendingBytes" value, we deal with this
				// in the following way - we allow the write if it is the only currently active write operation
				if (this->AreWritesPending())
				{
					return false;
				}
			}
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
		if (B == FALSE)
		{
			DWORD lastError = GetLastError();
			switch (lastError)
			{
			case ERROR_IO_PENDING:
				break;
			default:
				AsyncWriterException excp(AsyncWriterException::Type::WriteError, "Error writing data");
				excp.SetLastError(lastError);
				throw lastError;
			}
		}

		this->activeWrites[idxOfEmptySlot] = true;
		this->noOfActiveWrites++;
		this->pendingBytes += data->size();

		return true;
	}

	void WaitUntilSlotIsAvailable()
	{
		if (this->noOfActiveWrites < this->maxNoOfPendingWrites)
		{
			return;
		}

		DWORD dw = WaitForMultipleObjects(this->noOfActiveWrites, &this->events[0], FALSE, INFINITE);

		if (!(dw >= WAIT_OBJECT_0 && dw <= WAIT_OBJECT_0 + this->noOfActiveWrites))
		{
			AsyncWriterException excp(AsyncWriterException::Type::WaitError, "Error waiting for pending operations");
			if (dw == WAIT_FAILED)
			{
				excp.SetLastError(GetLastError());
			}

			throw excp;
		}

		int idxOfWriteOperationCompleted = dw - WAIT_OBJECT_0;
		this->ClearWrite(idxOfWriteOperationCompleted);
		return;
	}

	int ClearAllFinishedSlots()
	{
		if (this->noOfActiveWrites == 0)
		{
			return 0;
		}

		auto handlesAndIndices = this->GetUnfinishedWritesEventHandles();

		int pendingWritesFinished = 0;
		for (;;)
		{
			DWORD dw = WaitForMultipleObjects((DWORD)std::get<0>(handlesAndIndices).size(), &std::get<0>(handlesAndIndices)[0], FALSE, 0);

			if (dw == WAIT_TIMEOUT)
			{
				break;
			}

			if (dw >= WAIT_OBJECT_0 && dw <= WAIT_OBJECT_0 + std::get<0>(handlesAndIndices).size())
			{
				pendingWritesFinished++;
				int idx = dw - WAIT_OBJECT_0;
				int idxOfFinishedWrite = std::get<1>(handlesAndIndices)[idx];
				this->ClearWrite(idxOfFinishedWrite);
				if (std::get<0>(handlesAndIndices).size() > 1)
				{
					std::get<0>(handlesAndIndices).erase(std::get<0>(handlesAndIndices).begin() + idx);
					std::get<1>(handlesAndIndices).erase(std::get<1>(handlesAndIndices).begin() + idx);
					continue;
				}
				else
				{
					break;
				}
			}

			// everything else is an error
			AsyncWriterException excp(AsyncWriterException::Type::WaitError, "Error waiting for pending operations");
			if (dw == WAIT_FAILED)
			{
				excp.SetLastError(GetLastError());
			}

			throw excp;
		}

		return pendingWritesFinished;
	}

	void WaitUntilNoPendingWrites()
	{
		if (this->noOfActiveWrites == 0)
		{
			return;
		}

		auto handlesAndIndices = this->GetUnfinishedWritesEventHandles();
		DWORD dw = WaitForMultipleObjects((DWORD)std::get<0>(handlesAndIndices).size(), &std::get<0>(handlesAndIndices)[0], TRUE, INFINITE);
		if (dw >= WAIT_OBJECT_0 && dw <= WAIT_OBJECT_0 + std::get<0>(handlesAndIndices).size())
		{
			return;
		}

		// everything else is an error
		AsyncWriterException excp(AsyncWriterException::Type::WaitError, "Error waiting for pending operations");
		if (dw == WAIT_FAILED)
		{
			excp.SetLastError(GetLastError());
		}

		throw excp;
	}

	bool CancelPendingWrites()
	{
		if (!this->AreWritesPending())
		{
			return false;
		}

		BOOL B = CancelIo(this->hFile);
		if (B == FALSE)
		{
			AsyncWriterException excp(AsyncWriterException::Type::CancellationError, "Error cancelling pending operations");
			excp.SetLastError(GetLastError());
			throw excp;
		}

		return true;
	}
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

	void SetErrorState(std::string message, DWORD lastError)
	{
		if (!this->errorState.isInErrorState)
		{
			this->errorState.isInErrorState = true;
			this->errorState.information = message;
			this->errorState.lastErrorCode = lastError;
		}
	}

	std::tuple<std::vector<HANDLE>, std::vector<int>> GetUnfinishedWritesEventHandles()
	{
		std::vector<HANDLE> handles;
		std::vector<int> indices;
		handles.reserve(this->noOfActiveWrites);
		indices.reserve(this->noOfActiveWrites);
		for (int i = 0; i < this->maxNoOfPendingWrites; ++i)
		{
			if (this->activeWrites[i] == true)
			{
				handles.push_back(this->events[i]);
				indices.push_back(i);
			}
		}

		return std::make_tuple(handles, indices);
	}

	void ClearWrite(int idxOfWriteOperationCompleted)
	{
		this->pendingBytes -= this->writeData[idxOfWriteOperationCompleted].data->size();
		this->writeData[idxOfWriteOperationCompleted].data.reset();
		this->activeWrites[idxOfWriteOperationCompleted] = false;
		this->noOfActiveWrites--;

		DWORD bytesTransfered;
		BOOL B = GetOverlappedResult(
			this->hFile,
			&this->overlapped[idxOfWriteOperationCompleted],
			&bytesTransfered,
			FALSE);
		if (!B)
		{
			std::stringstream ss;
			ss << "OverlappedResult for write at offset " << ((std::uint64_t(this->overlapped[idxOfWriteOperationCompleted].OffsetHigh) << 32) | std::uint64_t(this->overlapped[idxOfWriteOperationCompleted].Offset)) << ".";
			this->SetErrorState(ss.str(), GetLastError());
		}
		// TODO: deal with errors...
	}

	void CloseEventHandles()
	{
		for (int i = 0; i < this->maxNoOfPendingWrites; ++i)
		{
			if (this->events[i] != NULL)
			{
				CloseHandle(this->events[i]);
			}
		}
	}
};
