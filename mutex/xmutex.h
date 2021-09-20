#ifndef XMUTEX_H
#define XMUTEX_H

/* RTC-X specific mutex implementation */
#include <stdint.h>


typedef struct {
        uint64_t lock;
} xmutex_t;


void xmutex_init(xmutex_t *mtx);
void xmutex_lock(xmutex_t *mtx);
void xmutex_unlock(xmutex_t *mtx);


typedef struct {
        uint64_t msg;
        xmutex_t count_lock;
        uint64_t count;
} xsig_t;

void xsig_init(xsig_t *signal);
void coherent_wait(xsig_t *signal, uint64_t msg);
void coherent_signal(xsig_t *signal, uint64_t msg, uint64_t nconsumers);
#endif
