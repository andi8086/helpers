#include "perf.h"

#include <stdlib.h>
#include <string.h>


static perf_t *perf_init_common(perf_t *perf, size_t num, bool enable_all_ids)
{

        perf->entries_current = perf->entries_base;
        perf->entries_max     = perf->entries_base + num;
        for (int i = 0; i < 16; i++) {
                perf->enabled_ids[i] = enable_all_ids ? UINT64_MAX : 0;
        }
        xmutex_init(&perf->lock);

        return perf;
}


perf_t *perf_init(size_t num, bool enable_all_ids)
{
        perf_t *perf = (perf_t *)malloc(sizeof(perf_t));
        if (!perf) {
                return NULL;
        }

        perf->entries_base = (perf_info_t *)malloc(sizeof(perf_info_t) * num);
        if (!perf->entries_base) {
                free(perf);
                return NULL;
        }

        return perf_init_common(perf, num, enable_all_ids);
}


perf_t *perf_init_static(perf_t *perf, perf_info_t *entries, size_t num,
                         bool enable_all_ids)
{
        perf->entries_base = entries;

        return perf_init_common(perf, num, enable_all_ids);
}


void perf_free(perf_t *perf)
{
        free(perf->entries_base);
        free(perf);
}


void perf_create_entry(perf_t *perf, uint16_t id, uint64_t start, uint64_t end)
{
        if (!perf_is_enabled(perf, id)) {
                return;
        }

        xmutex_lock(&perf->lock);

        perf->entries_current->time_stamp = end;
        perf->entries_current->id         = id;
        perf->entries_current->cycles     = end - start;

        perf->entries_current++;

        if (perf->entries_current >= perf->entries_max) {
                perf->entries_current = perf->entries_max - 1;
        }
        xmutex_unlock(&perf->lock);
}


void perf_create_entry_user(perf_t *perf, uint16_t id, uint64_t start,
                            uint64_t userdata)
{
        if (!perf_is_enabled(perf, id)) {
                return;
        }

        xmutex_lock(&perf->lock);

        perf->entries_current->time_stamp = start;
        perf->entries_current->id         = id;
        perf->entries_current->cycles     = userdata;

        perf->entries_current++;

        if (perf->entries_current >= perf->entries_max) {
                perf->entries_current = perf->entries_max - 1;
        }
        xmutex_unlock(&perf->lock);
}


void perf_enable(perf_t *perf, uint16_t id)
{
        if (id > PERF_MAX_ID) {
                return;
        }

        perf->enabled_ids[(id - 1) >> 6] |= (1ULL << ((id - 1) & 63));
}


void perf_disable(perf_t *perf, uint16_t id)
{
        if (id > PERF_MAX_ID) {
                return;
        }

        perf->enabled_ids[(id - 1) >> 6] &= ~(1ULL << ((id - 1) & 63));
}


bool perf_is_enabled(perf_t *perf, uint16_t id)
{
        if (!perf) {
                return false;
        }
        /* No extra check for id < PERF_MAX_ID,
           if id > 64 below is always false anyway
        */
        return perf->enabled_ids[(id - 1) >> 6] & (1ULL << ((id - 1) & 63));
}
