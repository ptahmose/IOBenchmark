#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "WriterException.h"
#include "propertybag.h"

class IWriter
{
public:
	enum class WriterType
	{
		Invalid,
		SimpleSync,
		Async,
		Async2,
        AsyncUnbuffered
	};

	static std::shared_ptr<IWriter> CreateInstance(WriterType type);
public:
	struct WriterOptions
	{
		std::uint32_t blkSize;
		std::uint64_t fileSize;
		std::string filename;

        /// A hash-code (as returned by std::type_info::hash_code) identifying the block generator to be used.
        std::size_t blkGenHashCode;
	};

	virtual void Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions) = 0;

	virtual void DoIt() = 0;

	virtual ~IWriter() {}
};

