#include "Blk.h"

using namespace std;

unique_ptr<CBlkGenBase> CreateBlkGenUniquePtr(std::size_t type_hashcode, std::uint32_t blkSize, int state)
{
    if (type_hashcode == typeid(CBlkGenZero).hash_code())
    {
        return make_unique<CBlkGenZero>(blkSize, state);
    }

    if (type_hashcode == typeid(CBlkGenCounterByte).hash_code())
    {
        return make_unique<CBlkGenCounterByte>(blkSize, state);
    }

    if (type_hashcode == typeid(CBlkGenCounterUInt32).hash_code())
    {
        return make_unique<CBlkGenCounterUInt32>(blkSize, state);
    }

    return unique_ptr<CBlkGenBase>();
}

/*std::shared_ptr<CBlkGenBase> CreateBlkGenSharedPtr(std::size_t type_hashcode, std::uint32_t blkSize, int state)
{
    if (type_hashcode == typeid(CBlkGenZero).hash_code())
    {
        return make_shared<CBlkGenZero>(blkSize, state);
    }

    if (type_hashcode == typeid(CBlkGenCounterByte).hash_code())
    {
        return make_shared<CBlkGenCounterByte>(blkSize, state);
    }

    if (type_hashcode == typeid(CBlkGenCounterUInt32).hash_code())
    {
        return make_shared<CBlkGenCounterUInt32>(blkSize, state);
    }

    return shared_ptr<CBlkGenBase>();
}*/