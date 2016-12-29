#ifndef DMA_H
#define DMA_H

#include <stm32f1xx.h>

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
    inline static setBuffer()
    {

    }
};

#endif // DMA_H
