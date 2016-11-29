//#include "queue.h"
//#include "hw_config.h"

////// Тестирование на устойчивость к прерываниям /////////////////////////////////////////
#ifdef _TEST_
#include "test.h"

class TestQueue: public Test
{
public:
    TestQueue(): Test("Queue"){};
} testQueue;

beginTest(Basic)
{
    Queue<char, 32> q;
    if(q.last()) return "last error";
    q.log("abc");
    const char * last = q.last();
    if(!last || *last != 'a') return "last error";
    q.push('B');
    char * t = "cbx";
    q.push(t, t + strlen(t));
    char buf[20];
    int x;
    for(x = 0; x < 20; x++)
    {
        char d;
        if(!q.pop(&d)) { buf[x] = 0; break;}
        buf[x] = d;
    }
    if(x == 20) return "pop error";
    if(strcmp(buf, "abcBcbx")) return "failed";
    
    return 0;
}
endTest(Basic, testQueue);

beginTest(Chain)
{
    Queue<char, 64> a;
    a.output.set(&a.input, &a);
    a.source = &a;
    a.log("123");
    char buf[20];
    if(a.pull(buf, 20) != 3) return "selfchain error";
    buf[3] = 0;
    if(strcmp(buf, "123")) return "selfchain error";
    if(a.length()) return "length error";

    Queue<char, 8> b;
    a.output.set(&b.input, &b);
    b.source = &a;
    a.log("0123456789");
    int t = b.pull(buf, 20);
    t += b.pull(buf + t, 20 - t);
    buf[t] = 0;
    if(strcmp(buf, "0123456789")) return "chain error";
    
    
    return 0;
    
    
}
endTest(Chain, testQueue);


class CircularTest: private Queue<char, 256>
{
private:
    friend void testSysTick_Handler();
    char testBuf[64], * tb;
    int volatile testSendCount;
    int volatile irq;
    void * lastStack;
    char * volatile _src, * volatile _dst, * volatile _newOutDst;
public:    
    CircularTest(): tb(testBuf), testSendCount(0), irq(0), lastStack(0) {};
    const char * run();
    static char * testReceiver(void * obj, char * start, char * end);
};

CircularTest ctest;

void testSysTick_Handler(void)
{
    if(ctest.testSendCount > 0) ctest.testSendCount--;
    else SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    ctest.log("(IRQ)");
    ctest.irq++;
    ctest._newOutDst = ctest.newDst;
    ctest._dst = ctest.dst;
    ctest._src = ctest.src;
    void * t[10], ** tt = t;
    ctest.lastStack = tt[18];
}


char * CircularTest::testReceiver(void * obj, char * start, char * end)
{
    int size = end - start;
    memcpy(ctest.tb, start, size);
    ctest.tb += size;
    *ctest.tb = 0;
    return start + size;
}

//#define __LDREX(a) *(a)
//#define __STREX(b, a) ((*(a) = b) & 0)   

const char * CircularTest::run()
{
    // Разгоняем SysTick до максимума
    //SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE;
    //*(void * *)0x3C = (void *)IRQTest;
    const char * error = 0;
    int abc, def;
    int X = 768;
    for(X = 0; X < 10000; X++)
    //for(X = 768; X < 769; X++)
    {
        testSendCount = X >> 11;
        irq = 0;
        SysTick->LOAD  = 1 + (X & 0x7FF);
        NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Systick Interrupt */
        SysTick->VAL   = 0;
        SysTick->CTRL  = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

        log("(ABC)");
        log("(DEF)");
        
        tb = testBuf;
        output = &testReceiver;
        pull();
        while(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk);
        pull();
        pull();
        abc = 1; def = 1;
        for(char * x = testBuf; *x; x++)
        {
            if(*x == '(')
            {
                char * t = x + 1;
                while(*x && *x != ')') x++;
                if(*x != ')') { error = "no ')'"; break;}
                char bbb[32];
                if(x - t > sizeof(bbb) - 1) { error = "too long"; break;}
                memcpy(bbb, t, x - t);
                bbb[x - t] = 0;
                if(!strcmp("ABC", bbb)) abc--;
                else if(!strcmp("DEF", bbb)) def--;
                else if(!strcmp("IRQ", bbb)) irq--;
                else {error = "unknown string"; break;}
            }
        }
        if(error) break;
        if(abc || def || irq) {error = "non zero"; break;}
    }
    return error;
}

const char * testOut()
{
    return ctest.run();
}

#endif
