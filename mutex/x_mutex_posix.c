#include "x_mutex.h"

#include <errno.h>


int x_mutex_create_posix(x_mutex_t *m)
{
        int res = pthread_mutex_init(m, NULL);
        switch (res) {
        case X_MUTEX_EOK: return X_MUTEX_EOK;
        case EAGAIN: return X_MUTEX_EAGAIN;
        case ENOMEM: return X_MUTEX_ENOMEM;
        case EPERM: return X_MUTEX_EPERM;
        default: return X_MUTEX_EUNKNOWN;
        }
        return X_MUTEX_EUNKNOWN;
}


int x_mutex_destroy_posix(x_mutex_t *m)
{
        int res = pthread_mutex_destroy(m);
        switch (res) {
        case X_MUTEX_EOK: return X_MUTEX_EOK;
        case EBUSY: return X_MUTEX_EBUSY;
        case EINVAL: return X_MUTEX_EINVAL;
        default: return X_MUTEX_EUNKNOWN;
        }
        return X_MUTEX_EUNKNOWN;
}


int x_mutex_trylock_posix(x_mutex_t *m)
{
        int res = pthread_mutex_trylock(m);
        switch (res) {
        case X_MUTEX_EOK: return X_MUTEX_EOK;
        case EAGAIN: return X_MUTEX_EAGAIN;
        case EINVAL: return X_MUTEX_EINVAL;
        case ENOTRECOVERABLE: return X_MUTEX_ENOTRECOVERABLE;
        case EOWNERDEAD: return X_MUTEX_EOWNERDEAD;
        case EBUSY: return X_MUTEX_EBUSY;
        default: return X_MUTEX_EUNKNOWN;
        }
        return X_MUTEX_EUNKNOWN;
}


int x_mutex_lock_posix(x_mutex_t *m)
{
        int res = pthread_mutex_lock(m);
        switch (res) {
        case X_MUTEX_EOK: return X_MUTEX_EOK;
        case EAGAIN: return X_MUTEX_EAGAIN;
        case EINVAL: return X_MUTEX_EINVAL;
        case ENOTRECOVERABLE: return X_MUTEX_ENOTRECOVERABLE;
        case EOWNERDEAD: return X_MUTEX_EOWNERDEAD;
        case EDEADLK: return X_MUTEX_EDEADLK;
        default: return X_MUTEX_EUNKNOWN;
        }
        return X_MUTEX_EUNKNOWN;
}


int x_mutex_unlock_posix(x_mutex_t *m)
{
        int res = pthread_mutex_unlock(m);
        switch (res) {
        case X_MUTEX_EOK: return X_MUTEX_EOK;
        case EPERM: return X_MUTEX_EPERM;
        default: return X_MUTEX_EUNKNOWN;
        }
        return X_MUTEX_EUNKNOWN;
}
