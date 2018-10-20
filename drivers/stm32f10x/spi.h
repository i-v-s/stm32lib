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
    struct Master { enum { cr1m = SPI_CR1_MSTR, cr1 = cr1m, cr2m = 0, cr2 = 0 }; };
    struct Slave  { enum { cr1m = SPI_CR1_MSTR, cr1 = 0, cr2m = 0, cr2 = 0 }; };
    template<int pol> struct ClockPolarity { static_assert(pol == 0 || pol == 1, "Wrong polarity"); enum { cr1m = SPI_CR1_CPOL, cr1 = pol ? cr1m : 0,         cr2m = 0, cr2 = 0 }; };
    template<int phase> struct ClockPhase { static_assert(phase == 1 || phase == 2, "Wrong phase"); enum { cr1m = SPI_CR1_CPHA, cr1 = (phase - 1) ? cr1m : 0, cr2m = 0, cr2 = 0 }; };
    template<bool en = true> struct Enable { enum { cr1m = SPI_CR1_SPE, cr1 = en ? cr1m : 0, cr2m = 0, cr2 = 0 }; };
    struct Disable { enum { cr1m = SPI_CR1_SPE, cr1 = 0, cr2m = 0, cr2 = 0 }; };
    struct ReceiveOnly { enum { cr1m = SPI_CR1_RXONLY, cr1 = cr1m, cr2m = 0, cr2 = 0 }; };
    template<bool ss = true> struct SlaveSelect { enum { cr1m = SPI_CR1_SSM | SPI_CR1_SSI, cr1 = SPI_CR1_SSM | (ss ? 0 : SPI_CR1_SSI), cr2m = 0, cr2 = 0 }; };
    //struct OnlyReceive { enum { flag = SPI_CR1_RXONLY, value = 1 }; };

#ifdef TEST_TEMPLATES
    static_assert(Options2<SlaveSelect<true>>::cr1m == (SPI_CR1_SSM | SPI_CR1_SSI), "Wrong mask!");
    static_assert(Options2<SlaveSelect<true>>::cr1  == SPI_CR1_SSM, "Wrong value!");
    static_assert(Options2<Enable<true>>::cr1m == SPI_CR1_SPE, "Wrong mask!");
    static_assert(Options2<Enable<true>>::cr1  == SPI_CR1_SPE, "Wrong value!");
    static_assert(Options2<Enable<true>, ClockPolarity<0>>::cr1m == (SPI_CR1_SPE | SPI_CR1_CPOL), "Wrong mask!");
    static_assert(Options2<Enable<true>, ClockPolarity<0>>::cr1  == SPI_CR1_SPE, "Wrong value!");
    static_assert(Options2<ClockPhase<2>, Enable<true>, ReceiveOnly>::cr1m == (SPI_CR1_CPHA | SPI_CR1_SPE | SPI_CR1_RXONLY), "Wrong mask!");
    static_assert(Options2<ClockPhase<2>, Enable<true>, ReceiveOnly>::cr1  == (SPI_CR1_CPHA | SPI_CR1_SPE | SPI_CR1_RXONLY), "Wrong value!");
    static_assert(Options2<Slave, ClockPolarity<0>, ClockPhase<2>, Enable<true>, ReceiveOnly>::cr1m == (SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SPE | SPI_CR1_RXONLY), "Wrong mask!");
    static_assert(Options2<Slave, ClockPolarity<0>, ClockPhase<2>, Enable<true>, ReceiveOnly>::cr1  == (                              SPI_CR1_CPHA | SPI_CR1_SPE | SPI_CR1_RXONLY), "Wrong value!");
    static_assert(Options2<Slave, ClockPolarity<0>, ClockPhase<2>, Enable<true>, ReceiveOnly>::cr2m == 0, "Wrong mask!");
#endif    
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
    template<typename... Args> static inline void configure()
    {
        Options2<Args...>::configure(p().CR1, p().CR2);
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
    inline static uint32_t blockingRead() { while(!(p().SR & SPI_SR_RXNE)); return p().DR; }
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