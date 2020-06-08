#pragma once

#include <cstdint>
#include <string>
#include <functional>

class CCmdlineArgs
{
private:
    std::wstring filename;
    std::uint64_t fileSize;
    std::uint32_t ringBufferSize;
    std::uint32_t writeOutSize;
public:
    CCmdlineArgs();
    bool ParseArguments(int argc, char** argv);

    std::wstring GetFilename() const;
    std::uint64_t GetFileSize() const;
    std::uint32_t GetRingBufferSize() const;
    std::uint32_t GetWriteOutSize() const;

private:
    void SetDefaults();

    static bool TryParseSizeUint64(const std::string& str, std::uint64_t& size);
    static bool TryParseSizeUint32(const std::string& str, std::uint32_t& size);
    static bool TryParseUint64(const std::string& str, std::uint64_t* value);
    static bool TryParseSize(const std::string& str, std::function<bool(const std::string&, const std::string&)> parseFunc);
    static bool TryGetSizeUint64(const std::string& number, const std::string& prefix, std::uint64_t& size);
    static bool TryGetSizeUint32(const std::string& number, const std::string& prefix, std::uint32_t& size);
    static std::uint64_t ParsePrefix(const std::string& str);
    static void Mult64to128(uint64_t op1, uint64_t op2, uint64_t* hi, uint64_t* lo);
};