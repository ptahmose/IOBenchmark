#pragma once

#include <exception>
#include <string>
#include <memory>
#include "utf8convert.h"
#include <Windows.h>

class WriterException : public std::exception
{
public:
	enum class ErrorType
	{
		APIError
	};

	explicit WriterException(ErrorType type, const std::string& msg) :
		type(type),
		error_message(msg)
	{}

	virtual const char* what() const throw()
	{
		return this->error_message.c_str();
	}

	void SetHRESULT(HRESULT hr)
	{
		this->hresult = hr;
	}

	void SetLastError(DWORD dw)
	{
		this->SetHRESULT(HRESULT_FROM_WIN32(dw));
	}

	std::string GetOSError()
	{
		LPWSTR fmtMsg;
		DWORD dw = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL,
			this->hresult,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&fmtMsg,
			0,
			NULL);

		if (dw == 0)
		{
			return std::string("Could not retrieve formatted message.");
		}

		std::unique_ptr<WCHAR, void(*)(LPWSTR)> upFmtMsg(fmtMsg, [](LPWSTR p)->void {if (p != NULL) { LocalFree(p); }});
		return Utf16ToUtf8(fmtMsg);
	}

private:
	ErrorType type;

	std::string error_message;      ///< Error message

	HRESULT hresult;
};