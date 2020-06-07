#pragma once

#include <exception>
#include <stdexcept>
#include <string>

/// Base class for all urb-specific exceptions.
class UrbException : public std::runtime_error
{
public:
	/// Constructor.
	/// \param szErrMsg Message describing the error.
	explicit UrbException(const char* szErrMsg)
		: std::runtime_error(szErrMsg)
	{}
};

/// Exception used to signal a memory allocation error.
class UrbMemoryAllocationException : public UrbException
{
public:
	/// Constructor.
	/// \param errMsg Message describing the error.
	explicit UrbMemoryAllocationException(const std::string& errMsg)
	    : UrbMemoryAllocationException(errMsg.c_str())
	{}

	/// Constructor.
	/// \param szErrMsg Message describing the error.
	explicit UrbMemoryAllocationException(const char* szErrMsg)
		: UrbException(szErrMsg)
	{}
};

/// Exception used to signal an invalid argument error.
class UrbInvalidArgumentException : public UrbException
{
public:
	/// Constructor.
	/// \param errMsg Message describing the error.
	explicit UrbInvalidArgumentException(const std::string& errMsg)
	    : UrbInvalidArgumentException(errMsg.c_str())
	{}

	/// Constructor.
	/// \param szErrMsg Message describing the error.
	explicit UrbInvalidArgumentException(const char* szErrMsg)
		: UrbException(szErrMsg)
	{}
};
