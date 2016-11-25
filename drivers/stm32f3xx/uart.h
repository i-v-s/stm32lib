#include <string.h>
#include "queue.h"
#include "mx_gpio.h"


class UART
{
public:
    Queue<char, 64> tx;
private:
    char rxData[512];
    char * volatile rxPos;
    USART_TypeDef * USART;
    DMA_TypeDef * DMA;
    DMA_Channel_TypeDef * DMA_RX, * DMA_TX;
    char * txPos;
    static char * transmit(void * obj, char * start, char * end);
public:
    Receiver<char> rxOutput;
    UART(): txPos(0), rxOutput(0) { tx.output.set(&transmit, this); rxPos = rxData;}
    void uartInit(USART_TypeDef * USART, DMA_TypeDef * DMA, DMA_Channel_TypeDef * DMA_RX, DMA_Channel_TypeDef * DMA_TX);
    void rxNotEmpty();
};
