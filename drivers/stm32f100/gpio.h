#ifndef PIN_H
#define PIN_H
#include <stm32f1xx.h>


enum IO_Mode
{
    IO_Analog = 0,
    IO_In = 1,
    IO_Out = 2,
    IO_AF0 = 8,
    IO_AF1 = 9,
    IO_AF2 = 10,
    IO_AF3 = 11,
    IO_AF4 = 12,
    IO_AF5 = 13,
    IO_AF6 = 14,
    IO_AF7 = 15
};

enum IO_Speed
{
    IO_10MHz = 1,
    IO_2MHz = 2,
    IO_50MHz = 3
};

enum IO_Pull
{
    IO_NoPull = 0,
    IO_Pullup = 1,
    IO_Pulldown = 2
};

template<uint32_t gpio, uint32_t mask>
struct Pins
{
    inline uint32_t operator =(uint32_t b)
    {
        ((GPIO_TypeDef *)gpio)->BSRR = (b & mask) | ((~b && mask) << 16);
        return b;
    }
    inline operator uint32_t()
    {
        return *(uint16_t *)(&((GPIO_TypeDef *)gpio)->IDR) & mask;
    }
    constexpr uint64_t mask4()
    {
        uint64_t o = 0;
        for (uint32_t m = mask; m; m <<= 1) o = (o << 4) | ((m >> 15) & 1);
        return o;
    }

    template<IO_Mode mode, IO_Speed speed = IO_2MHz, bool openDrain = false, IO_Pull pull = IO_NoPull>
    inline void init()
    {
        GPIO_TypeDef * const _gpio = (GPIO_TypeDef *) gpio;
        static_assert(mask <= 0xFFFF, "Wrong pins mask");
        const uint64_t m4 = mask4();
        const uint64_t mm = m4 | (m4 << 1) | (m4 << 2) | (m4 << 3);
        uint64_t cr;
        switch (mode) {
        case IO_Analog : cr = 0; break;
        case IO_In     :
            switch (pull) {
            case IO_NoPull   : cr = m4 << 2; break;
            case IO_Pulldown : cr = m4 << 3; set(); break;
            case IO_Pullup   : cr = m4 << 3; clear(); break;
            }
            break;
        case IO_Out    :
        default        :
            cr = speed * m4;
            if (openDrain) cr += m4 << 2;
            if (mode >= IO_AF0) {
                cr += m4 << 3;

            }
            break;
        }

        _gpio->CRL = (_gpio->CRL & (uint32_t)mm) | (uint32_t)cr;
        _gpio->CRH = (_gpio->CRH & (uint32_t)(mm >> 32)) | (uint32_t)(cr >> 32);
    }
    inline void set(void) { ((GPIO_TypeDef *) gpio)->BSRR = mask; }
    inline void clear(void) { ((GPIO_TypeDef *) gpio)->BRR = mask; }
};

#endif // PIN_H
