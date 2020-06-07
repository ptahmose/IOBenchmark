#include "pch.h"
#include "occupancycontroller.h"
#include <intrin.h>
#include <stdexcept>
#include <utility>

using namespace std;

COccupancyController::COccupancyController(uint32_t cnt) :
    bitfield(static_cast<uint32_t*>(
        malloc(
            ((cnt + (COccupancyController::SizeOfBitFieldItem * 8 - 1)) / COccupancyController::SizeOfBitFieldItem)* COccupancyController::SizeOfBitFieldItem)),
        [](uint32_t* p)->void {free(p); }),
    bitcnt(cnt),
    bufferOccupiedCount(0)
{
    this->ClearAll();
}

void COccupancyController::SetAllOccupiedStateChanged(std::function<void()> func)
{
    this->allOccupiedStateChanged = std::move(func);
}

bool COccupancyController::IsOccupied(std::uint32_t idx)
{
    return _bittest(this->GetBitField(), idx) != 0;
}

bool COccupancyController::IsFree(std::uint32_t idx)
{
    return _bittest(this->GetBitField(), idx) == 0;
}

void COccupancyController::ClearAll()
{
    memset(this->bitfield.get(), 0, this->GetCntOfBitfieldItems() * COccupancyController::SizeOfBitFieldItem);
}

void COccupancyController::SetOccupied(uint32_t idx)
{
    size_t offset = idx / (SizeOfBitFieldItem * 8);
    const bool bitbefore = _interlockedbittestandset(
        this->GetBitField() + offset, 
        static_cast<long>(idx % static_cast<uint32_t>(SizeOfBitFieldItem * 8)));
    if (bitbefore == true)
    {
        throw std::runtime_error("Bit was already set before.");
    }

    this->IncrementOccupiedCount();
}

void COccupancyController::SetFree(std::uint32_t idx)
{
    size_t offset = idx / (SizeOfBitFieldItem * 8);
    const bool bitbefore = _interlockedbittestandreset(
        this->GetBitField() + offset, 
        static_cast<long>(idx % static_cast<uint32_t>(SizeOfBitFieldItem * 8)));
    if (bitbefore == false)
    {
        throw std::runtime_error("Bit was already clear before.");
    }

    this->DecrementOccupiedCount();
}

void COccupancyController::IncrementOccupiedCount()
{
    const auto occupiedBefore = std::atomic_fetch_add(&this->bufferOccupiedCount, 1);
    if (occupiedBefore == this->bitcnt - 1)
    {
        // now all buffers are occupied
        if (this->allOccupiedStateChanged)
        {
            this->allOccupiedStateChanged();
        }
    }
}

void COccupancyController::DecrementOccupiedCount()
{
    const auto occupiedBefore = std::atomic_fetch_sub(&this->bufferOccupiedCount, 1);
    if (occupiedBefore == this->bitcnt)
    {
        // we just changed from "all buffers are occupied" to "at least one buffer is free"
        if (this->allOccupiedStateChanged)
        {
            this->allOccupiedStateChanged();
        }
    }
}

std::uint32_t COccupancyController::GetNumberOfBuffersFree()
{
    return atomic_load(&this->bufferOccupiedCount);
}

bool COccupancyController::GetAreAllBuffersOccupied()
{
    return this->GetNumberOfBuffersFree() == 0;
}

bool COccupancyController::GetIsFreeBufferAvailable()
{
    return this->GetNumberOfBuffersFree() > 0;
}