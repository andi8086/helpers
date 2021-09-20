#ifndef X_MUTEX_H
#define X_MUTEX_H


/* be compatible with Linux errno.h */
#define X_MUTEX_EOK             0
#define X_MUTEX_EPERM           1
#define X_MUTEX_EAGAIN          11
#define X_MUTEX_ENOMEM          12
#define X_MUTEX_EBUSY           16
#define X_MUTEX_EINVAL          22
#define X_MUTEX_EDEADLK         45
#define X_MUTEX_EOWNERDEAD      130
#define X_MUTEX_ENOTRECOVERABLE 131
#define X_MUTEX_EUNKNOWN        66666

#ifdef _WIN32
#        include <windows.h>

typedef CRITICAL_SECTION x_mutex_t;

#        include "x_mutex_win32.h"

#        define x_mutex_create(m)  x_mutex_create_win32(m)
#        define x_mutex_destroy(m) x_mutex_destroy_win32(m)
#        define x_mutex_trylock(m) x_mutex_trylock_win32(m)
#        define x_mutex_lock(m)    x_mutex_lock_win32(m)
#        define x_mutex_unlock(m)  x_mutex_unlock_win32(m)

#elif defined(SOC_AM65XX)
#        include "../threads/x-atomic.h"

#        include <stdint.h>

typedef uint64_t x_mutex_t;

#        include "x_mutex_tirtos.h"

#        define x_mutex_create(m)  x_mutex_create_tirtos(m)
#        define x_mutex_destroy(m) x_mutex_destroy_tirtos(m)
#        define x_mutex_trylock(m) x_mutex_trylock_tirtos(m)
#        define x_mutex_lock(m)    x_mutex_lock_tirtos(m)
#        define x_mutex_unlock(m)  x_mutex_unlock_tirtos(m)

#else

#        include <pthread.h>

typedef pthread_mutex_t x_mutex_t;

#        include "x_mutex_posix.h"

#        define x_mutex_create(m)  x_mutex_create_posix(m)
#        define x_mutex_destroy(m) x_mutex_destroy_posix(m)
#        define x_mutex_trylock(m) x_mutex_trylock_posix(m)
#        define x_mutex_lock(m)    x_mutex_lock_posix(m)
#        define x_mutex_unlock(m)  x_mutex_unlock_posix(m)

#endif

#endif
