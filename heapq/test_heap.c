#include <criterion/criterion.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "../slx_common.h"

#include "heap.h"

bool uintptr_is_lt(void *a, void *b)
{
        return (uintptr_t)a < (uintptr_t)b;
}

Test(HEAP, main_test)
{
        /* first element is the size, i.e. 16 elements */
        uintptr_t buffer[16] = {17, 1,  15, 13, 22, 69,  9,    7,
                                6,  33, 12, 2,  8,  333, 1000, 88};

        for (int i = 0; i < 16; i++) {
                cr_log_info("buffer[%d] = %" PRIuPTR " ", i, buffer[i]);
        }

        heapq_t *h = malloc(sizeof(heapq_t));
        h          = heapq_heapify(h, (void **)buffer, 16, uintptr_is_lt);

        for (int i = 0; i < 16; i++) {
                cr_log_info("h->elements[%d] = %" PRIuPTR " ", i,
                            (uintptr_t)h->elements[i]);
        }

        for (int i = 0; i < 18; i++) {
                cr_log_info("popped #%d: %" PRIuPTR " ", i,
                            (uintptr_t)heapq_pop(h));
        }
        free(h);
}
