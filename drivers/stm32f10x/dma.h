#ifndef DMA_H
#define DMA_H

#include <stm32f1xx.h>
#include "utils.h"


template<typename Src, typename Dst, typename... Args> struct Link;

namespace dma_ {    
    template<typename ccr, typename count = NoValue, typename cpar = NoValue, typename cmar = NoValue> struct ChannelCfg 
    { 
        typedef ccr CCR; 
        static inline void apply(DMA_Channel_TypeDef &channel) {
            ccr  ::apply(channel.CCR);
            count::apply(channel.CNDTR);
            cpar ::apply(channel.CPAR);
            cmar ::apply(channel.CMAR);
        }
    };
    template<uintptr_t dma_, int ch> struct ChannelId { enum { dma = dma_, channel = ch}; };
    template<bool en = true> using Enable = ChannelCfg<Bits<uint32_t, DMA_CCR_EN, en ? DMA_CCR_EN : 0>>;
}

template<typename ccr1, typename ccr2, typename c1, typename c2, typename p1, typename p2, typename m1, typename m2>
    struct Merge<dma_::ChannelCfg<ccr1, c1, p1, m1>, dma_::ChannelCfg<ccr2, c2, p2, m2>> {
        typedef dma_::ChannelCfg<
            typename Merge<ccr1, ccr2>::Result, 
            typename Merge<c1, c2>::Result,
            typename Merge<p1, p2>::Result, 
            typename Merge<m1, m2>::Result
        > Result;
    };


template<uintptr_t device> struct GetDmaRx;
template<uintptr_t device> struct GetDmaTx;

template<typename T, T *ptr_, size_t count_> struct Memory 
{   
    inline constexpr T *ptr() { return ptr_; }
    enum { 
        itemSize = sizeof(T), 
        count = count_,
        mask = DMA_CCR_MSIZE | DMA_CCR_MINC | DMA_CCR_DIR, // Default: Memory size, increment, direction
        msize = (itemSize == 4) ? 2 : ((itemSize == 2) ? 1 : 0),
        value = (msize << DMA_CCR_MSIZE_Pos) | DMA_CCR_MINC
    };
    static_assert(itemSize == 1 || itemSize == 2 || itemSize == 4, "Wrong size of type!");
    using Source = dma_::ChannelCfg<
        Bits<uint32_t, mask, value | DMA_CCR_DIR>,
        Value<uint32_t, count>,
        NoValue,
        Value<uintptr_t, (uintptr_t)ptr_>>;
    using Destination = dma_::ChannelCfg<
        Bits<uint32_t, mask, value>,
        Value<uint32_t, count>,
        NoValue,
        Value<uintptr_t, (uintptr_t)ptr_>>;
};

template<typename T1, T1 *ptr1, size_t count1, typename T2, T2 *ptr2, size_t count2, typename... Args>
struct Link<Memory<T1, ptr1, count1>, Memory<T2, ptr2, count2>, Args...>
{
    using A = Reduce<Args...>;
    static_assert(count1 == count2, "Memory count mismatch");
};


template<typename Link> void configureDMA()
{
}

template<int chan, int num = 1> DMA_Channel_TypeDef * di();
template<> inline constexpr DMA_Channel_TypeDef * di<1, 1>() {return DMA1_Channel1; }
template<> inline constexpr DMA_Channel_TypeDef * di<2, 1>() {return DMA1_Channel2; }
template<> inline constexpr DMA_Channel_TypeDef * di<3, 1>() {return DMA1_Channel3; }
template<> inline constexpr DMA_Channel_TypeDef * di<4, 1>() {return DMA1_Channel4; }
template<> inline constexpr DMA_Channel_TypeDef * di<5, 1>() {return DMA1_Channel5; }
template<> inline constexpr DMA_Channel_TypeDef * di<6, 1>() {return DMA1_Channel6; }
template<> inline constexpr DMA_Channel_TypeDef * di<7, 1>() {return DMA1_Channel7; }

template<int chan, int num = 1>
class DMAChannel
{
public:
    inline static void setBuffer()
    {

    }
};
/*
configureDMA<
    Link<Spi, Memory<>>,
    Link<Memory<>, Memory<>>
>();
*/



#endif // DMA_H
