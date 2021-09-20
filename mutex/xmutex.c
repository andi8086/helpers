#include "xmutex.h"
#include "../threads/x-atomic.h"
#include "../threads/x-threads.h"

#include <string.h>


void xmutex_init(xmutex_t *mtx)
{
        x_atomic_clear64(&mtx->lock);
}


void xmutex_lock(xmutex_t *mtx)
{
        while (x_atomic_test_set64(&mtx->lock)) {
#if !defined(X_MUTEX_NO_THREAD_YIELD) && defined(X_THREAD_SUPPORT)
                x_thread_yield();
#endif
        }
}


void xmutex_unlock(xmutex_t *mtx)
{
        x_atomic_clear64(&mtx->lock);
}


void xsig_init(xsig_t *signal)
{
        memset(signal, 0, sizeof(xsig_t));
}


void coherent_wait(xsig_t *signal, uint64_t m)
{
        while (1) {
                xmutex_lock(&signal->count_lock);
                if (signal->count > 0 && signal->msg == m) {
                        goto xsem_pend;
                }
                xmutex_unlock(&signal->count_lock);
#if !defined(X_MUTEX_NO_THREAD_YIELD) && defined(X_THREAD_SUPPORT)
                x_thread_yield();
#endif
        }
xsem_pend:
        signal->count--;
        xmutex_unlock(&signal->count_lock);
}


void coherent_signal(xsig_t *signal, uint64_t msg, uint64_t nconsumers)
{
        uint64_t res;
        /* first we wait until it is zero (all clients have confirmed), so that
         * a new barrier can be placed */
        do {
                res = x_atomic_load64(&signal->count);
                if (res != 0) {
#if defined(X_THREAD_SUPPORT) && !defined(X_SIGNAL_NO_THREAD_YIELD)
                        x_thread_yield();
#endif
                }
        } while (res != 0);

        xmutex_lock(&signal->count_lock);
        signal->count = nconsumers;
        signal->msg   = msg;
        xmutex_unlock(&signal->count_lock);

        do {
                res = x_atomic_load64(&signal->count);
                if (res != 0) {
#if defined(X_THREAD_SUPPORT) && !defined(X_SIGNAL_NO_THREAD_YIELD)
                        x_thread_yield();
#endif
                }
        } while (res != 0);
}
