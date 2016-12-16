#ifndef CLOCK_H
#define CLOCK_H
#include <cstdint>
#include <stm32f1xx.h>

enum ClockSource
{
    CS_None = 0,
    CS_SysClk = 4,
    CS_HSI = 5,
    CS_HSE = 6,
    CS_PLLdiv2 = 7,
    CS_PLL,
    CS_HCLK,
    CS_PCLK1,
    CS_PCLK2
};

namespace Clock {

///////// enable /////////////////
template<ClockSource cs> void enable();
template<> void inline enable<CS_HSE>()
{
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)); // Включаем HSE, если не включено
}
template<> void inline enable<CS_HSI>()
{
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)); // Включаем HSI, если не включено
}
//////// setClockSource
template<ClockSource cs> void setClockSource();

template<> void setClockSource<CS_PLL>()
{
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
template<> void setClockSource<CS_HSE>()
{
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSE;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE);
}
template<> void setClockSource<CS_HSI>()
{
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
}

template<ClockSource cs, uint32_t in, uint32_t out>
inline void configurePLL()
{
    RCC->CR &= ~RCC_CR_PLLON; // Выключаем PLL
    static_assert(cs == CS_HSI || cs == CS_HSE, "Wrong PLL source");
    constexpr uint32_t pllmul = out / in;
    static_assert(pllmul >= 2, "PLL mul too low");
    static_assert(pllmul <= 16, "PLL mul too high");
    static_assert(pllmul * in == out, "Incorrect frequency");
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_PLLMULL | RCC_CFGR_PLLSRC)) // HSI => PLL
            | ((pllmul - 2) << RCC_CFGR_PLLMULL_Pos) // Устанавливаем множитель PLL
            | ((cs == CS_HSE) ? RCC_CFGR_PLLSRC : 0); // Устанавливаем источник PLL
    RCC->CR |= RCC_CR_PLLON; // Включаем PLL
    while (!(RCC->CR & RCC_CR_PLLRDY));
}

template<ClockSource cs, uint32_t pre> constexpr uint32_t busPre();
template<> inline constexpr uint32_t busPre<CS_HCLK, 1>() { return RCC_CFGR_HPRE_DIV1; }
template<> inline constexpr uint32_t busPre<CS_HCLK, 2>() { return RCC_CFGR_HPRE_DIV2; }
template<> inline constexpr uint32_t busPre<CS_HCLK, 4>() { return RCC_CFGR_HPRE_DIV4; }
template<> inline constexpr uint32_t busPre<CS_HCLK, 8>() { return RCC_CFGR_HPRE_DIV8; }
template<> inline constexpr uint32_t busPre<CS_HCLK, 16>() { return RCC_CFGR_HPRE_DIV16; }
template<> inline constexpr uint32_t busPre<CS_HCLK, 64>() { return RCC_CFGR_HPRE_DIV64; }
template<> inline constexpr uint32_t busPre<CS_HCLK, 128>() { return RCC_CFGR_HPRE_DIV128; }
template<> inline constexpr uint32_t busPre<CS_HCLK, 256>() { return RCC_CFGR_HPRE_DIV256; }
template<> inline constexpr uint32_t busPre<CS_HCLK, 512>() { return RCC_CFGR_HPRE_DIV512; }

template<> inline constexpr uint32_t busPre<CS_PCLK1, 1>() { return RCC_CFGR_PPRE1_DIV1; }
template<> inline constexpr uint32_t busPre<CS_PCLK1, 2>() { return RCC_CFGR_PPRE1_DIV2; }
template<> inline constexpr uint32_t busPre<CS_PCLK1, 4>() { return RCC_CFGR_PPRE1_DIV4; }
template<> inline constexpr uint32_t busPre<CS_PCLK1, 8>() { return RCC_CFGR_PPRE1_DIV8; }
template<> inline constexpr uint32_t busPre<CS_PCLK1, 16>() { return RCC_CFGR_PPRE1_DIV16; }

template<> inline constexpr uint32_t busPre<CS_PCLK2, 1>() { return RCC_CFGR_PPRE2_DIV1; }
template<> inline constexpr uint32_t busPre<CS_PCLK2, 2>() { return RCC_CFGR_PPRE2_DIV2; }
template<> inline constexpr uint32_t busPre<CS_PCLK2, 4>() { return RCC_CFGR_PPRE2_DIV4; }
template<> inline constexpr uint32_t busPre<CS_PCLK2, 8>() { return RCC_CFGR_PPRE2_DIV8; }
template<> inline constexpr uint32_t busPre<CS_PCLK2, 16>() { return RCC_CFGR_PPRE2_DIV16; }

template<uint32_t freq, uint32_t hclk, uint32_t pclk1, uint32_t pclk2> inline void setBusPre()
{
    static_assert(hclk <= 72000000, "HCLK frequency too high");
    static_assert(pclk1 <= 36000000, "PCLK1 frequency too high");
    static_assert(pclk2 <= 72000000, "PCLK2 frequency too high");
    constexpr uint32_t hpre = freq / hclk, ppre1 = hclk / pclk1, ppre2 = hclk / pclk2;
    static_assert(hpre * hclk == freq, "Wrong HCLK frequency");
    static_assert(ppre1 * pclk1 == hclk, "Wrong PCLK1 frequency");
    static_assert(ppre2 * pclk2 == hclk, "Wrong PCLK2 frequency");
    constexpr uint32_t pres = busPre<CS_HCLK, hpre>()
                            | busPre<CS_PCLK1, ppre1>()
                            | busPre<CS_PCLK2, ppre2>();
    RCC->CFGR = pres | (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2));
}

template<uint32_t freq> inline void setFlashLatency()
{
    static_assert(freq <= 72000000, "Too high frequency");
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | ((freq - 1) / 24000000);
}

}

template<uint32_t freq = 64000000,
         uint32_t HCLK = freq,
         uint32_t PCLK1 = HCLK,
         uint32_t PCLK2 = HCLK,
         uint32_t HSE = 0>
class SystemClock
{
public:
    enum {
        sysClock = freq,
        hclk =  HCLK,
        pclk1 = PCLK1,
        pclk2 = PCLK2
    };
    template<ClockSource cs> static inline void enableMCO()
    {
        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_MCO) | (cs << RCC_CFGR_MCO_Pos);
    }
};

template<uint32_t freq = 64000000,
         uint32_t HCLK = freq,
         uint32_t PCLK1 = HCLK,
         uint32_t PCLK2 = HCLK>
class SystemClockHSI : public SystemClock<freq, HCLK, PCLK1, 0>
{
public:
    void static init()
    {
        static_assert(freq <= 64000000, "Too high frequency");
        using namespace Clock;
        enable<CS_HSI>();
        if (freq != 8000000) {
            setClockSource<CS_HSI>();
            configurePLL<CS_HSI, 4000000, freq>();
            setFlashLatency<freq>();
            setClockSource<CS_PLL>();
        }
        else setClockSource<CS_HSI>();
    }
};

template<uint32_t freq = 72000000,
         uint32_t HCLK = freq,
         uint32_t PCLK1 = HCLK,
         uint32_t PCLK2 = HCLK,
         uint32_t HSE = 8000000>
class SystemClockHSE : public SystemClock<freq, HCLK, PCLK1, HSE>
{
public:
    void static init()
    {
        static_assert(freq <= 72000000, "Too high frequency");
        using namespace Clock;
        enable<CS_HSE>();
        if (freq != HSE) {
            configurePLL<CS_HSE, HSE, freq>();
            setBusPre<freq, HCLK, PCLK1, PCLK2>();
            setFlashLatency<freq>();
            setClockSource<CS_PLL>();
        } else
            setClockSource<CS_HSE>();
    }
};

template<uint32_t freq = 72000000,
         uint32_t HCLK = freq,
         uint32_t PCLK1 = HCLK / 2,
         uint32_t PCLK2 = HCLK>
using SystemClockHSE8 = SystemClockHSE<freq, HCLK, PCLK1, PCLK2, 8000000>;

template<class Clock>
inline void setSysTick(float period)
{
    //if (Clock::hclk)
    //static_assert(period > 0 && period * (Clock::hclk / 8) <= 0xFFFFFF, "Wrong period");
    SysTick->LOAD = uint32_t(period * (Clock::hclk / 8));
    SysTick->CTRL = SysTick_CTRL_ENABLE;
}

inline bool sysTickFlag()
{
    return SysTick->CTRL & SysTick_CTRL_COUNTFLAG;
}

#endif // CLOCK_H
