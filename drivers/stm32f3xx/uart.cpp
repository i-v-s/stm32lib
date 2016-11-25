#include "uart.h"

void UART::uartInit(USART_TypeDef * USART, DMA_TypeDef * DMA, DMA_Channel_TypeDef * DMA_RX, DMA_Channel_TypeDef * DMA_TX)
{
    this->USART = USART;
    this->DMA = DMA;
    this->DMA_RX = DMA_RX;
    this->DMA_TX = DMA_TX;
    DMA_RX->CNDTR = sizeof(rxData);
    DMA_RX->CMAR = (uint32_t) rxData;
    DMA_RX->CPAR = (uint32_t) &USART->RDR;
    DMA_RX->CCR = DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_EN | DMA_CCR_CIRC
        | DMA_CCR_TEIE | DMA_CCR_HTIE | DMA_CCR_TCIE;

    DMA_TX->CCR = DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_DIR
        | DMA_CCR_TEIE | DMA_CCR_TCIE;
    DMA_TX->CNDTR = 0;
    DMA_TX->CPAR = (uint32_t) &USART->TDR;
    
    NVIC->ISER[0] = (1 << 14) | (1 << 15);
    NVIC->ISER[1] = (1 << 5);
    
    //USART->BRR = 0x0753; // 38400
    //USART->BRR = 0x1D4C; //  9600
    //USART->BRR = 0x04E2; // 57600
    USART->BRR = 0x271;  // 115200
    
    USART->RTOR = 20;
    USART->CR1 = USART_CR1_UE | USART_CR1_RTOIE;
    USART->CR2 = USART_CR2_RTOEN;
    USART->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
    USART->CR1 |= USART_CR1_TE | USART_CR1_RE;    
}

//#include <stdio.h>
//Queue<char, 256> log;

char * UART::transmit(void * obj, char * start, char * end)
{
    UART * u = (UART *) obj;
    DMA_Channel_TypeDef * DMA_TX = u->DMA_TX;
    char * pos = u->txPos;
    if(!pos) pos = start;
    if(uint32_t ctr = DMA_TX->CNDTR) return pos - ctr;
    uint32_t CCR = DMA_TX->CCR;
    DMA_TX->CCR = CCR & ~DMA_CCR_EN; // Выключаем DMA
    //log.log("-");
    int size = end - pos;
    if(!size) return end;
    if(size < 0) 
    {
        pos = start;
        size = end - start;
        //log.log("<");
    }
    DMA_TX->CMAR = (uint32_t)pos;
    DMA_TX->CNDTR = size;
    u->txPos = pos + size;
    //char buf[20];
    //sprintf(buf, "+%d", size);
    //log.log(buf);
    DMA_TX->CCR = CCR | DMA_CCR_EN; // Включить отправку
    return pos;
}

void UART::rxNotEmpty()
{
    static bool work = false;
    if(work) return;
    work = true;
    char * pos = rxPos, * p = rxData + sizeof(rxData) - DMA_RX->CNDTR;
    if(p > pos)
        rxOutput(pos, p);
    else if(p < pos)
    {
        rxOutput(pos, rxData + sizeof(rxData));
        rxOutput(rxData, p);
        rxPos = p;    
    }
    rxPos = p;
    work = false;
}

/////////////////////// Обработка прерываний для UART //////////////////////////////////////////////


#ifdef __cplusplus
 extern "C" {
#endif

extern UART &uart;
     
void DMA1_Channel4_IRQHandler() // Весь буфер отправлен
{
    int isr = DMA1->ISR;
    if(int t = isr & DMA_ISR_TCIF4)
    {
        DMA1->IFCR = t;
        //log.log("!");
        uart.tx.pull();
    }
}

void DMA1_Channel5_IRQHandler() // При приёме заполнено полбуфера или весь буфер
{
    int isr = DMA1->ISR;
    if(int t = isr & (DMA_ISR_HTIF5 | DMA_ISR_TCIF5))
    {
        uart.rxNotEmpty();
        DMA1->IFCR = t;
    }
}

void USART1_IRQHandler() // Прошло достаточное время с последнего приёма
{
    int isr = USART1->ISR;
    if(int t = isr & USART_ISR_RTOF)
    {
        uart.rxNotEmpty();
        USART1->ICR = t;
    }
}

#ifdef __cplusplus
}
#endif

#ifdef _TEST_
#include "test.h"

class TestUART: public Test
{
public:
    TestUART(): Test("UART"){};
} testUART;

beginTest(Transmit)
{
    UART uart;
    USART_TypeDef USART;
    DMA_TypeDef DMA; 
    DMA_Channel_TypeDef DMA_RX, DMA_TX;
        
    uart.uartInit(&USART, &DMA, &DMA_RX, &DMA_TX);
    char buf[20];
    uart.tx.log("abcde");
    char * t = (char *)DMA_TX.CMAR;
    memcpy(buf, t, DMA_TX.CNDTR);
    buf[DMA_TX.CNDTR] = 0;
    if(strcmp(buf, "abcde")) return "error";
    
    return 0;
}
endTest(Transmit, testUART);

#endif
