#ifndef TIMER_H
#define TIMER_H

#include <stm32f1xx.h>

enum ETimer
{
    Timer1 = (uintptr_t)TIM1,
    Timer2 = (uintptr_t)TIM2,
    Timer3 = (uintptr_t)TIM3,
    Timer4 = (uintptr_t)TIM4,
#ifdef TIM6
    Timer6 = (uintptr_t)TIM6,
#endif
#ifdef TIM7
    Timer7 = (uintptr_t)TIM7
#endif
};

enum ETrigger
{
    ITR0 = 0,
    ITR1 = 1,
    ITR2 = 2,
    ITR3 = 3,
    TI1FP = 5
};

//enum Mode

/*namespace smcr {
    template<ETrigger trg, bool inv = false>
    inline constexpr uint32_t externalTrigger() { return TIM_SMCR_ECE | (inv ? TIM_SMCR_ETP : 0) | ;


}*/

template<ETimer etim>
struct Timer
{
    __IO uint32_t *CCR;
    static inline constexpr TIM_TypeDef &p() { return *(TIM_TypeDef *) etim; }
    //static inline constexpr volatile uint32_t *ccr() { return &p().CCR1; }
    template<int c> inline void ccr(uint32_t v)
    {
        switch(c) {
        case 0: p().CCR1 = v;
        case 1: p().CCR2 = v;
        case 2: p().CCR3 = v;
        case 3: p().CCR4 = v;
        }
    }
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
    template<bool inv = false>
    static inline void externalMode2()
    {
        p().SMCR = (p().SMCR & ~(TIM_SMCR_ETF | TIM_SMCR_ETPS)) | TIM_SMCR_ECE | (inv ? TIM_SMCR_ETP : 0);
    }
    template<uint32_t trigger = 6> // Filtered Timer Input 2 (TI2FP2)
    static inline void externalMode1()
    {
        p().CCMR1 = (1 << TIM_CCMR1_CC2S_Pos); // CC2 channel is configured as input, IC2 is mapped on TI2

        //(1 << TIM_CCMR1_CC1S_Pos);
        p().CCER = 0;

        p().SMCR = (p().SMCR & ~(TIM_SMCR_TS | TIM_SMCR_SMS))
                | (trigger << TIM_SMCR_TS_Pos)
                | (4 << TIM_SMCR_SMS_Pos); // Reset Mode - Rising edge of the selected trigger input (TRGI) reinitializes the counter
    }
    static inline void enableDMA()
    {
        p().DIER |= TIM_DIER_TDE;
        p().DIER |= TIM_DIER_TIE;
    }

    //template<bool onePulse = false>
    void enable(uint32_t top)
    {
        p().ARR = top;
        p().CR2 = 0;
        p().CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
    }
};

#endif // TIMER_H
