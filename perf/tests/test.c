#define _POSIX_C_SOURCE 199309L
#include "../perf.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <time.h>
#ifdef _WIN32
#        include <Windows.h>
#endif

uint64_t get_time_stamp()
{
#ifdef _WIN32
        LARGE_INTEGER frequency;
        LARGE_INTEGER count;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&count);
        return count.QuadPart * 1000 / frequency.QuadPart;
#else
        struct timespec t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        return (uint64_t)(t.tv_sec) * (uint64_t)1000000000 +
               (uint64_t)(t.tv_nsec);
#endif
}

perf_t *test_perf = NULL;

#undef PERF_GET_CYCLES
#define PERF_GET_CYCLES get_time_stamp()

void test_function(perf_t *perf)
{
        PERF_START(test_perf, 1);

        double val = 0.0;
        for (int i = 0; i < 2; ++i) {
                val += tanh(val + 0.1) * log(tanh(sin(val)));

                PERF_START(test_perf, 2);
                for (int j = 0; j < 10000000; ++j) {
                        val += tanh(val + 0.1) * log(tanh(sin(val)));
                }
                PERF_STOP(test_perf, 2);
        }
        PERF_STOP(test_perf, 1);
}

int main(void)
{
        printf("------ Testing perf util ------\n");
        perf_t *perf = perf_init(1000, false);

        if (!perf) {
                printf("perf init failed!\n");
                return 1;
        }

        perf_info_t buf[1000];
        perf_t perf2;
        if (!perf_init_static(&perf2, buf, 1000, false)) {
                printf("perf static init failed!\n");
                return 2;
        }

        int res = EXIT_SUCCESS;

        if (perf_is_enabled(perf, 1)) {
                printf("perf id=1 already enabled!\n");
                res = 3;
                goto exit;
        }

        perf_enable(perf, 1);

        if (!perf_is_enabled(perf, 1)) {
                printf("perf id=1 not enabled!\n");
                res = 4;
                goto exit;
        }

        perf_disable(perf, 1);

        if (perf_is_enabled(perf, 1)) {
                printf("perf id=1 still enabled!\n");
                res = 5;
                goto exit;
        }

        test_perf = perf;

        perf_enable(perf, 1);
        perf_enable(perf, 2);

        for (uint64_t i = 0; i < 10; ++i) {
                test_function(perf);
        }

        perf_disable(perf, 2);

        for (uint64_t i = 0; i < 10; ++i) {
                test_function(perf);
        }

        perf_info_t *first = perf->entries_base;

        uint64_t num_entries = perf->entries_current - perf->entries_base;

        for (uint64_t i = 0; i < num_entries; ++i) {
                printf("Entry %lli: id=%lli, ts=%lli cycles=%lli\n", i,
                       first[i].id, first[i].time_stamp, first[i].cycles);
        }
exit:
        perf_free(perf);

        printf("------ Testing perf util done ------\n");

        return res;
}
