#pragma once
#include <memory>
#include <cinttypes>

class COccupancyController
{
private:
    std::unique_ptr<std::uint32_t, void(*)(std::uint32_t*)> bitfield;
    std::uint32_t bitcnt;
public:
    COccupancyController(std::uint32_t cnt);

    bool IsOccupied(std::uint32_t idx);
    bool IsFree(std::uint32_t idx);

    void SetOccupied(std::uint32_t idx);
    void SetFree(std::uint32_t idx);
private:
    void ClearAll();
    std::uint32_t GetCntOfBitfieldItems() const
    {
        return (this->bitcnt + 31) / 32;
    }
    long* GetBitField() { return reinterpret_cast<long*>(this->bitfield.get()); }
};
