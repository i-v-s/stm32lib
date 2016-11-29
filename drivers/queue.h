#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <string.h>
#include <assert.h>

template<typename T> struct Receiver
{
    typedef T * (* Method)(void * obj, T * start,  T * end);
    Method method;
    void * object;
    inline T * operator()(T * start, T * end){return method(object, start, end);}
    Receiver(Method method, void * object = 0) : method(method), object(object) {}
    inline void set(Method rcv, void * obj = 0) {method = rcv; object = obj;}
    inline void set(const T * (* rcv)(void * obj, const T * start, const T * end), void * obj) {method = (Method)rcv; object = obj;}
};

class Pullable
{
public:
    virtual void pull() = 0;
};

template<typename T, int SIZE> class Queue: public Pullable
{
protected:
    T data[SIZE];
    T * volatile src, * volatile dst, * volatile newDst;
    //char outState = 1;
    volatile bool pullWork;
    //static int slen(const T * s) { const T * x = s; for(; *x; x++); return x - s;}
public:
    Pullable * source;
    Receiver<T> output;
    inline int length() const {T * d = dst; return (d - src) & (SIZE - 1);}
    Queue(): src(data), dst(data), newDst(data), output(0), pullWork(false), source(0) {}
    virtual void pull()
    {
        if(!output.method || pullWork) return;
        pullWork = true;
        
        T * s = src, * d = dst;
        int size = (d - s) & (SIZE - 1); // Сколько всего надо отправить
        if(!size)
        {
            pullWork = false;
            return;
        }
        T * end = s + size;
        if(end >= data + SIZE)
        {
            s = output(s, data + SIZE);
            if(s >= data + SIZE)
                s = (end > data + SIZE) ? output(data, end - SIZE) : data;
        }
        else
            s = output(s, end);
        assert(s >= data && s <= end && s < data + SIZE);
        src = s;
        if(source) source->pull();
        pullWork = false;
    }
    int pull(T * dest, int size)
    {
        if(pullWork) return 0;
        pullWork = true;
        
        T * s = src, * d = dst;
        int count = (d - s) & (SIZE - 1); // Сколько всего надо отправить
        if(size > count) size = count;
        if(!size)
        {
            pullWork = false;
            return 0;
        }
        T * end = s + size;
        if(end >= data + SIZE)
        {
            int t = data + SIZE - s;
            memcpy(dest, s, t);
            end -= SIZE;
            dest += t;
            memcpy(dest, data, end - data);
        }
        else
            memcpy(dest, s, end - s);
        //assert(s >= data && s <= end && s < data + SIZE);
        src = end;
        if(source) source->pull();
        pullWork = false;
        return size;
    }
    const T * push(const T * text, const T * end)
    {
        T * s = src, * d, * nd; int f, size = end - text;
        do
        { // Попытка выделить место в буфере для записи
            d = (T *)__LDREXW((unsigned long *)&newDst);
            f = d - dst; // Если значения равны - значит мы первые
            int empty = (s - d - 1) & (SIZE - 1);
            if(!empty)
            { // Некуда писать
                __CLREX();
                pull();
                return text;
            }
            if(size > empty) size = empty;
            nd = d + size;
            if(nd > data + SIZE) nd -= SIZE;
        } while(__STREXW((unsigned long)nd, (unsigned long *)&newDst));
        int left = data + SIZE - d; // Сколько до конца отрезка
        if(left >= size) // Производим запись
            memcpy(d, text, size * sizeof(T));
        else
        {
            memcpy(d, text, left * sizeof(T));
            memcpy(data, text + left, (size - left) * sizeof(T));
        }
        if(f) return text + size;
        do 
        { // Попытка обновить dst
            nd = (T *)__LDREXW((unsigned long *)&newDst);
            dst = nd;
        } while(__STREXW((unsigned long)nd, (unsigned long *)&newDst));
        pull();
        return text + size;
    }
    inline bool push(const T &val) { return push(&val, &val + 1) > &val;}
    inline bool pop(T * result = 0) 
    {
        T * s;
        do
        {
            s = (T *)__LDREXW((unsigned long *)&src);
            if(s == dst) { __CLREX(); return false;}
            if(result) *result = *(s++);
            if(s >= data + SIZE) s -= SIZE;
        } while(__STREXW((unsigned long)s, (unsigned long *)&src));
        return true;
    }
    inline const T * last() const {T * s = src; return (dst == s) ? 0 : s;}
    static const T * input(void * obj, const T * start, const T * end)
    {
        Queue<T, SIZE> * q = (Queue<T, SIZE> *) obj;
        return q->push(start, end);
    }
    inline int log(const T * text)
    {
        int size = strlen(text);
        return text + size - push(text, text + size);
    }
};

#endif
