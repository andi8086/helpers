#ifndef HELPERS_PERF_H
#define HELPERS_PERF_H
/************************************************************************
 *                Performance measurement utilities
 *
 ************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include "../mutex/xmutex.h"

#define PERF_MAX_ID 1024

#if defined(_WIN32) || !defined(SOC_AM65XX)
#        define PERF_GET_CYCLES 0
#else
/* On AM65 plattform use the cpu cycle count stored in the PMCCNTR_EL0
   register.
   Important: this requires a prior initialization of these performance counters
   (see cycle_counter_init in cycles.h file*/
#        define CYCLES                                               \
                ({                                                   \
                        uint64_t rval = 0U;                          \
                        __asm__ __volatile__("mrs %0, PMCCNTR_EL0\n" \
                                             : "=r"(rval));          \
                        rval;                                        \
                })
#        define CORE_FREQ       0.8f
#        define PERF_GET_CYCLES CYCLES
#endif

#define PERF_START(PERF, ID) uint64_t cycles_start_##ID = PERF_GET_CYCLES

#define PERF_STOP(PERF, ID)                                        \
        if (PERF && ((PERF)->enabled_ids[(ID - 1) >> 6] &          \
                     (1ULL << ((((ID)&63)) - 1)))) {               \
                perf_create_entry((PERF), (ID), cycles_start_##ID, \
                                  PERF_GET_CYCLES);                \
        }

#define PERF_MARK(PERF, ID)                                      \
        if (PERF && ((PERF)->enabled_ids[(ID - 1) >> 6] &        \
                     (1ULL << (((ID)&63) - 1)))) {               \
                perf_create_entry((PERF), (ID), PERF_GET_CYCLES, \
                                  PERF_GET_CYCLES);              \
        }

#define PERF_USER(PERF, ID, USERDATA)                                 \
        if (PERF && ((PERF)->enabled_ids[(ID - 1) >> 6] &             \
                     (1ULL << (((ID)&63) - 1)))) {                    \
                perf_create_entry_user((PERF), (ID), PERF_GET_CYCLES, \
                                       USERDATA);                     \
        }


#pragma pack(push, 1)
typedef struct {
        uint64_t id;
        uint64_t time_stamp;
        uint64_t cycles;
} perf_info_t;

typedef struct perf {
        xmutex_t lock;
        perf_info_t *entries_base;
        perf_info_t *entries_current;
        perf_info_t *entries_max;
        uint64_t enabled_ids[16]; /*todo: make this atomic?*/
} perf_t;
#pragma pack(pop)


perf_t *perf_init(size_t num, bool enable_all_ids);
perf_t *perf_init_static(perf_t *perf, perf_info_t *entries, size_t num,
                         bool enable_all_ids);
void perf_free(perf_t *perf);

/* enables perf measurement for id. Not threadsafe! */
void perf_enable(perf_t *perf, uint16_t id);
/* disables perf measurement for id. Not threadsafe! */
void perf_disable(perf_t *perf, uint16_t id);
/* checks if perf measurement is enabled for id. Not threadsafe! */
bool perf_is_enabled(perf_t *perf, uint16_t id);

/* creates a perf_info_t entry in perf buffer. Stays at last if buffer is full!
   Note: Do not concurrently call perf_enable/disable! */
void perf_create_entry(perf_t *perf, uint16_t id, uint64_t start, uint64_t end);

/* creates a perf_info_t entry in perf buffer. Instead of end time, and time
 * difference, user data is logged */
void perf_create_entry_user(perf_t *perf, uint16_t id, uint64_t start,
                            uint64_t userdata);

#endif
