#include "x_mutex.h"


int x_mutex_create_win32(x_mutex_t *m)
{
        InitializeCriticalSection(m);
        return X_MUTEX_EOK;
}


int x_mutex_destroy_win32(x_mutex_t *m)
{
        DeleteCriticalSection(m);
        return X_MUTEX_EOK;
}


int x_mutex_trylock_win32(x_mutex_t *m)
{
        if (TryEnterCriticalSection(m)) {
                return X_MUTEX_EOK;
        } else {
                return X_MUTEX_EPERM;
        }
}


int x_mutex_lock_win32(x_mutex_t *m)
{
        EnterCriticalSection(m);
        return X_MUTEX_EOK;
}


int x_mutex_unlock_win32(x_mutex_t *m)
{
        LeaveCriticalSection(m);
        return X_MUTEX_EOK;
}
