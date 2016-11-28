#ifndef CLOCK_H
#define CLOCK_H
#include <cstdint>

template<uint32_t freq = 24000000, uint32_t hse = 0>
void initSystemClock()
{
    static_assert(freq <= 24000000, "Too high frequency");
}



#endif // CLOCK_H
