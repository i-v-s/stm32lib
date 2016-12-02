#ifndef TIMER_H
#define TIMER_H

#include <stm32f1xx.h>

enum ETimer
{
    Timer1 = (uintptr_t)TIM1,
    Timer2 = (uintptr_t)TIM2,
    Timer3 = (uintptr_t)TIM3,
    Timer4 = (uintptr_t)TIM4,
    Timer6 = (uintptr_t)TIM6,
    Timer7 = (uintptr_t)TIM7
};

template<ETimer etim>
struct Timer
{
    __IO uint32_t *CCR;
    static inline constexpr TIM_TypeDef &p() { return *(TIM_TypeDef *) etim; }
    static inline constexpr volatile uint32_t *ccr() { return &p().CCR1; }
    template<unsigned int mask>
    static void setPwmMode()
    {
        static_assert(mask < 0x10, "Wrong channel mask");
        constexpr uint32_t mode = 6 << TIM_CCMR1_OC1M_Pos;
        p().CCMR1 = ((mask & 1) ? mode : 0) | ((mask & 2) ? mode << 8 : 0);
        p().CCMR2 = ((mask & 4) ? mode : 0) | ((mask & 8) ? mode << 8 : 0);
        constexpr uint32_t er = TIM_CCER_CC1E;
        p().CCER = ((mask&1)?er:0) | ((mask&2)?er<<4:0) | ((mask&4)?er<<8:0) | ((mask&8)?er<<12:0);
    }

    void enable(uint32_t top)
    {
        p().ARR = top;
        p().CR2 = 0;
        p().CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
    }
};

#endif // TIMER_H
