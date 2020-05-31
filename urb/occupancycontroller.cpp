#include "pch.h"
#include "occupancycontroller.h"
#include <intrin.h>

using namespace std;

COccupancyController::COccupancyController(uint32_t cnt) :
    bitfield(static_cast<uint32_t*>(malloc(((cnt + 31) / 32) * 4)), [](uint32_t* p)->void {free(p); }),
    bitcnt(cnt)
{
    this->ClearAll();
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
    memset(this->bitfield.get(), 0, this->GetCntOfBitfieldItems() * 4);
}

void COccupancyController::SetOccupied(uint32_t idx)
{
    _interlockedbittestandset(this->GetBitField(), idx);
}
void COccupancyController::SetFree(std::uint32_t idx)
{
    _interlockedbittestandreset(this->GetBitField(), idx);
}
