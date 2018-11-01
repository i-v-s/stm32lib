#ifndef TIMER_H
#define TIMER_H

#include <stm32f1xx.h>
#include "utils.h"
#include "gpio.h"

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

// APB1 timers:
template<> inline constexpr ClockSource getClockSource<(uintptr_t)TIM2>() { return CS_PCLK1_TIM; };
#ifdef TIM3
template<> inline constexpr ClockSource getClockSource<(uintptr_t)TIM3>() { return CS_PCLK1_TIM; };
#endif
#ifdef TIM4
template<> inline constexpr ClockSource getClockSource<(uintptr_t)TIM4>() { return CS_PCLK1_TIM; };
#endif

// APB2 timers
template<> inline constexpr ClockSource getClockSource<(uintptr_t)TIM1>() { return CS_PCLK2_TIM; };
#ifdef TIM8
template<> inline constexpr ClockSource getClockSource<(uintptr_t)TIM8>() { return CS_PCLK2_TIM; };
#endif
#ifdef TIM9
template<> inline constexpr ClockSource getClockSource<(uintptr_t)TIM9>() { return CS_PCLK2_TIM; };
#endif
#ifdef TIM10
template<> inline constexpr ClockSource getClockSource<(uintptr_t)TIM10>() { return CS_PCLK2_TIM; };
#endif
#ifdef TIM11
template<> inline constexpr ClockSource getClockSource<(uintptr_t)TIM11>() { return CS_PCLK2_TIM; };
#endif

template<uintptr_t tim, typename Clock, const Clock *clock = nullptr> struct Timer;
template<class Timer_, uint8_t channel> class TimerChannel;

namespace tim_ {
    // Timer options
    template<bool on = true> struct Preload { enum { cr1m = TIM_CR1_ARPE, cr1 = on ? cr1m : 0, cr2m = 0, cr2 = 0}; };
    template<bool on = true> struct OnePulse { enum { cr1m = TIM_CR1_OPM, cr1 = on ? cr1m : 0, cr2m = 0, cr2 = 0}; };
    struct Enable { enum { cr1m = TIM_CR1_CEN, cr1 = cr1m, cr2m = 0, cr2 = 0}; };
    struct Disable { enum { cr1m = TIM_CR1_CEN, cr1 = 0, cr2m = 0, cr2 = 0}; };
    struct SendReset  { enum { cr1m = 0, cr1 = 0, cr2m = TIM_CR2_MMS_Msk, cr2 = (0 << TIM_CR2_MMS_Pos) }; };
    struct SendEnable { enum { cr1m = 0, cr1 = 0, cr2m = TIM_CR2_MMS_Msk, cr2 = (1 << TIM_CR2_MMS_Pos) }; };
    struct SendUpdate { enum { cr1m = 0, cr1 = 0, cr2m = TIM_CR2_MMS_Msk, cr2 = (2 << TIM_CR2_MMS_Pos) }; };    
    struct SendComparePulse { enum { cr1m = 0, cr1 = 0, cr2m = TIM_CR2_MMS_Msk, cr2 = (3 << TIM_CR2_MMS_Pos) }; };
    template<class Channel> struct SendCompare { enum { cr1m = 0, cr1 = 0, cr2m = TIM_CR2_MMS_Msk, cr2 = ((Channel::idx + 3) << TIM_CR2_MMS_Pos) }; };
    
    // Timer sources
    template<typename Src> struct SrcInfo;
    template<uintptr_t t, typename Clock, const Clock *clock> struct SrcInfo<Timer<t, Clock, clock>> 
    { enum { smcrMsk = TIM_SMCR_SMS_Msk, smcrVal = 0, tim = t }; };
    template<class Timer, uint8_t channel> struct SrcInfo<TimerChannel<Timer, channel>>
    { 
        static_assert(channel == 1 || channel == 2, "Wrong source channel");
        enum { smcrMsk = TIM_SMCR_SMS_Msk | TIM_SMCR_TS_Msk, smcrVal = (channel + 4) << TIM_SMCR_TS_Pos, tim = 0/*Timer::getTim()*/ }; 
    };
    struct TI1EdgeDetector;
    template<> struct SrcInfo<TI1EdgeDetector>
    { enum { smcrMsk = TIM_SMCR_SMS_Msk | TIM_SMCR_TS_Msk, smcrVal = 4 << TIM_SMCR_TS_Pos, tim = 0/*Timer::getTim()*/ }; };
    
    struct InternalClock { enum { smcrMsk = TIM_SMCR_SMS_Msk, smcrVal = 0, tim = 0 }; };
    template<int mode> struct Encoder { static_assert(mode >= 1 && mode <= 3, "Wrong encoder mode!"); enum { smcrMsk = TIM_SMCR_SMS_Msk, smcrVal = mode << TIM_SMCR_SMS_Pos, tim = 0 }; };
    template<typename Src> struct Reset    { typedef SrcInfo<Src> Info; enum { smcrMsk = Info::smcrMsk, smcrVal = uint32_t(Info::smcrVal) | (4 << TIM_SMCR_SMS_Pos), tim = Info::tim }; };
    template<typename Src> struct Gated    { typedef SrcInfo<Src> Info; enum { smcrMsk = Info::smcrMsk, smcrVal = uint32_t(Info::smcrVal) | (5 << TIM_SMCR_SMS_Pos), tim = Info::tim }; };
    template<typename Src> struct Trigger  { typedef SrcInfo<Src> Info; enum { smcrMsk = Info::smcrMsk, smcrVal = uint32_t(Info::smcrVal) | (6 << TIM_SMCR_SMS_Pos), tim = Info::tim }; };
    template<typename Src> struct External { typedef SrcInfo<Src> Info; enum { smcrMsk = Info::smcrMsk, smcrVal = uint32_t(Info::smcrVal) | (7 << TIM_SMCR_SMS_Pos), tim = Info::tim }; };
    template<typename... Args> struct SourceOptions;
    template<> struct SourceOptions<> { enum { smcrMsk = 0, smcrVal = 0, tim = 0 }; };
    
    template<typename Arg, typename... Args> struct SourceOptions<Arg, Args...>
    {
        typedef SourceOptions<Args...> Next;
        static_assert(!Arg::tim || !Next::tim, "Only one master allowed!");
        enum {
            smcrMsk = uint32_t(Next::smcrMsk) | uint32_t(Arg::smcrMsk),
            smcrVal = uint32_t(Next::smcrVal) | uint32_t(Arg::smcrVal),
            tim = Arg::tim ? Arg::tim : Next::tim
        };
    };

    // Input capture channel options
    template<bool en = true> struct Capture { enum { icMask = ((TIM_CCER_CC1E) << 8), icVal = en ? icMask : 0 }; };
    template<uint8_t filter> struct Filter { static_assert(filter < 16, "Wrong filter value"); enum { icMask = TIM_CCMR1_IC1F_Msk, icVal = (filter << TIM_CCMR1_IC1F_Pos) }; };
    template<typename... Args> struct InputOptions;
    template<> struct InputOptions<> { enum { 
        icVal  = 0, // Default: none
        icMask = 0
    };};
    template<typename Arg, typename... Args> struct InputOptions<Arg, Args...> {
        typedef InputOptions<Args...> Next;
        enum { 
            icMask = uint32_t(Next::icMask) | uint32_t(Arg::icMask),
            icVal = uint32_t(Next::icVal) | uint32_t(Arg::icVal)// ? uint32_t(Arg::flag) : 0)
        };
    };

    // Output compare channel options
    struct Fast          { enum { ocVal = TIM_CCMR1_OC1FE, ocMask = ocVal }; };
    struct PreloadCCR    { enum { ocVal = TIM_CCMR1_OC1PE, ocMask = ocVal }; };
    struct ClearOnETRF   { enum { ocVal = TIM_CCMR1_OC1CE, ocMask = ocVal }; };
    template<bool en = true> struct EnableIO { enum { ocMask = ((TIM_CCER_CC1E) << 8), ocVal = en ? ocMask : 0 }; };
    template<bool inv = true> struct Invert { enum { ocMask = ((TIM_CCER_CC1P) << 8), ocVal = inv ? ocMask : 0, icMask = ocMask, icVal = ocVal }; };
    enum OutputCompareEnum { Frozen = 0, Match = 1, NotMatch = 2, Toggle = 3, Low = 4, High = 5, PWM1 = 6, PWM2 = 7 };
        
    template<typename... Args> struct OutputOptions;
    template<> struct OutputOptions<> { enum { 
        ocVal = 0,  // Default: no fast, no preload, no clear, selection output
        ocMask = TIM_CCMR1_CC1S | TIM_CCMR1_OC1FE | TIM_CCMR1_OC1FE | TIM_CCMR1_OC1CE
    };};
    template<typename Arg, typename... Args> struct OutputOptions<Arg, Args...> { enum { 
        ocMask = uint32_t(OutputOptions<Args...>::ocMask) | uint32_t(Arg::ocMask),
        ocVal = uint32_t(OutputOptions<Args...>::ocVal) | uint32_t(Arg::ocVal)// ? uint32_t(Arg::flag) : 0)
    }; };
}

// Master - Slave definitions ////////////////////////////////////////////////////////

template<uintptr_t m, uintptr_t s> uint32_t getTS();
#define GET_TS(m, s, ts) template<> inline constexpr uint32_t getTS<uintptr_t(m), uintptr_t(s)>() { return (ts) << TIM_SMCR_TS_Pos; }
 // TIM2 - slave:
GET_TS(TIM1, TIM2, 0)
#ifdef TIM8
GET_TS(TIM8, TIM2, 1)
#endif
GET_TS(TIM3, TIM2, 2)
GET_TS(TIM4, TIM2, 3)
 // TIM3 - slave:
GET_TS(TIM1, TIM3, 0)
GET_TS(TIM2, TIM3, 1)
GET_TS(TIM4, TIM3, 3)
// TIM4 - slave
GET_TS(TIM1, TIM4, 0)
GET_TS(TIM2, TIM4, 1)
GET_TS(TIM3, TIM4, 2)
#ifdef TIM8
GET_TS(TIM8, TIM4, 3)
#endif
// TIM5 - Slave
#ifdef TIM5
GET_TS(TIM2, TIM5, 0)
GET_TS(TIM3, TIM5, 1)
GET_TS(TIM4, TIM5, 2)
#ifdef TIM8
GET_TS(TIM8, TIM5, 3)
#endif
GET_TS(TIM5, TIM3, 2)
#endif

template<uintptr_t tim, typename Clock, const Clock *clock>
struct Timer
{
    static inline constexpr uintptr_t getTim() { return tim; }
    static inline void update() { p().EGR = TIM_EGR_UG; }
    static inline uint32_t getCounter() { return p().CNT; }
    template<typename... Args> static inline void configure()
    {
        typedef Options2<Args...> O;
        if (O::cr1m)
            p().CR1 = (p().CR1 & ~O::cr1m) | O::cr1;
        if (O::cr2m)
            p().CR2 = (p().CR2 & ~O::cr2m) | O::cr2;        
    }

    template<uint32_t frequency> static inline void setFrequency()
    {
        uint32_t src = clock->template getFrequency<getClockSource<tim>()>();
        p().PSC = src / frequency - 1;
    }
    
    template<typename... Args> static inline void setSource()
    {
        typedef tim_::SourceOptions<Args...> O;
        uint32_t msk = O::smcrMsk, val = O::smcrVal;
        if (O::tim) {
            msk |= TIM_SMCR_TS;
            val |= getTS<O::tim, tim>();
        }
        p().SMCR = (p().SMCR & ~msk) | val;
    }
        
    __IO uint32_t *CCR;
    static inline constexpr TIM_TypeDef &p() { return *(TIM_TypeDef *) tim; }
    static inline constexpr __IO uint32_t &top() { return p().ARR; }
    template<int c> static inline constexpr __IO uint32_t &ccr()
    {
        static_assert(c >= 1 && c <= 4);
        switch(c) {
        default:
        case 1: return p().CCR1;
        case 2: return p().CCR2;
        case 3: return p().CCR3;
        case 4: return p().CCR4;
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
    inline static void enable()
    {
        p().CR1 |= TIM_CR1_CEN;
    }
    inline static void enable(uint32_t top)
    {
        p().ARR = top;
        p().CR2 = 0;
        p().CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
    }
    inline static constexpr uint32_t ahb() { return 0; }
    inline static constexpr uint32_t apb1() { 
        switch(tim) {
        default: return 0;
        case uintptr_t(TIM2): return RCC_APB1ENR_TIM2EN;
        case uintptr_t(TIM3): return RCC_APB1ENR_TIM3EN;
        case uintptr_t(TIM4): return RCC_APB1ENR_TIM4EN;
#ifdef TIM5
        case uintptr_t(TIM5): return RCC_APB1ENR_TIM5EN;
#endif
#ifdef TIM6
        case uintptr_t(TIM6): return RCC_APB1ENR_TIM6EN;
#endif
#ifdef TIM7
        case uintptr_t(TIM7): return RCC_APB1ENR_TIM7EN;
#endif
       }
    }
    inline static constexpr uint32_t apb2() { 
        switch(tim) {
        default: return 0;
        case uintptr_t(TIM1): return RCC_APB2ENR_TIM1EN;        
        }
    }
    static_assert(apb1() ^ apb2(), "No bus specified for timer!");
};

template<class Timer, uint8_t mask>
class TimerChannels
{
public:
    template<tim_::OutputCompareEnum mode, typename... Args> static inline void configureOutput()
    {
        typedef tim_::OutputOptions<Args...> O;
        constexpr uint8_t ccmrVal = (mode << TIM_CCMR1_OC1M_Pos) | (O::ocVal & 0xFF), ccmrMask = TIM_CCMR1_OC1M_Msk | (O::ocMask & 0xFF); 
        constexpr uint8_t ccerVal = (O::ocVal >> 8), ccerMask = (O::ocMask >> 8);
        DupValues<mask & 3,    8, ccmrMask, ccmrVal>::patch(Timer::p().CCMR1);
        DupValues<(mask >> 2), 8, ccmrMask, ccmrVal>::patch(Timer::p().CCMR1);
        DupValues<mask,        4, ccerMask, ccerVal>::patch(Timer::p().CCER);
    }
    template<typename... Args> static inline void configureInput()
    {
        typedef tim_::InputOptions<Args...> O;
        constexpr uint32_t icVal  = (O::icMask & TIM_CCMR1_CC1S_Msk) ? O::icVal : (O::icVal | TIM_CCMR1_CC1S_0); // Set default input
        constexpr uint32_t icMask = O::icMask | TIM_CCMR1_CC1S_Msk; 
        constexpr uint8_t ccmrVal = (icVal & 0xFF), ccmrMask = (icMask & 0xFF); 
        constexpr uint8_t ccerVal = (icVal >> 8), ccerMask = (icMask >> 8);
        DupValues<mask & 3,    8, ccmrMask, ccmrVal>::patch(Timer::p().CCMR1);
        DupValues<(mask >> 2), 8, ccmrMask, ccmrVal>::patch(Timer::p().CCMR1);
        DupValues<mask,        4, ccerMask, ccerVal>::patch(Timer::p().CCER);        
    }
};

template<class Timer_, uint8_t channel>
class TimerChannel : public TimerChannels<Timer_, (1 << (channel - 1))>
{
    static_assert(channel >= 1 && channel <= 4, "Wrong channel number");
public:
    typedef Timer_ Timer;
    static inline constexpr __IO uint32_t &ccr() { return Timer::template ccr<channel>(); }
    enum { idx = channel };
};

#define TIM_REMAP(tim, ch, port, pin, mask, val) template<typename Clock, const Clock *clock> struct Remap<TimerChannel<Timer<uintptr_t(tim), Clock, clock>, ch>, Pin<uintptr_t(port), pin>> { enum { maprMsk = mask, maprVal = val }; }

// TIM2
TIM_REMAP(TIM2, 1, GPIOA, 0,  AFIO_MAPR_TIM2_REMAP_0, 0);
TIM_REMAP(TIM2, 1, GPIOA, 15, AFIO_MAPR_TIM2_REMAP_0, AFIO_MAPR_TIM2_REMAP_0);
TIM_REMAP(TIM2, 2, GPIOA, 1, AFIO_MAPR_TIM2_REMAP_0, 0);
TIM_REMAP(TIM2, 2, GPIOB, 3, AFIO_MAPR_TIM2_REMAP_0, AFIO_MAPR_TIM2_REMAP_0);


// TIM3
TIM_REMAP(TIM3, 1, GPIOA, 6, AFIO_MAPR_TIM3_REMAP, 0);
TIM_REMAP(TIM3, 2, GPIOA, 7, AFIO_MAPR_TIM3_REMAP, 0);
TIM_REMAP(TIM3, 1, GPIOB, 4, AFIO_MAPR_TIM3_REMAP, AFIO_MAPR_TIM3_REMAP_1);
TIM_REMAP(TIM3, 2, GPIOB, 5, AFIO_MAPR_TIM3_REMAP, AFIO_MAPR_TIM3_REMAP_1);
TIM_REMAP(TIM3, 3, GPIOB, 0, AFIO_MAPR_TIM3_REMAP_0, 0);
TIM_REMAP(TIM3, 4, GPIOB, 1, AFIO_MAPR_TIM3_REMAP_0, 0);
TIM_REMAP(TIM3, 1, GPIOC, 6, AFIO_MAPR_TIM3_REMAP, AFIO_MAPR_TIM3_REMAP);
TIM_REMAP(TIM3, 2, GPIOC, 7, AFIO_MAPR_TIM3_REMAP, AFIO_MAPR_TIM3_REMAP);
TIM_REMAP(TIM3, 3, GPIOC, 8, AFIO_MAPR_TIM3_REMAP, AFIO_MAPR_TIM3_REMAP);
TIM_REMAP(TIM3, 4, GPIOC, 9, AFIO_MAPR_TIM3_REMAP, AFIO_MAPR_TIM3_REMAP);

// TIM4
TIM_REMAP(TIM4, 1, GPIOB, 6, AFIO_MAPR_TIM4_REMAP, 0);
TIM_REMAP(TIM4, 2, GPIOB, 7, AFIO_MAPR_TIM4_REMAP, 0);
TIM_REMAP(TIM4, 3, GPIOB, 8, AFIO_MAPR_TIM4_REMAP, 0);
TIM_REMAP(TIM4, 4, GPIOB, 9, AFIO_MAPR_TIM4_REMAP, 0);
TIM_REMAP(TIM4, 1, GPIOD, 12, AFIO_MAPR_TIM4_REMAP, AFIO_MAPR_TIM4_REMAP);
TIM_REMAP(TIM4, 2, GPIOD, 13, AFIO_MAPR_TIM4_REMAP, AFIO_MAPR_TIM4_REMAP);
TIM_REMAP(TIM4, 3, GPIOD, 14, AFIO_MAPR_TIM4_REMAP, AFIO_MAPR_TIM4_REMAP);
TIM_REMAP(TIM4, 4, GPIOD, 15, AFIO_MAPR_TIM4_REMAP, AFIO_MAPR_TIM4_REMAP);


//template<typename Clock, const Clock *clock> struct Remap<TimerChannel<Timer<uintptr_t(TIM2), Clock, clock>, 2>, Pin<uintptr_t(GPIOB), 3>> { enum { maprMsk = AFIO_MAPR_TIM2_REMAP_0, maprVal = maprMsk }; };

#ifdef TEST_TEMPLATES
static_assert(Options2<tim_::SendCompare<TimerChannel<int, 2>>>::cr2m == TIM_CR2_MMS, "Wrong mask!");
static_assert(Options2<tim_::SendCompare<TimerChannel<int, 2>>>::cr2 == (5 << TIM_CR2_MMS_Pos), "Wrong value!");
#endif

#endif // TIMER_H
