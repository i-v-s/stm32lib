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
    template<typename... Args> static inline void setOptions()
    {
        typedef Options<Args...> O;
        p().CR1 = (p().CR1 & ~O::mask) | O::value;
    }
    
    template<uint32_t rate> static inline void setBoudrate() 
    {
        p().CR1 = p().CR1 & ~SPI_CR1_BR_Msk | boudrate2cr1<rate>();
    }
    inline void enable() { p().CR1 |= SPI_CR1_SPE; }
    inline static constexpr uint32_t ahb() { return 0; }
    inline static constexpr uint32_t apb1() { 
        switch(spi) {
        default: return 0;
        case uintptr_t(SPI2): return RCC_APB1ENR_SPI2EN;
#ifdef SPI3
        case uintptr_t(SPI3): return RCC_APB1ENR_SPI3EN;
#endif
        }
    }
    inline static constexpr uint32_t apb2() { return (spi == uintptr_t(SPI1)) ? RCC_APB2ENR_SPI1EN : 0; }
    static_assert(apb1() ^ apb2(), "No bus specified for spi!");
};

template<typename Device> struct MISO {};
template<typename Device> struct MOSI {};
template<typename Device> struct SCK  {};
template<typename Device> struct NSS  {};


#define SPI_REMAP(spi, func, port, pin, mask, val) template<typename Clock, const Clock *clock> \
struct Remap<func<Spi<uintptr_t(spi), Clock, clock>>, Pin<uintptr_t(port), pin>> \
    { enum { maprMsk = mask, maprVal = val ? mask : 0 }; }

SPI_REMAP(SPI1, NSS,  GPIOA, 4,  AFIO_MAPR_SPI1_REMAP, 0);
SPI_REMAP(SPI1, SCK,  GPIOA, 5,  AFIO_MAPR_SPI1_REMAP, 0);
SPI_REMAP(SPI1, MISO, GPIOA, 6,  AFIO_MAPR_SPI1_REMAP, 0);
SPI_REMAP(SPI1, MOSI, GPIOA, 7,  AFIO_MAPR_SPI1_REMAP, 0);

SPI_REMAP(SPI1, NSS,  GPIOA, 15, AFIO_MAPR_SPI1_REMAP, 1);
SPI_REMAP(SPI1, SCK,  GPIOB, 3,  AFIO_MAPR_SPI1_REMAP, 1);
SPI_REMAP(SPI1, MISO, GPIOB, 4,  AFIO_MAPR_SPI1_REMAP, 1);
SPI_REMAP(SPI1, MOSI, GPIOB, 5,  AFIO_MAPR_SPI1_REMAP, 1);

#endif