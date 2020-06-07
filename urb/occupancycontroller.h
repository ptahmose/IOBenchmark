#pragma once
#include <memory>
#include <cinttypes>
#include <atomic>
#include <functional>

class COccupancyController
{
private:
    /// Size of the "items" which make up the bit-field in bytes.
    const size_t SizeOfBitFieldItem = sizeof(std::uint32_t);
    std::unique_ptr<std::uint32_t, void(*)(std::uint32_t*)> bitfield;
    std::uint32_t bitcnt;

    std::atomic_uint32_t bufferOccupiedCount;
    std::function<void()> allOccupiedStateChanged;
public:
    COccupancyController(std::uint32_t cnt);

    void SetAllOccupiedStateChanged(std::function<void()> func);

    bool IsOccupied(std::uint32_t idx);
    bool IsFree(std::uint32_t idx);

    void SetOccupied(std::uint32_t idx);
    void SetFree(std::uint32_t idx);

    std::uint32_t GetNumberOfBuffersFree();
    bool GetAreAllBuffersOccupied();
    bool GetIsFreeBufferAvailable();
private:
    void ClearAll();
    std::uint32_t GetCntOfBitfieldItems() const
    {
        return (this->bitcnt + (static_cast<uint32_t>(COccupancyController::SizeOfBitFieldItem) * 8 - 1)) / static_cast<uint32_t>(COccupancyController::SizeOfBitFieldItem * 8);
    }

    long* GetBitField() { return reinterpret_cast<long*>(this->bitfield.get()); }

    void IncrementOccupiedCount();
    void DecrementOccupiedCount();
};
