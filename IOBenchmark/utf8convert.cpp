#include "utf8convert.h"
#include <Windows.h>

std::wstring Utf8ToUtf16(const std::string & utf8)
{
    std::wstring utf16;
	if (utf8.empty())
	{
		return utf16;
	}

	// Safely fail if an invalid UTF-8 character sequence is encountered
	constexpr DWORD kFlags = MB_ERR_INVALID_CHARS;

	// Safely cast the length of the source UTF-8 string (expressed in chars)
	// from size_t (returned by std::string::length()) to int 
	// for the MultiByteToWideChar API.
	// If the size_t value is too big to be stored into an int, 
	// throw an exception to prevent conversion errors like huge size_t values 
	// converted to *negative* integers.
	if (utf8.length() > static_cast<size_t>((std::numeric_limits<int>::max)()))
	{
		throw std::overflow_error("Input string too long.");
	}
	const int utf8Length = static_cast<int>(utf8.length());

	// Get the size of the destination UTF-16 string
	const int utf16Length = ::MultiByteToWideChar(
		CP_UTF8,       // source string is in UTF-8
		kFlags,        // conversion flags
		utf8.data(),   // source UTF-8 string pointer
		utf8Length,    // length of the source UTF-8 string, in chars
		NULL,          // unused - no conversion done in this step
		0);            // request size of destination buffer, in wchar_ts

	if (utf16Length == 0)
	{
		// Conversion error: capture error code and throw
		const auto error = ::GetLastError();
		throw Utf8ConversionException(
			error == ERROR_NO_UNICODE_TRANSLATION ?
			"Invalid UTF-8 sequence found in input string."
			:
			"Cannot get result string length when converting " \
			"from UTF-8 to UTF-16 (MultiByteToWideChar failed).",
			error,
			Utf8ConversionException::ConversionType::FromUtf8ToUtf16);
	}

	utf16.resize(utf16Length);

	// Do the actual conversion from UTF-8 to UTF-16
	int result = ::MultiByteToWideChar(
		CP_UTF8,       // source string is in UTF-8
		kFlags,        // conversion flags
		utf8.data(),   // source UTF-8 string pointer
		utf8Length,    // length of source UTF-8 string, in chars
		&utf16[0],     // pointer to destination buffer
		utf16Length);  // size of destination buffer, in wchar_ts           

	if (result == 0)
	{
		// Conversion error: capture error code and throw
		const DWORD error = ::GetLastError();
		throw Utf8ConversionException(
			error == ERROR_NO_UNICODE_TRANSLATION ?
			"Invalid UTF-8 sequence found in input string."
			:
			"Cannot convert from UTF-8 to UTF-16 "\
			"(MultiByteToWideChar failed).",
			error,
			Utf8ConversionException::ConversionType::FromUtf8ToUtf16);
	}

	return utf16;
}

static std::string Utf16ToUtf8(LPCWSTR sz, size_t length)
{
	std::string utf8;
	if (length==0)
	{
		return utf8;
	}

	// Safely fail if an invalid UTF-16 character sequence is encountered
	constexpr DWORD kFlags = WC_ERR_INVALID_CHARS;

	// Safely cast the length of the source UTF-16 string (expressed in wchar_ts)
	// from size_t (returned by std::wstring::length()) to int 
	// for the WideCharToMultiByte API.
	// If the size_t value is too big to be stored into an int, 
	// throw an exception to prevent conversion errors like huge size_t values 
	// converted to *negative* integers.
	if (length > static_cast<size_t>((std::numeric_limits<int>::max)()))
	{
		throw std::overflow_error("Input string too long.");
	}

	const int utf16Length = static_cast<int>(length);

	// Get the length, in chars, of the resulting UTF-8 string
	const int utf8Length = ::WideCharToMultiByte(
		CP_UTF8,            // convert to UTF-8
		kFlags,             // conversion flags
		sz,			        // source UTF-16 string
		length,			    // length of source UTF-16 string, in wchar_ts
		NULL,               // unused - no conversion required in this step
		0,                  // request size of destination buffer, in chars
		NULL, NULL);	    // unused

	if (utf8Length == 0)
	{
		// Conversion error: capture error code and throw
		const DWORD error = ::GetLastError();
		throw Utf8ConversionException(
			error == ERROR_NO_UNICODE_TRANSLATION ?
			"Invalid UTF-16 sequence found in input string."
			:
			"Cannot get result string length when converting "\
			"from UTF-16 to UTF-8 (WideCharToMultiByte failed).",
			error,
			Utf8ConversionException::ConversionType::FromUtf16ToUtf8);
	}

	// Make room in the destination string for the converted bits
	utf8.resize(utf8Length);

	// Do the actual conversion from UTF-16 to UTF-8
	int result = ::WideCharToMultiByte(
		CP_UTF8,            // convert to UTF-8
		kFlags,             // conversion flags
		sz,			        // source UTF-16 string
		length,		        // length of source UTF-16 string, in wchar_ts
		&utf8[0],           // pointer to destination buffer
		utf8Length,         // size of destination buffer, in chars
		NULL, NULL);	    // unused

	if (result == 0)
	{
		// Conversion error: capture error code and throw
		const DWORD error = ::GetLastError();
		throw Utf8ConversionException(
			error == ERROR_NO_UNICODE_TRANSLATION ?
			"Invalid UTF-16 sequence found in input string."
			:
			"Cannot convert from UTF-16 to UTF-8 "\
			"(WideCharToMultiByte failed).",
			error,
			Utf8ConversionException::ConversionType::FromUtf16ToUtf8);
	}

	return utf8;
}

std::string Utf16ToUtf8(const std::wstring& utf16)
{
	return Utf16ToUtf8(utf16.c_str(), utf16.size());
}

std::string Utf16ToUtf8(const wchar_t* sz)
{
	return Utf16ToUtf8(sz, wcslen(sz));
}