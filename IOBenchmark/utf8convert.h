#pragma once

#include <string>
#include <stdexcept>

class Utf8ConversionException : public std::runtime_error
{
public:

	// Possible conversion "directions"
	enum class ConversionType
	{
		FromUtf8ToUtf16 = 0,
		FromUtf16ToUtf8
	};


	// Initialize with error message raw C-string, last Win32 error code and conversion direction
	Utf8ConversionException(const char* message, uint32_t errorCode, ConversionType type)
		: std::runtime_error(message), _errorCode(errorCode), _conversionType(type)
	{}

	// Initialize with error message string, last Win32 error code and conversion direction
	Utf8ConversionException(const std::string& message, uint32_t errorCode, ConversionType type)
		: std::runtime_error(message), _errorCode(errorCode), _conversionType(type)
	{}

	// Retrieve error code associated to the failed conversion
	uint32_t ErrorCode() const
	{
		return this->_errorCode;
	}

	// Direction of the conversion (e.g. from UTF-8 to UTF-16)
	ConversionType Direction() const
	{
		return this->_conversionType;
	}


private:
	// Error code from GetLastError()
	uint32_t _errorCode;

	// Direction of the conversion
	ConversionType _conversionType;
};

std::wstring Utf8ToUtf16(const std::string& utf8);
std::wstring Utf8ToUtf16(const char* utf8);
std::string Utf16ToUtf8(const std::wstring& utf16);
std::string Utf16ToUtf8(const wchar_t* sz);