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
    CS_PCLK2,
    CS_PCLK1_TIM,
    CS_PCLK2_TIM
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

template<uint32_t HSE = 8000000>
class SystemClocksCurrent
{
private:
    uint32_t sysClk, pllClk, hClk, pClk1, pClk2, pClk1Tim, pClk2Tim;
    static inline uint32_t getPLLSrcFreq(uint32_t cf)
    {
        if (cf & RCC_CFGR_PLLSRC) // HSE
            return (cf & RCC_CFGR_PLLXTPRE) ? HSE / 2 : HSE;
        else // HSI
            return 8000000 / 2;
    }
    static inline uint32_t getPLLMul(uint32_t cf)
    {
        uint32_t m = (cf >> RCC_CFGR_PLLMULL_Pos) & 15;
        return (m == 15) ? 16 : (m + 2);
    }
    static inline uint32_t getHPre(uint32_t cf)
    {
        switch(cf & RCC_CFGR_HPRE) {
        default: return 1;
        case 8: return 2;
        case 9: return 4;
        case 10: return 8;
        case 11: return 16;
        case 12: return 64;
        case 13: return 128;
        case 14: return 256;
        case 15: return 512;
        }
    }
    template<int bus>
    static inline uint32_t getPPre(uint32_t cf)
    {
        static_assert(bus == 1 || bus == 2, "Wrong bus number");
        switch((cf >> ((bus == 1) ? RCC_CFGR_PPRE1_Pos : RCC_CFGR_PPRE2_Pos)) & 7) {
        default: return 1;
        case 4: return 2;
        case 5: return 4;
        case 6: return 8;
        case 7: return 16;
        }
    }
public:
    SystemClocksCurrent() {}
    void update()
    {
        uint32_t cf = RCC->CFGR;
        pllClk = getPLLSrcFreq(cf) * getPLLMul(cf);
        switch(cf & RCC_CFGR_SWS_Msk) {
        case 0:              sysClk = 8000000; break; // HSI
        case RCC_CFGR_SWS_0: sysClk = HSE;     break;
        case RCC_CFGR_SWS_1: sysClk = pllClk;  break;
        }
        hClk = sysClk / getHPre(cf);
        uint32_t pre1 = getPPre<1>(cf), pre2 = getPPre<2>(cf);
        pClk1 = hClk / pre1;
        pClk1Tim = (pre1 == 1) ? pClk1 : (pClk1 * 2);
        pClk2 = hClk / pre2;
        pClk2Tim = (pre2 == 1) ? pClk2 : (pClk2 * 2);      
    }
    template<ClockSource cs> inline uint32_t getFrequency() const
    {
        switch(cs) {
        default: return 0;
        case CS_SysClk: return sysClk;
        case CS_HSI: return 8000000;
        case CS_HSE: return HSE;
        case CS_PLLdiv2: return pllClk / 2;
        case CS_PLL: return pllClk;
        case CS_HCLK: return hClk;
        case CS_PCLK1: return pClk1;
        case CS_PCLK2: return pClk2;
        case CS_PCLK1_TIM: return pClk1Tim;
        case CS_PCLK2_TIM: return pClk2Tim;
        }
    }
};

template<typename... Devices> struct DevList;
template<> struct DevList<> { enum { ahb = 0, apb1 = 0, apb2 = 0 }; };
template<typename Dev, typename... Devs> struct DevList<Dev, Devs...>
{
    typedef DevList<Devs...> Next;
    enum {
        ahb = Next::ahb | Dev::ahb(),
        apb1 = Next::apb1 | Dev::apb1(),
        apb2 = Next::apb2 | Dev::apb2()
    };
};


template<typename... Devices> void enableDevices()
{
    typedef DevList<Devices...> L;
    static_assert(L::ahb | L::apb1 | L::apb2, "Nothing to enable!");
    if (L::ahb) RCC->AHBENR |= L::ahb;
    if (L::apb1) RCC->APB1ENR |= L::apb1;
    if (L::apb2) RCC->APB2ENR |= L::apb2;
}

template<typename... Devices> void disableDevices()
{
    typedef DevList<Devices...> L;
    static_assert(L::ahb | L::apb1 | L::apb2, "Nothing to disable!");
    if (L::ahb) RCC->AHBENR &= ~L::ahb;
    if (L::apb1) RCC->APB1ENR &= ~L::apb1;
    if (L::apb2) RCC->APB2ENR &= ~L::apb2;
}


template<uintptr_t device>
ClockSource getClockSource();

template<class Clock>
inline void setSysTick(float period)
{
    //if (Clock::hclk)
    //static_assert(period > 0 && period * (Clock::hclk / 8) <= 0xFFFFFF, "Wrong period");
    SysTick->LOAD = uint32_t(period * (Clock::hclk / 8));
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;
}

inline bool sysTickFlag()
{
    return SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
}

#endif // CLOCK_H
