#ifndef SPI_H
#define SPI_H
#include <cstdint>
#include <stm32f1xx.h>
#include "utils.h"
#include "dma.h"

// APB2 SPI
template<> inline constexpr ClockSource getClockSource<(uintptr_t)SPI1>() { return CS_PCLK2; };

// APB1 SPI
#ifdef SPI2
template<> inline constexpr ClockSource getClockSource<(uintptr_t)SPI2>() { return CS_PCLK1; };
#endif
#ifdef SPI3
template<> inline constexpr ClockSource getClockSource<(uintptr_t)SPI3>() { return CS_PCLK1; };
#endif

// SPI DMA channels
template<> struct GetDmaRx<uintptr_t(SPI1)> { using Result = dma_::ChannelId<uintptr_t(DMA1), 2>; };
template<> struct GetDmaTx<uintptr_t(SPI1)> { using Result = dma_::ChannelId<uintptr_t(DMA1), 3>; };

namespace spi_ {
// SPI options
    template<uintptr_t spi> struct SpiDmaApply
    {
        template<typename DmaCfg> static inline void apply()
        {
            switch(spi) {
            case uintptr_t(SPI1):
                if (DmaCfg::CCR::v & DMA_CCR_DIR) {
                    DmaCfg::apply(*DMA1_Channel3);
                } else {
                    DmaCfg::apply(*DMA1_Channel2);
                }
                break;
            }
        }
    };
    
    template<typename cr1, typename cr2 = NoValue, typename dma = List<>> struct SpiCfg { 
        typedef cr1 CR1; typedef cr2 CR2;
        static inline void apply(SPI_TypeDef &spi) {
            CR1::apply(spi.CR1);
            CR2::apply(spi.CR2);
            switch(uintptr_t(&spi)) {
            case uintptr_t(SPI1):
                Iterate<SpiDmaApply<uintptr_t(SPI1)>, dma>::apply();
                break;
            }
        }
    };

    template<bool en = true> 
        using Enable        = SpiCfg<Bits32<SPI_CR1_SPE, en ? SPI_CR1_SPE : 0>>;
    using Disable           = SpiCfg<Bits32<SPI_CR1_SPE, 0>>;
    using Master            = SpiCfg<Bits32<SPI_CR1_MSTR,     SPI_CR1_MSTR>>;
    using ReceiveOnly       = SpiCfg<Bits32<SPI_CR1_RXONLY,   SPI_CR1_RXONLY>>;
    template<int pol> 
        using ClockPolarity = SpiCfg<Bits32<SPI_CR1_CPOL, toBool<pol>() ? SPI_CR1_CPOL : 0>>;
    template<int phase> 
        using ClockPhase    = SpiCfg<Bits32<SPI_CR1_CPHA, toBool<phase, 1> ? SPI_CR1_CPHA : 0>>;

    // Slave modes:
    template<bool = true> struct InternalSelect;
    struct ExternalSelect;
    
    template<typename Select> struct Slave_;
    template<typename Select = None> using Slave = typename Slave_<Select>::Result;
    template<> struct Slave_<None>                      { using Result = SpiCfg<Bits32<SPI_CR1_MSTR, 0>>; };
    template<> struct Slave_<ExternalSelect>            { using Result = SpiCfg<Bits32<SPI_CR1_MSTR | SPI_CR1_SSM, 0>>; };
    template<bool ss> struct Slave_<InternalSelect<ss>> { using Result = SpiCfg<Bits32<SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI, SPI_CR1_SSM | (ss ? 0 : SPI_CR1_SSI)>>; };

    template<typename T, uintptr_t ptr_, uint32_t count_> using MemDmaCfg = dma_::ChannelCfg<
        Bits<uint32_t, DMA_CCR_MINC, DMA_CCR_MINC>,
        Value<uint32_t, (uint32_t)count_>,
        NoValue,
        Value<uint32_t, uint32_t(ptr_)>
    >;
    
    template</*uintptr_t spi,*/ bool dir> using SpiDmaCfg = dma_::ChannelCfg<
        Bits<uint32_t, DMA_CCR_MEM2MEM | DMA_CCR_PINC | DMA_CCR_DIR, dir ? DMA_CCR_DIR : 0>, // mem2mem:false, pinc:false, dir: read from peripheral
        NoValue,
        NoValue, //Value<uintptr_t, (uintptr_t)&((SPI_TypeDef *)spi)->DR>,
        NoValue>;    
    
    template<typename T, uintptr_t ptr_, size_t count_, typename... Args> 
        using ToMemory = SpiCfg<
            NoValue, // CR1
            Bits32<SPI_CR2_RXDMAEN, SPI_CR2_RXDMAEN>, // CR2
            List<Reduce<
                MemDmaCfg<T, ptr_, count_>,
                SpiDmaCfg<false>,
                Args...
            >>
        >;
}

template<typename cr1A, typename cr2A, typename... DmaA, typename cr1B, typename cr2B, typename... DmaB>
    struct Merge<spi_::SpiCfg<cr1A, cr2A, List<DmaA...>>, spi_::SpiCfg<cr1B, cr2B, List<DmaB...>>> {
        typedef spi_::SpiCfg<
            typename Merge<cr1A, cr1B>::Result,
            typename Merge<cr2A, cr2B>::Result,
            List<DmaA..., DmaB...>
        > Result;
    };

#ifdef TEST_TEMPLATES
namespace spi_{        
    //static_assert(spi_::SpiDmaCfg<uintptr_t(SPI1), true>::CCR::m == (DMA_CCR_MEM2MEM | DMA_CCR_PINC | DMA_CCR_DIR), "Wrong mask!");
    //static_assert(spi_::SpiDmaCfg<uintptr_t(SPI1), true>::CCR::v == (DMA_CCR_DIR), "Wrong value!");

    static_assert(Slave<InternalSelect<true>>::CR1::m == (SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI), "Wrong mask!");
    static_assert(Slave<InternalSelect<true>>::CR1::v ==                 SPI_CR1_SSM,                "Wrong value!");
    static_assert(Reduce<Enable<true>>::CR1::m == SPI_CR1_SPE, "Wrong mask!");
    static_assert(Reduce<Enable<true>>::CR1::v == SPI_CR1_SPE, "Wrong value!");
    static_assert(Merge<Enable<true>, ClockPolarity<0>>::Result::CR1::m == (SPI_CR1_SPE | SPI_CR1_CPOL), "Wrong mask!");
    static_assert(Merge<Enable<true>, ClockPolarity<0>>::Result::CR1::v  == SPI_CR1_SPE, "Wrong value!");
    static_assert(Reduce<Enable<true>, ClockPolarity<0>>::CR1::m == (SPI_CR1_SPE | SPI_CR1_CPOL), "Wrong mask!");
    static_assert(Reduce<Enable<true>, ClockPolarity<0>>::CR1::v  == SPI_CR1_SPE, "Wrong value!");
    static_assert(Reduce<ClockPhase<2>, Enable<true>, ReceiveOnly>::CR1::m == (SPI_CR1_CPHA | SPI_CR1_SPE | SPI_CR1_RXONLY), "Wrong mask!");
    static_assert(Reduce<ClockPhase<2>, Enable<true>, ReceiveOnly>::CR1::v == (SPI_CR1_CPHA | SPI_CR1_SPE | SPI_CR1_RXONLY), "Wrong value!");
    static_assert(Reduce<Slave<>, ClockPolarity<0>, ClockPhase<2>, Enable<true>, ReceiveOnly>::CR1::m == (SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SPE | SPI_CR1_RXONLY), "Wrong mask!");
    static_assert(Reduce<Slave<>, ClockPolarity<0>, ClockPhase<2>, Enable<true>, ReceiveOnly>::CR1::v == (                              SPI_CR1_CPHA | SPI_CR1_SPE | SPI_CR1_RXONLY), "Wrong value!");
    static_assert(Reduce<Slave<>, ClockPolarity<0>, ClockPhase<2>, Enable<true>, ReceiveOnly>::CR2::m == 0, "Wrong mask!");
}
#endif    


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
        Reduce<Args...>::apply(p());
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

/*template<uintptr_t spi, typename Clock, const Clock *clock, typename Memory, typename... Args>
struct Link<Spi<spi, Clock, clock>, Memory, Args...>
{
    using DmaId = typename GetDmaRx<spi>::Result;
    using DmaCfg = Reduce<spi_::SpiDmaCfg<spi, false>, typename Memory::Destination, Args...>;
};

template<uintptr_t spi, typename Clock, const Clock *clock, typename Memory, typename... Args>
struct Link<Memory, Spi<spi, Clock, clock>, Args...>
{
    using DmaId = typename GetDmaRx<spi>::Result;
    using DmaCfg = Reduce<typename Memory::Source, spi_::SpiDmaCfg<spi, true>, Args...>;
};*/

#ifdef TEST_TEMPLATES
//{
    //using TEST_Link1 = Link<Spi<uintptr_t(SPI1), int, (int*)0>, Memory<uint8_t, (uint8_t*)0, 3>, dma_::Enable<true>>;
    //static_assert(TEST_Link1::DmaId::dma == uintptr_t(DMA1) && TEST_Link1::DmaId::channel == 2, "Wrong DMA channel!");
    //static_assert(TEST_Link1::DmaCfg::CCR::m == (DMA_CCR_MEM2MEM | DMA_CCR_PINC | DMA_CCR_DIR | DMA_CCR_EN | DMA_CCR_MSIZE | DMA_CCR_MINC), "Wrong DMA CCR mask!");
    
//}
#endif

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