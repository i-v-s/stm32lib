#ifndef CLOCK_H
#define CLOCK_H
#include <cstdint>
#include <stm32f1xx.h>

template<uint32_t freq = 24000000,
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
    void static init()
    {
        static_assert(freq <= 24000000, "Too high frequency");
        if (!HSE) {
            while (!(RCC->CR & RCC_CR_HSIRDY)) RCC->CR |= RCC_CR_HSION; // Включаем HSI, если не включено
            if (freq != 8000000) {
                static_assert(freq >= 16000000, "PLL output must be in 16-24 MHz range");
                while (RCC->CFGR & RCC_CFGR_SWS) RCC->CFGR &= ~RCC_CFGR_SW; // Переходим на HSI
                RCC->CR &= ~RCC_CR_PLLON; // Выключаем PLL
                constexpr uint32_t pllmul = freq / 4000000;
                static_assert(pllmul >= 2 && pllmul <= 16 && pllmul * 4000000 == freq, "Incorrect frequency");
                RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_PLLMULL | RCC_CFGR_PLLSRC)) // HSI => PLL
                        | ((pllmul - 2) << RCC_CFGR_PLLMULL_Pos); // Устанавливаем множитель PLL
                RCC->CR |= RCC_CR_PLLON; // Включаем PLL
                while (!(RCC->CR & RCC_CR_PLLRDY));
                RCC->CFGR |= RCC_CFGR_SW_PLL; // Переходим на PLL
                while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
                constexpr uint32_t hpre = freq / HCLK;
                static_assert(hpre * HCLK == freq, "Wrong HCLK frequency");
                /*switch (hpre) {
                case 1 : hpre = 0;
                case 2 : hpre =
                }*/

            }
        }
    }
};

#endif // CLOCK_H
