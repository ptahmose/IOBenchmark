#pragma once

#include <string>
#include <cstdint>

class IWriter
{
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
