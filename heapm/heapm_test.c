#include "heapm.h"

#include <stdlib.h>
#include <inttypes.h>

static void shuffle(int *array, size_t n)
{
        if (n <= 1 || !array) {
                return;
        }
        size_t i;
        for (i = 0; i < n - 1; i++) {
                size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
                int t    = array[j];
                array[j] = array[i];
                array[i] = t;
        }
}


uint8_t static_mem[1048576];

#define STRINGIFY(x) STRFY(x)
#define STRFY(x)     #x

#define TEST_CHECK(x)                                             \
        {                                                         \
                int line = __LINE__;                              \
                if (!(x)) {                                       \
                        printf("Error: '%s' failed in line %d\n", \
                               STRINGIFY(x), line);               \
                        return 1;                                 \
                }                                                 \
        }


int main(void)
{
        hm_ctx_t ctx;

        printf(
            "******************** standard sequence test ******************\n");
        hm_init(&ctx, static_mem, sizeof(static_mem));

        uint64_t all_free = hm_available(&ctx, false);

        TEST_CHECK(all_free == ctx.mem_size - sizeof(hm_pfx_t));

        void *p1 = hm_alloc(&ctx, 131072);

        all_free = hm_available(&ctx, false);
        TEST_CHECK(all_free == ctx.mem_size - 131072 - 2 * sizeof(hm_pfx_t));

        hm_free(&ctx, p1);

        void *p2, *p3, *p4, *p5;
        p1 = hm_alloc(&ctx, 65536);
        p2 = hm_alloc(&ctx, 65536);
        p3 = hm_alloc(&ctx, 65536);

        TEST_CHECK(hm_allocated(&ctx) == 4 * sizeof(hm_pfx_t) + 3 * 65536);

        hm_free(&ctx, p1);
        hm_free(&ctx, p3);
        hm_free(&ctx, p2);

        all_free = hm_available(&ctx, false);
        TEST_CHECK(all_free == ctx.mem_size - sizeof(hm_pfx_t));

        p1 = hm_alloc(&ctx, 65536);
        p2 = hm_alloc(&ctx, 65536);
        p3 = hm_alloc(&ctx, 65536);
        p4 = hm_alloc(&ctx, 65536);
        p5 = hm_alloc(&ctx, 65536);

        TEST_CHECK(hm_allocated(&ctx) == 6 * sizeof(hm_pfx_t) + 5 * 65536);

        hm_free(&ctx, p3);
        hm_free(&ctx, p4);

        p3 = hm_alloc(&ctx, 131072);

        TEST_CHECK(hm_allocated(&ctx) ==
                   5 * sizeof(hm_pfx_t) + 3 * 65536 + 131072);

        hm_free(&ctx, p1);
        hm_free(&ctx, p5);
        hm_free(&ctx, p2);
        hm_free(&ctx, p3);

        TEST_CHECK(all_free == ctx.mem_size - sizeof(hm_pfx_t));

        printf("\nPASSED\n");

        void *p[6];
        int order[] = {0, 1, 2, 3, 4, 5};

        const int n_allocs = 6;
        uint64_t calc, tree;

        printf(
            "***************** random size, random seq test ***************\n");
        for (int i = 0; i < 1080; i++) {
                size_t total_req = 0;
                for (int j = 0; j < n_allocs; j++) {
                        size_t req = rand() % 100000;
                        p[j]       = hm_alloc(&ctx, req);
                        total_req += req;
                        if (!p[j]) {
                                show_mem(&ctx);
                                printf("error, could not allocate\n");
                                return 1;
                        }
                }

                calc = total_req + (n_allocs + 1) * sizeof(hm_pfx_t);
                tree = hm_allocated(&ctx);
                TEST_CHECK(calc == tree);

                calc = ctx.mem_size - total_req -
                       (n_allocs + 1) * sizeof(hm_pfx_t);
                tree = hm_available(&ctx, false);
                TEST_CHECK(calc == tree);

                shuffle(order, 6);
                for (int j = 0; j < 6; j++) {
                        hm_free(&ctx, p[order[j]]);
                }
        }
        printf("\nPASSED\n");

        printf(
            "***************** equal size, random seq test ****************\n");
        for (int i = 0; i < 1080; i++) {
                size_t total_req = 0;
                size_t req       = rand() % 100000;
                for (int j = 0; j < 6; j++) {
                        p[j] = hm_alloc(&ctx, req);
                        total_req += req;
                        if (!p[j]) {
                                show_mem(&ctx);
                                printf("error, could not allocate\n");
                                return 1;
                        }
                }
                calc = total_req + (n_allocs + 1) * sizeof(hm_pfx_t);
                tree = hm_allocated(&ctx);
                TEST_CHECK(calc == tree);

                calc = ctx.mem_size - total_req -
                       (n_allocs + 1) * sizeof(hm_pfx_t);
                tree = hm_available(&ctx, false);
                TEST_CHECK(calc == tree);

                shuffle(order, 6);
                for (int j = 0; j < 6; j++) {
                        hm_free(&ctx, p[order[j]]);
                }
        }

        printf("\nPASSED\n");

        printf(
            "********************* aligned alloc test *********************\n");

        for (int i = 0; i < 6; i++) {
                p[i] = hm_aligned_alloc(&ctx, 65536, 8);
                TEST_CHECK(((uintptr_t)p[i] & 7) == 0);
        }

        for (int i = 0; i < 6; i++) {
                hm_free(&ctx, p[i]);
        }
        printf("\nPASSED\n");

        p[0] = hm_alloc(&ctx, 65536);

        printf(
            "******************** malloc line store test ******************\n");

        /* make sure, the preceeding alloc is on the line tested below :D */
        TEST_CHECK(((hm_pfx_t *)p[0] - 1)->malloc_line == 176);

        printf("\nPASSED\n");

        printf(
            "****************** malloc out of memory test *****************\n");

        TEST_CHECK(hm_alloc(&ctx, 2019502) == NULL);

        printf("\nPASSED\n");

        printf(
            "******************** malloc max block test *******************\n");

        uint64_t max_block = hm_max(&ctx);
        void *p_max        = hm_alloc(&ctx, max_block);
        TEST_CHECK(p_max != NULL);

        TEST_CHECK(hm_available(&ctx, true) == 0);
        TEST_CHECK(hm_available(&ctx, false) == 0);
        TEST_CHECK(hm_max(&ctx) == 0);

        hm_free(&ctx, p_max);

        TEST_CHECK(hm_max(&ctx) == max_block);

        printf("\nPASSED\n");

        printf(
            "****************** general memory footprint ******************\n");

        printf("Root structure:       %" PRIu64 "\n", sizeof(hm_pfx_t));
        printf("Per allocated block:  %" PRIu64 "\n", sizeof(hm_pfx_t));

        return 0;
}
