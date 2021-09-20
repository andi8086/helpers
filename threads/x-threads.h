#ifndef X_THREADS_H
#define X_THREADS_H

#ifndef X_THREAD_SUPPORT
#        define X_THREAD_SUPPORT
#endif

#if defined(X_THREADS_TIRTOS)

#        include <stdint.h>
/* Something strange is going on with headers.. so suddenly we need all these
 * here */
typedef uint8_t xdc_UInt8;
typedef int8_t xdc_Int8;
typedef int16_t xdc_Int16;
typedef int32_t xdc_Int32;
typedef intptr_t xdc_IArg;
typedef uintptr_t xdc_UArg;
typedef uint32_t xdc_UInt32;
typedef uint16_t xdc_UInt16;
typedef uint16_t xdc_Bits16;
typedef uint32_t xdc_Bits32;
// #       include <ti/csl/csl_error.h>
// #        include <targets/arm/std.h>
#        include <ti/sysbios/BIOS.h>
#        include <xdc/runtime/Error.h>
#        include <ti/sysbios/knl/Task.h>

/* We handle threads as TIRTOS tasks */
typedef void (*x_thread_func_t)(UArg arg0, UArg arg1);
typedef Task_Handle x_thread_t; /* the task handle */
typedef struct x_thread_params {
        UArg arg0;
        UArg arg1;
        int stack_size;
        int priority;
        Error_Block eb;
} x_thread_params_t;

#elif defined(_WIN32)

#        include <windows.h>
#        include <process.h>

typedef void (*x_thread_func_t)(void *);
typedef HANDLE x_thread_t; /* the thread handle */
typedef void x_thread_params_t;

#elif defined(__gnu_linux__)

#        include <pthread.h>

typedef void *(*x_thread_func_t)(void *);
typedef pthread_t x_thread_t; /* the pthread ID */
typedef void x_thread_params_t;

#else
#        pragma message "Threading not supported for your platform\n"
#        undef X_THREAD_SUPPORT
#endif


#ifdef X_THREAD_SUPPORT
x_thread_t x_thread_create(x_thread_func_t fxn, x_thread_params_t *p);
void x_thread_wait_infinite(x_thread_t t);
void x_thread_yield(void);
void x_thread_kill(x_thread_t t);
#endif


#ifdef X_THREADS_TIRTOS
#        define X_THREAD_FUNC(name) static void name(UArg a0, UArg a1)
#elif defined(_WIN32)
#        define X_THREAD_FUNC(name) static void name(void *p)
#elif defined(__gnu_linux__)
#        define X_THREAD_FUNC(name) static void *name(void *p)
#endif


#endif
