#include <string.h>
#include <cstdint>
#include <stm32f1xx.h>
#include <core_cm3.h>
#include "../queue.h"

enum EUsart
{
    Usart1 = (uintptr_t)USART1,
    Usart2 = (uintptr_t)USART2,
    Usart3 = (uintptr_t)USART3
};

enum Parity
{
    PNone = 0,
    POdd,
    PEven
};

template<EUsart eu>
class Usart
{
public:
    Queue<char, 64> tx;
    static constexpr USART_TypeDef &u() { return * (USART_TypeDef *) eu; }
private:
    char rxData[512];
    char * volatile rxPos;
    USART_TypeDef * USART;
    DMA_TypeDef * DMA;
    DMA_Channel_TypeDef * DMA_RX, * DMA_TX;
    char * txPos;
    static char * transmit(void * obj, char * start, char * end) { return nullptr; }
public:

    Receiver<char> rxOutput;
    Usart(): txPos(0), rxOutput(0) { tx.output.set(&transmit, this); rxPos = rxData;}
    //void uartInit(USART_TypeDef * USART, DMA_TypeDef * DMA, DMA_Channel_TypeDef * DMA_RX, DMA_Channel_TypeDef * DMA_TX);
    template<class Clock, uint32_t baud, uint8_t length = 8, Parity parity = PNone>
    void init() {
        u().CR1 = USART_CR1_UE; // Включаем USART

        constexpr uint32_t pclk = (eu == Usart1) ? Clock::pclk2 : Clock::pclk1;
        u().BRR = pclk / baud; // Устанавливаем скорость

        uint32_t cr1 = USART_CR1_UE;
        constexpr int sz = length + ((parity == PNone) ? 0 : 1);
        static_assert(sz == 8 || sz == 9, "Usart length/parity wrong");
        if(sz == 9) cr1 |= USART_CR1_M;
        if(parity) cr1 |= USART_CR1_PCE | ((parity == POdd) ? USART_CR1_PS : 0);
        cr1 |= USART_CR1_TE | USART_CR1_RE;
        u().CR1 = cr1;
        u().CR2 = 0;
        u().CR3 = 0;
    }
    void send(uint32_t data)
    {
        while(!(u().SR & USART_SR_TXE));
        u().DR = data;
    }

    //void rxNotEmpty();
};
