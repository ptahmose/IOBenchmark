#include "cmdlineargs.h"
#include "json.hpp"
#include "args.hxx"
#include <string>
#include <iostream>
#include <regex>
#include <intrin.h>
#include "Blk.h"

using namespace std;
using namespace nlohmann;

CCmdlineArgs::CCmdlineArgs()
    : blkSize(0), fileSize(0), writerType(IWriter::WriterType::Invalid), hashCodeBlkNameGeneratorValid(false)
{
    this->listOfBlockGenerators.emplace("Zero", typeid(CBlkGenZero).hash_code());
    this->listOfBlockGenerators.emplace("ByteCounter", typeid(CBlkGenCounterByte).hash_code());
    this->listOfBlockGenerators.emplace("Uint32Counter", typeid(CBlkGenCounterUInt32).hash_code());
}

bool CCmdlineArgs::ParseArguments(int argc, char** argv)
{
    std::unordered_map<std::string, IWriter::WriterType> map{
       {"default", IWriter::WriterType::SimpleSync},
       {"sync", IWriter::WriterType::SimpleSync},
       {"async", IWriter::WriterType::Async},
       {"async2", IWriter::WriterType::Async2},
       {"asyncunbuffered", IWriter::WriterType::AsyncUnbuffered},
       {"unbuffered", IWriter::WriterType::Unbuffered}
      };

    args::ArgumentParser parser("IOBenchmark");
    args::ValueFlag<string> blocksizeString(parser, "blocksize", "The blocksize", { 'b',"blocksize" });
    args::ValueFlag<string> filesizeString(parser, "filesize", "The filesize", { 'f',"filesize" });
    args::ValueFlag<string> writerSpecificParamsString(parser, "params", "writer specific parameters", { 'p',"params" });
    args::ValueFlag<string> blockGeneratorTypeString(parser, "blkgen", "block generator type", { 'g',"blkgentype" });
    args::Positional<std::string> filenameArg(parser, "filename", "The filename");
    args::MapFlag<std::string, IWriter::WriterType> writerType(parser, "WriterType", "Type of writer", { 'w', "writertype" }, map);
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return true;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return false;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return false;
    }

    if (blocksizeString)
    {
        std::cout << "blocksize: " << args::get(blocksizeString) << std::endl;

        uint32_t blocksize;
        CCmdlineArgs::TryParseSizeUint32(args::get(blocksizeString), blocksize);
        this->blkSize = blocksize;
    }

    if (filesizeString)
    {
        std::cout << "filesize: " << args::get(filesizeString) << std::endl;

        uint64_t filesize;
        CCmdlineArgs::TryParseSizeUint64(args::get(filesizeString), filesize);
        this->fileSize = filesize;
    }

    if (writerType)
    {
        this->writerType = args::get(writerType);
    }

    if (writerSpecificParamsString)
    {
        this->ParseFileWriterSpecificOptions(args::get(writerSpecificParamsString));
    }

    if (blockGeneratorTypeString)
    {
        this->ParseBlockGeneratorType(args::get(blockGeneratorTypeString));
    }

    if (filenameArg)
    {
        std::cout << "filename: " << args::get(filenameArg) << std::endl;
        this->filename = args::get(filenameArg);
    }

    return true;
}

/*static*/bool CCmdlineArgs::TryParseSize(const std::string& str, std::function<bool(const std::string&, const std::string&)> parseFunc)
{
    const std::regex regex("[[:blank:]]*([[:digit:]]+)[[:blank:]]*(ki|Ki|Mi|Gi|Ti|Pi|k|K|M|G|T|P)?[[:blank:]]*");
    std::smatch match;
    if (std::regex_match(str, match, regex))
    {
        if (match.size() == 3 && match[1].matched == true)
        {
            return parseFunc(match[1].str(), match[2].matched ? match[2].str() : "");
        }
    }

    return false;
}

/*static*/bool CCmdlineArgs::TryParseSizeUint32(const std::string& str, std::uint32_t& size)
{
    return TryParseSize(
        str,
        [&](const std::string& number, const std::string& prefix)->bool
    {
        return TryGetSizeUint32(number, prefix, size);
    });
}

/*static*/bool CCmdlineArgs::TryParseSizeUint64(const std::string& str, std::uint64_t& size)
{
    return TryParseSize(
        str,
        [&](const std::string& number, const std::string& prefix)->bool
    {
        return TryGetSizeUint64(number, prefix, size);
    });
}


/*static*/bool CCmdlineArgs::TryGetSizeUint32(const std::string& number, const std::string& prefix, std::uint32_t& size)
{
    if (prefix.empty())
    {
        uint64_t v;
        bool b = TryParseUint64(number, &v);
        if (b == true && v <= (std::numeric_limits<uint32_t>::max)())
        {
            size = (uint32_t)v;
            return true;
        }

        return false;
    }
    else
    {
        uint64_t v;
        bool b = TryParseUint64(number, &v);
        if (b == true && v <= (std::numeric_limits<uint32_t>::max)())
        {
            uint64_t mul = ParsePrefix(prefix);
            if (mul == 0)
            {
                return false;
            }

            uint64_t prodLo, prodHi;
            Mult64to128(mul, v, &prodHi, &prodLo);
            if (prodHi > 0 || prodLo > (std::numeric_limits<uint32_t>::max)())
            {
                return false;
            }

            size = (uint32_t)prodLo;
            return true;
        }
    }

    return false;
}

/*static*/bool CCmdlineArgs::TryGetSizeUint64(const std::string& number, const std::string& prefix, std::uint64_t& size)
{
    if (prefix.empty())
    {
        uint64_t v;
        bool b = TryParseUint64(number, &v);
        if (b == true)
        {
            size = v;
            return true;
        }

        return false;
    }
    else
    {
        uint64_t v;
        bool b = TryParseUint64(number, &v);
        if (b == true && v <= (std::numeric_limits<uint32_t>::max)())
        {
            uint64_t mul = ParsePrefix(prefix);
            if (mul == 0)
            {
                return false;
            }

            uint64_t prodLo, prodHi;
            Mult64to128(mul, v, &prodHi, &prodLo);
            if (prodHi > 0)
            {
                return false;
            }

            size = prodLo;
            return true;
        }
    }

    return false;
}

/*static*/std::uint64_t CCmdlineArgs::ParsePrefix(const std::string& str)
{
    if (str == "ki" || str == "Ki")
    {
        return 1024;
    }

    if (str == "Mi")
    {
        return 1048576;
    }

    if (str == "Gi")
    {
        return 1073741824;
    }

    if (str == "Ti")
    {
        return 1099511627776ull;
    }

    if (str == "Pi")
    {
        return 1125899906842624ull;
    }

    if (str == "k" || str == "K")
    {
        return 1000;
    }

    if (str == "M")
    {
        return 1000000;
    }

    if (str == "G")
    {
        return 1000000000;
    }

    if (str == "T")
    {
        return 1000000000000ull;
    }

    if (str == "P")
    {
        return 1000000000000000ull;
    }

    return 0;
}

/*static*/bool CCmdlineArgs::TryParseUint64(const std::string& str, std::uint64_t* value)
{
    size_t s;
    try
    {
        uint64_t v = std::stoull(str.c_str(), &s, 10);
        if (s == str.length())
        {
            if (value != nullptr)
            {
                *value = v;
            }

            return true;
        }
    }
    catch (std::invalid_argument&)
    {
        return false;
    }
    catch (std::out_of_range&)
    {
        return false;
    }

    return false;
}

/*static*/ void CCmdlineArgs::Mult64to128(uint64_t op1, uint64_t op2, uint64_t* hi, uint64_t* lo)
{
    *lo = _umul128(op1, op2, hi);
    /*
    // portable version
    uint64_t u1 = (op1 & 0xffffffff);
    uint64_t v1 = (op2 & 0xffffffff);
    uint64_t t = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k = (t >> 32);

    op1 >>= 32;
    t = (op1 * v1) + k;
    k = (t & 0xffffffff);
    uint64_t w1 = (t >> 32);

    op2 >>= 32;
    t = (u1 * op2) + k;
    k = (t >> 32);

    if (hi != nullptr)
    {
        *hi = (op1 * op2) + w1 + k;
    }

    if (lo != nullptr)
    {
        *lo = (t << 32) + w3;
    }
    */
}

bool CCmdlineArgs::ParseFileWriterSpecificOptions(const std::string& str)
{
    this->writerSpecificPropBag = make_shared<CPropertyBag>();
    try
    {
        auto json = json::parse(str);
        for (auto& [key, value] : json.items())
        {
            if (value.is_boolean())
            {
                auto b = value.get<bool>();
                this->writerSpecificPropBag->AddItem_Bool(key, b);
            }
            else if (value.is_number_integer())
            {
                auto n = value.get<int>();
                this->writerSpecificPropBag->AddItem_Int32(key, n);
            }
            else if (value.is_string())
            {
                auto s = value.get<string>();
                this->writerSpecificPropBag->AddItem_String(key, s);
            }
            else
            {
                return false;
            }
        }
    }
    catch (json::parse_error & e)
    {
        return false;
    }

    return true;
}

bool CCmdlineArgs::ParseBlockGeneratorType(const std::string& str)
{
    const auto gen = this->listOfBlockGenerators.find(str);
    if (gen == this->listOfBlockGenerators.cend())
    {
        return false;
    }

    this->hashCodeBlkNameGenerator = gen->second;
    this->hashCodeBlkNameGeneratorValid = true;
    return true;
}

std::size_t CCmdlineArgs::GetBlockGenTypeHash() const
{
    if (this->hashCodeBlkNameGeneratorValid)
    {
        return this->hashCodeBlkNameGenerator;
    }

    return typeid(CBlkGenZero).hash_code();
}
