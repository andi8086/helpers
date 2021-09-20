#include "x-threads.h"


#ifdef X_THREADS_TIRTOS
static x_thread_t thread_create_tirtos(x_thread_func_t fxn,
                                       x_thread_params_t *p)
{
        Task_Params params;

        Task_Params_init(&params);
        Error_init(&(p->eb));

        params.priority  = p->priority;
        params.stackSize = p->stack_size;
        params.arg0      = p->arg0;
        params.arg1      = p->arg1;

        return (x_thread_t)Task_create(fxn, &params, &(p->eb));
}
#endif


#ifdef _WIN32
static x_thread_t thread_create_win32crt(x_thread_func_t fxn,
                                         x_thread_params_t *p)
{
        return (x_thread_t)_beginthread(fxn, 0, p);
}
#endif


#ifdef __gnu_linux__
static x_thread_t thread_create_pthread(x_thread_func_t fxn,
                                        x_thread_params_t *p)
{
        int s;
        pthread_t id;

        s = pthread_create(&id, NULL, fxn, p);

        if (s) {
                return 0;
        }

        return id;
}
#endif


x_thread_t x_thread_create(x_thread_func_t fxn, x_thread_params_t *p)
{
#if defined(X_THREADS_TIRTOS)
        return thread_create_tirtos(fxn, p);
#elif defined(_WIN32)
        return thread_create_win32crt(fxn, p);
#elif defined(__gnu_linux__)
        return thread_create_pthread(fxn, p);
#endif
}


void x_thread_wait_infinite(x_thread_t t)
{
#if defined(X_THREADS_TIRTOS)
        /* tasks are assumed to never end */
#elif defined(_WIN32)
        WaitForSingleObject(t, INFINITE);
#elif defined(__gnu_linux__)
        (void)pthread_join(t, NULL);
#endif
}

void x_thread_yield(void)
{
#if defined(X_THREADS_TIRTOS)
        Task_yield();
#elif defined(_WIN32)
        (void)SwitchToThread();
#elif defined(__gnu_linux__)
        (void)sched_yield();
#endif
}


void x_thread_kill(x_thread_t t)
{
#if defined(X_THREADS_TIRTOS)
        /* Not implemented */
#elif defined(_WIN32)
        TerminateThread(t, 0);
#elif defined(__gnu_linux__)
        pthread_cancel(t);
#endif
}
