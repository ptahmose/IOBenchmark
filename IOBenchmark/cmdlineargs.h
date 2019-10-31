#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include "IWriter.h"

class CCmdlineArgs
{
private:
	std::uint32_t blkSize;
	std::uint64_t fileSize;
	std::string filename;
	IWriter::WriterType writerType;
public:
	CCmdlineArgs();
	bool ParseArguments(int argc, char** argv);

	std::uint32_t GetBlkSize() { return this->blkSize; }
	std::uint64_t GetFileSize() { return this->fileSize; }
	const std::string& GetFilename() { return this->filename; }

private:
	static bool TryParseSize(const std::string& str, std::function<bool(const std::string&, const std::string&)> parseFunc);
	static bool TryParseSizeUint32(const std::string& str, std::uint32_t& size);
	static bool TryParseSizeUint64(const std::string& str, std::uint64_t& size);

	static bool TryGetSizeUint32(const std::string& number, const std::string& prefix, std::uint32_t& size);
	static bool TryGetSizeUint64(const std::string& number, const std::string& prefix, std::uint64_t& size);

	static std::uint64_t ParsePrefix(const std::string& str);
	static bool TryParseUint64(const std::string& str, std::uint64_t* value);

	static void Mult64to128(std::uint64_t op1, std::uint64_t op2, std::uint64_t* hi, std::uint64_t* lo);
};
