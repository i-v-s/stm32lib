#ifndef SPI_H
#define SPI_H
#include <cstdint>
#include <stm32f1xx.h>
#include "utils.h"

// APB2 SPI
template<> inline constexpr ClockSource getClockSource<(uintptr_t)SPI1>() { return CS_PCLK2; };

// APB1 SPI
#ifdef SPI2
template<> inline constexpr ClockSource getClockSource<(uintptr_t)SPI2>() { return CS_PCLK1; };
#endif
#ifdef SPI3
template<> inline constexpr ClockSource getClockSource<(uintptr_t)SPI3>() { return CS_PCLK1; };
#endif

namespace spi_ {
// SPI options
    struct Master { enum { flag = SPI_CR1_MSTR, value = SPI_CR1_MSTR }; };
    struct Slave  { enum { flag = SPI_CR1_MSTR, value = 0 }; };
    template<int pol> struct ClockPolarity { enum { flag = SPI_CR1_CPOL, value = pol ? SPI_CR1_CPOL : 0 }; };
    template<int phase> struct ClockPhase { enum { flag = SPI_CR1_CPHA, value = phase - 1 }; };
    template<bool en = true> struct Enable { enum { flag = SPI_CR1_SPE, value = en ? SPI_CR1_SPE : 0 }; };
    struct Disable { enum { flag = SPI_CR1_SPE, value = 0 }; };
    struct ReceiveOnly { enum { flag = SPI_CR1_RXONLY, value = SPI_CR1_RXONLY }; };
    //struct OnlyReceive { enum { flag = SPI_CR1_RXONLY, value = 1 }; };
}

template<uintptr_t spi, typename Clock, const Clock *clock = nullptr>
class Spi
{
    static inline constexpr SPI_TypeDef &p() { return *(SPI_TypeDef *) spi; }
public:
    template<uint32_t rate> static inline constexpr uint32_t boudrate2cr1()
    {
        uint32_t src = clock->template getFrequency<getClockSource<spi>()>();
        switch(src / rate) {
        default:
        case 2: return 0;
        case 4: return 1;
        case 8: return 2;
        case 16: return 3;
        case 32: return 4;
        case 64: return 5;
        case 128: return 6;
        case 256: return 7;
        }
    }
    template<typename... Args> void setOptions()
    {
      typedef Options<Args...> O;
      p().CR1 = (p().CR1 & ~O::mask) | O::value;
    }
    
    template<uint32_t rate> inline void setBoudrate() 
    {
        p().CR1 = p().CR1 & ~SPI_CR1_BR_Msk | boudrate2cr1<rate>();
    }
    inline void enable() { p().CR1 |= SPI_CR1_SPE; }
};

#endif