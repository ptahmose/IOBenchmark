#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "WriterException.h"

class IWriter
{
public:
	enum class WriterType
	{
		Invalid,
		SimpleSync,
		Async
	};

	static std::shared_ptr<IWriter> CreateInstance(WriterType type);
public:
	struct WriterOptions
	{
		std::uint32_t blkSize;
		std::uint64_t fileSize;
		std::string filename;
	};

	virtual void Init(const WriterOptions& options) = 0;

	virtual void DoIt() = 0;

	virtual ~IWriter() {}
};

