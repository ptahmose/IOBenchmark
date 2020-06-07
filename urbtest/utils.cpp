#include "utils.h"
#include <cinttypes>

void FillWithAbc(void* ptr, size_t size)
{
    std::uint8_t* p = static_cast<std::uint8_t*>(ptr);
    char c = 'a';
    for (size_t i = 0; i < size; ++i)
    {
        *p = c;
        if (c == 'z')
        {
            c = 'a';
        }

        ++c;
        ++p;
    }
}