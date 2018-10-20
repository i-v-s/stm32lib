#ifndef PIN_H
#define PIN_H
#include <stm32f1xx.h>


enum IO_Mode
{
    IO_Analog = 0,
    IO_In = 1,
    IO_Out = 2,
    IO_AF0 = 8,
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

template<uintptr_t gpio, uint32_t mask>
struct Pins
{
    static inline constexpr uint16_t getMask() { return mask; }
    static inline constexpr GPIO_TypeDef &p() { return *(GPIO_TypeDef *)gpio;}
    inline uint32_t operator =(uint32_t b)
    {
        p().BSRR = (b & mask) | ((~b && mask) << 16);
        return b;
    }
    static inline uint32_t get() { return p().IDR & mask; }
    inline operator uint32_t() { return p().IDR & mask; }
    static constexpr uint64_t mask4()
    {
        uint64_t o = 0;
        for (int x = 0; x < 16; x++)
            o |= (uint64_t((mask >> x) & 1) << (x * 4));
        return o;
    }

    template<IO_Mode mode, IO_Speed speed = IO_2MHz, bool openDrain = false, IO_Pull pull = IO_NoPull>
    static inline void configure()
    {
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

        p().CRL = (p().CRL & ~(uint32_t)mm) | (uint32_t)cr;
        p().CRH = (p().CRH & ~(uint32_t)(mm >> 32)) | (uint32_t)(cr >> 32);
    }
    static inline void set(void)   { p().BSRR = mask; }
    static inline void clear(void) { p().BRR  = mask; }
};

template<uintptr_t gpio, uint32_t idx>
struct Pin : public Pins<gpio, (1 << idx)> {};

template<typename Device, typename Pin> struct Remap;
template<typename... Args> struct Remaps;
template<> struct Remaps<> { enum { maprMsk = 0, maprVal = 0 }; };
template<typename Remap, typename... Args> struct Remaps<Remap, Args...> {
    typedef Remaps<Args...> Next;
    static_assert(!(uint32_t(Remap::maprMsk) & uint32_t(Next::maprMsk) & (uint32_t(Remap::maprVal) ^ uint32_t(Next::maprVal))), "Conflicting remaps");
    enum {
        maprMsk = uint32_t(Remap::maprMsk) | uint32_t(Next::maprMsk),
        maprVal = uint32_t(Remap::maprVal) | uint32_t(Next::maprVal)
    };
};

template<typename... Args>
inline void configureRemaps()
{
    typedef Remaps<Args...> R;
    static_assert(R::maprMsk, "Nothing to remap!");
    AFIO->MAPR = (AFIO->MAPR & ~R::maprMsk) | R::maprVal;
}

#endif // PIN_H
