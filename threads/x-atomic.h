#ifndef X_ATOMIC_H
#define X_ATOMIC_H

/* This header defines locking-free atomic operations needed in firmware and
 * libraries */

#ifdef _MSC_VER
#        include <windows.h>

#        define x_atomic_store64(A, B)     (void)InterlockedExchange64(A, B)
#        define x_atomic_load64(A)         InterlockedCompareExchange64(A, 0, 0)
#        define x_atomic_test_set64(A)     InterlockedCompareExchange64(A, 1, 0)
#        define x_atomic_clear64(A)        (void)InterlockedExchange64(A, 0)
#        define x_atomic_fetch_add64(A, B) InterlockedExchangeAdd64(A, B)
#        define x_atomic_fetch_sub64(A, B) InterlockedExchangeAdd64(A, -B)
#elif defined(__GNUC__)
#        ifdef __clang__
#                error("Compiler not supported")
#        endif

#        define x_atomic_store64(A, B) __atomic_store_n(A, B, __ATOMIC_RELEASE)
#        define x_atomic_load64(A)     __atomic_load_n(A, __ATOMIC_ACQUIRE)
#        define x_atomic_test_set64(A) \
                __atomic_test_and_set(A, __ATOMIC_ACQUIRE)
#        define x_atomic_clear64(A) __atomic_clear(A, __ATOMIC_RELEASE)
#        define x_atomic_fetch_add64(A, B) \
                __atomic_fetch_add(A, B, __ATOMIC_ACQ_REL)
#        define x_atomic_fetch_sub64(A, B) \
                __atomic_fetch_sub(A, B, __ATOMIC_ACQ_REL)
#else
#        error("Compiler not supported")
#endif

#endif
