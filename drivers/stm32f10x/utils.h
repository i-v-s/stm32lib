#ifndef UTILS_H
#define UTILS_H
#define TEST_TEMPLATES

// Options //////////////////////////////////////////

template<typename... Args> struct Options;
template<> struct Options<> { enum { mask = 0, value = 0 }; };
template<typename Arg, typename... Args> struct Options<Arg, Args...>
{
    enum { 
        mask = uint32_t(Options<Args...>::mask) | uint32_t(Arg::flag),
        value = uint32_t(Options<Args...>::value) | uint32_t(Arg::value)// ? uint32_t(Arg::flag) : 0)
    };
};

template<typename... Args> struct Options2;
template<> struct Options2<> { enum { cr1m = 0, cr2m = 0, cr1 = 0, cr2 = 0 }; };
template<typename Arg, typename... Args> struct Options2<Arg, Args...>
{
    typedef Options2<Args...> Next;
    enum { 
        cr1m = uint32_t(Next::cr1m) | uint32_t(Arg::cr1m),
        cr2m = uint32_t(Next::cr2m) | uint32_t(Arg::cr2m),
        cr1 = uint32_t(Next::cr1m) | uint32_t(Arg::cr1),
        cr2 = uint32_t(Next::cr2m) | uint32_t(Arg::cr2)
    };
};


// DupValues //////////////////////////////////////////

template<uint32_t outMask, int fieldSize, uint32_t mask, uint32_t value, int outP = 0> struct DupValues{
    static_assert(!(value & ~mask), "Wrong mask!");
    typedef DupValues<outMask & ~(1 << outP), fieldSize, mask, value, outP + 1> Next;
    enum {
        om = outMask & (1 << outP),
        msk = Next::msk | (om ? mask << (fieldSize * outP) : 0),
        val = Next::val | (om ? value << (fieldSize * outP) : 0)
    };
    static inline constexpr bool empty() { return !msk; }
    template<typename T> static inline void patch(T &item) {
        if (msk)
            item = (item & ~msk) | val;
    }
};

template<int fieldSize, uint32_t mask, uint32_t value, int outP> struct DupValues<0, fieldSize, mask, value, outP> { 
    enum { msk = 0, val = 0 }; 
    static inline constexpr bool empty() { return true; }
    template<typename T> static inline void patch(T &item) {}
};

#ifdef TEST_TEMPLATES

static_assert(DupValues<0, 2, 3, 1>::empty(), "Not empty");
static_assert(DupValues<5, 2, 0, 0>::empty(), "Not empty");
static_assert(DupValues<1, 2, 3, 1>::val == 1, "Wrong value"); // 01
static_assert(DupValues<2, 2, 3, 1>::val == 4, "Wrong value"); // 01 00
static_assert(DupValues<3, 2, 3, 1>::val == 5, "Wrong value"); // 01 01
static_assert(DupValues<5, 2, 3, 1>::val == 17, "Wrong value"); // 01 00 01
static_assert(DupValues<5, 3, 3, 1>::msk == 195, "Wrong mask"); // 011 000 011

#endif
#endif
