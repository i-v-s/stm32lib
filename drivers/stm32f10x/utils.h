#ifndef UTILS_H
#define UTILS_H


template<typename... Args> struct Options;
template<> struct Options<> { enum { mask = 0, value = 0 }; };
template<typename Arg, typename... Args> struct Options<Arg, Args...>
{
    enum { 
        mask = uint32_t(Options<Args...>::mask) | uint32_t(Arg::flag),
        value = uint32_t(Options<Args...>::value) | uint32_t(Arg::value)// ? uint32_t(Arg::flag) : 0)
    };
};


#endif
