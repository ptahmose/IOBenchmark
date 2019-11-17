#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include "IWriter.h"
#include "propertybag.h"

class CCmdlineArgs
{
private:
    struct blkgenNameComp 
    {
        bool operator() (const std::string& lhs, const std::string& rhs) const
        {
            return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
        }
    };
private:
    std::map<std::string, std::size_t, blkgenNameComp> listOfBlockGenerators;
    std::uint32_t blkSize;
    std::uint64_t fileSize;
    std::string filename;
    IWriter::WriterType writerType;
    int logVerbosity;
    std::shared_ptr<CPropertyBag> writerSpecificPropBag;
    bool hashCodeBlkNameGeneratorValid;
    std::size_t hashCodeBlkNameGenerator;
public:
    CCmdlineArgs();
    bool ParseArguments(int argc, char** argv);

    std::uint32_t GetBlkSize() const { return this->blkSize; }
    std::uint64_t GetFileSize() const { return this->fileSize; }
    const std::string& GetFilename() const { return this->filename; }
    int GetLogVerbosity() const { return this->logVerbosity; }
    IWriter::WriterType GetWriterType() { return this->writerType; }
    std::shared_ptr<IPropertyBagRead> GetWriterSpecificPropertyBag() const { return this->writerSpecificPropBag; }

    /// Gets the type-hash for the "block generator". If no specific generator was given on the command line,
    /// we return a default here.
    /// \returns The type-hash for the "block generator".
    std::size_t GetBlockGenTypeHash() const;
private:
    bool ParseFileWriterSpecificOptions(const std::string& str);
    bool ParseBlockGeneratorType(const std::string& str);

    static bool TryParseSize(const std::string& str, std::function<bool(const std::string&, const std::string&)> parseFunc);
    static bool TryParseSizeUint32(const std::string& str, std::uint32_t& size);
    static bool TryParseSizeUint64(const std::string& str, std::uint64_t& size);

    static bool TryGetSizeUint32(const std::string& number, const std::string& prefix, std::uint32_t& size);
    static bool TryGetSizeUint64(const std::string& number, const std::string& prefix, std::uint64_t& size);

    static std::uint64_t ParsePrefix(const std::string& str);
    static bool TryParseUint64(const std::string& str, std::uint64_t* value);

    static void Mult64to128(std::uint64_t op1, std::uint64_t op2, std::uint64_t* hi, std::uint64_t* lo);
};
