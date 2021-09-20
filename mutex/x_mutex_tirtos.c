#include "x_mutex.h"
#include "../threads/x-threads.h"

#include <errno.h>


int x_mutex_create_tirtos(x_mutex_t *m)
{
        *m = 0;
        return X_MUTEX_EOK;
}


int x_mutex_destroy_tirtos(x_mutex_t *m)
{
        return X_MUTEX_EOK;
}


int x_mutex_trylock_tirtos(x_mutex_t *m)
{
        uint64_t res = x_atomic_load64(m);
        if (res == 0) {
                return X_MUTEX_EOK;
        }
        return X_MUTEX_EAGAIN;
}


int x_mutex_lock_tirtos(x_mutex_t *m)
{
        while (x_atomic_load64(m) == 1) {
                x_thread_yield();
        }
        x_atomic_store64(m, 1);
        return X_MUTEX_EOK;
}


int x_mutex_unlock_tirtos(x_mutex_t *m)
{
        x_atomic_store64(m, 0);
        return X_MUTEX_EOK;
}
