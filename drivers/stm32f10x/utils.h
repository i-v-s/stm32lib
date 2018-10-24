#ifndef UTILS_H
#define UTILS_H
#define TEST_TEMPLATES

// Data containers //////////////////////////////////

template<typename T, T mask, T value> struct Bits { 
    static_assert(!(value & ~mask), "Value not in mask"); 
    enum { m = mask, v = value };
    template<typename Reg> static inline void apply(Reg &reg) { if (mask) reg = (reg & ~mask) | value; }
};
template<uint32_t mask, uint32_t value> using Bits32 = Bits<uint32_t, mask, value>;
template<typename T, T value> struct Value { enum { v = value }; template<typename Reg> static inline void apply(Reg &reg) { reg = value; }};
template<typename... Args> struct List;/* {
    template<template<typename> typename apply> static inline void forEach();
};*/
template<typename Arg, typename... Args> 
struct List<Arg, Args...>
{
    template<template<typename> typename apply>
    static inline void forEach() 
    { 
        apply<Arg>(); 
        List<Args...>::forEach();
    }
};
template<> struct List<>
{
    template<template<typename> typename apply> 
    static inline void forEach() 
    { 
    }
};

struct NoValue { enum { m = 0, v = 0 }; template<typename Reg> static inline void apply(Reg &) {}; };
using None = NoValue;
using NV = NoValue;

// apply() function
template<typename Value, typename Variable> void apply(Variable &variable);
template<typename T, T mask, T value, typename Variable> 
    inline void apply<Bits<T, mask, value>, Variable>(Variable &variable) { if(mask) variable = (variable & ~mask) | value; }
template<typename T, T value, typename Variable> 
    inline void apply<Value<T, value>, Variable>(Variable &variable) { variable = value; }
template<typename Variable>
    inline void apply<NoValue, Variable>(Variable &) {}

// Merge of containers:
template<typename A, typename B> struct Merge;
template<typename T, T m1, T v1, T m2, T v2> struct Merge<Bits<T, m1, v1>, Bits<T, m2, v2>> {
    static_assert(!((v1 ^ v2) & m1 & m2), "Unable to merge Bits");
    typedef Bits<T, m1 | m2, v1 | v2> Result;
};
template<typename T, T m, T v> struct Merge<Bits<T, m, v>, NoValue> { typedef Bits<T, m, v> Result; };
template<typename T, T m, T v> struct Merge<NoValue, Bits<T, m, v>> { typedef Bits<T, m, v> Result; };

template<typename T, T value> struct Merge<Value<T, value>, NoValue> { using Result = Value<T, value>; };
template<typename T, T value> struct Merge<NoValue, Value<T, value>> { using Result = Value<T, value>; };
template<> struct Merge<NoValue, NoValue> { using Result = NoValue; };
template<typename... A1, typename... A2> struct Merge<List<A1...>, List<A2...>> { using Result = List<A1..., A2...>; };
//template<typename T1, T1 value1, typename T2, T2 value2> struct Merge<Value<T1, value1>, Value<T2, value2>> { using Result = Value<T, value>; };

// Reduce of containers:
template<typename... Args> struct Reduce_;
template<typename... Args> using Reduce = typename Reduce_<Args...>::Result;
template<typename Arg> struct Reduce_<Arg> { typedef Arg Result; };
template<typename... Args, typename Arg> struct Reduce_<Arg, Args...> { typedef typename Merge<Arg, Reduce<Args...>>::Result Result; };

// Iterate
template<typename F, typename List> struct Iterate;
template<typename F, typename Arg, typename... Args> struct Iterate<F, List<Arg, Args...>>
{
    static inline void apply()
    {
        F::template apply<Arg>();
        Iterate<F, List<Args...>>::apply();
    }
};
template<typename F> struct Iterate<F, List<>> { static inline void apply() {}; };



// For each
//template<typename... Args>
//void forEach()

// Options //////////////////////////////////////////

template<typename... Args> struct Options;
template<> struct Options<> { enum { mask = 0, value = 0 }; };
template<typename Arg, typename... Args> struct Options<Arg, Args...>
{
    typedef Options<Args...> Next;
    enum { 
        mask  = uint32_t(Next::mask)  | uint32_t(Arg::flag),
        value = uint32_t(Next::value) | uint32_t(Arg::value)
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
        cr1 = uint32_t(Next::cr1) | uint32_t(Arg::cr1),
        cr2 = uint32_t(Next::cr2) | uint32_t(Arg::cr2)
    };
    template<typename T> static inline void configure(T &CR1, T &CR2) {
        static_assert(cr1m || cr2m, "Nothing to configure");
        if (cr1m)
            CR1 = (CR1 & ~cr1m) | cr1;
        if (cr2m)
            CR2 = (CR2 & ~cr2m) | cr2;
    }
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

template<int v, int b = 0>
inline constexpr bool toBool()
{
    static_assert(v - b == 1 || v - b == 0, "Wrong parameter!");
    return bool(v - b);
}

#ifdef TEST_TEMPLATES

static_assert(Merge<Bits<uint32_t, 3, 2>, Bits<uint32_t, 6, 2>>::Result::m == 7, "Wrong mask!");
static_assert(Reduce<Bits<uint32_t, 3, 2>, Bits<uint32_t, 6, 2>>::m == 7, "Wrong mask!");
static_assert(Reduce<Bits<uint32_t, 1, 1>, Bits<uint32_t, 2, 2>, Bits<uint32_t, 4, 0>>::m == 7, "Wrong mask!");
static_assert(Reduce<Bits<uint32_t, 1, 1>, Bits<uint32_t, 2, 2>, Bits<uint32_t, 4, 0>>::v == 3, "Wrong mask!");

//static_assert(Merge<>);

static_assert(DupValues<0, 2, 3, 1>::empty(), "Not empty");
static_assert(DupValues<5, 2, 0, 0>::empty(), "Not empty");
static_assert(DupValues<1, 2, 3, 1>::val == 1, "Wrong value"); // 01
static_assert(DupValues<2, 2, 3, 1>::val == 4, "Wrong value"); // 01 00
static_assert(DupValues<3, 2, 3, 1>::val == 5, "Wrong value"); // 01 01
static_assert(DupValues<5, 2, 3, 1>::val == 17, "Wrong value"); // 01 00 01
static_assert(DupValues<5, 3, 3, 1>::msk == 195, "Wrong mask"); // 011 000 011

#endif
#endif
