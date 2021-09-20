#ifdef __clang__
#        ifdef _MSC_VER
#                undef _MSC_VER
#        endif
#endif

#include <criterion/criterion.h>

#include "../ringbuffer.h"
#include "../../threads/x-threads.h"


Test(HELPERS_RINGBUFFER, ringbuffer_dynamic_malloc_free)
{
        void *buffer = h_ringbuff_alloc(4, 16, 0);

        cr_expect(buffer != NULL);

        h_ringbuff_free(buffer);
}


Test(HELPERS_RINGBUFFER, ringbuffer_dynamic_aligned_malloc_free)
{
        void *buffer = h_ringbuff_aligned_alloc(16, 4, 16, 0);

        cr_expect(buffer != NULL);

        /* Check alignment of memory */
        cr_expect(((uint64_t)buffer & 0xF) == 0);

        h_ringbuff_aligned_free(buffer);
}


Test(HELPERS_RINGBUFFER, ringbuffer_init)
{
        const int num_els = 16;

        void *buffer =
            h_ringbuff_alloc(4, num_els, sizeof(h_ringbuff_header_t));

        h_ringbuff_header_t *h = (h_ringbuff_header_t *)buffer;

        cr_expect(h->ring_size == 4 * (num_els + 1));
        cr_expect(h->header_size == sizeof(struct h_ringbuff_header));

        h_ringbuff_init(buffer);

        cr_expect(h->r == h->w);
        cr_expect(h->w == h->header_size);

        h_ringbuff_free(buffer);
}


Test(HELPERS_RINGBUFFER, ringbuffer_push_pop_avail)
{
        void *buffer = h_ringbuff_alloc(4, 16, 0);
        h_ringbuff_init(buffer);

        h_ringbuff_header_t *h = (h_ringbuff_header_t *)buffer;

        uint32_t i;

        for (i = 0; i < 16; i++) {
                cr_expect(h_ringbuff_avail(buffer) == (16 - i));
                cr_expect(h_ringbuff_push(buffer, &i, 4) == 0);
        }

        cr_expect(h_ringbuff_push(buffer, &i, 4) == 1);

        h_ringbuff_pop(buffer, 8);
        cr_expect(h_ringbuff_avail(buffer) == 2);

        h_ringbuff_free(buffer);
}


Test(HELPERS_RINGBUFFER, ringbuffer_full_empty)
{
        void *buffer = h_ringbuff_alloc(4, 16, 0);
        h_ringbuff_init(buffer);

        h_ringbuff_header_t *h = (h_ringbuff_header_t *)buffer;

        uint32_t data = 0;

        cr_expect(h_ringbuff_is_empty(buffer) == true);
        cr_expect(h_ringbuff_is_full(buffer) == false);

        cr_expect(h_ringbuff_push(buffer, &data, 4) == 0);

        cr_expect(h_ringbuff_is_empty(buffer) == false);
        cr_expect(h_ringbuff_is_full(buffer) == false);

        h_ringbuff_pop(buffer, 4);

        cr_expect(h_ringbuff_is_empty(buffer) == true);
        cr_expect(h_ringbuff_is_full(buffer) == false);

        for (int i = 0; i < 16; i++) {
                cr_expect(h_ringbuff_push(buffer, &data, 4) == 0);
        }

        cr_expect(h_ringbuff_is_empty(buffer) == false);
        cr_expect(h_ringbuff_is_full(buffer) == true);

        h_ringbuff_free(buffer);
}


Test(HELPERS_RINGBUFFER, ringbuffer_checkdata)
{
        const uint32_t nr_elems = 16;
        void *buffer = h_ringbuff_alloc(sizeof(uint32_t), nr_elems, 0);
        h_ringbuff_init(buffer);

        h_ringbuff_header_t *h = (h_ringbuff_header_t *)buffer;

        for (uint32_t j = 0; j < nr_elems; j++) {
                for (uint32_t i = 0; i < nr_elems; i++) {
                        uint32_t val = i + nr_elems * j;
                        cr_assert_eq(
                            h_ringbuff_push(buffer, &val, sizeof(uint32_t)), 0,
                            "Iteration %d\n, element %d\n, read_pointer: %llu, "
                            "write_pointer: %llu",
                            j, i, h->r, h->w);
                }

                for (uint32_t i = 0; i < nr_elems; i++) {
                        uint32_t *next = (uint32_t *)h_ringbuff_read(
                            buffer, sizeof(uint32_t));
                        cr_assert_not_null(next, "Next element is null");
                        cr_assert_eq(
                            *next, i + nr_elems * j,
                            "Read element does not match pushed element!");
                        h_ringbuff_pop(buffer, sizeof(uint32_t));
                }
        }

        h_ringbuff_free(buffer);
}


#define PRODUCER_COUNT 10000000


typedef struct context {
        void *buf;
        size_t count;
        size_t sum;
        bool stop;
} context_t;


THREAD_FUNC(producer)
{
        context_t *ctx = (context_t *)p;
        for (size_t i = 0; i < PRODUCER_COUNT; ++i) {
                uint32_t val = i;
                ctx->sum += i;
                while (h_ringbuff_push(ctx->buf, &val, sizeof(uint32_t)) != 0) {
                        x_thread_yield();
                }
                ++ctx->count;
        }
}


THREAD_FUNC(consumer)
{
        context_t *ctx = (context_t *)p;

        bool really_stop = false;
        while (!really_stop) {
                uint32_t *next =
                    (uint32_t *)h_ringbuff_read(ctx->buf, sizeof(uint32_t));

                if (!next) {
                        really_stop = ctx->stop;
                        x_thread_yield();
                        continue;
                }

                cr_assert_not_null(
                    next,
                    "Read element is null, even though buffer is not empty!");
                ctx->sum += *next;
                ++ctx->count;
                h_ringbuff_pop(ctx->buf, sizeof(uint32_t));
        }
}


Test(HELPERS_RINGBUFFER, ringbuffer_concurrent)
{
        void *buffer = h_ringbuff_aligned_alloc(64, sizeof(uint32_t), 16, 0);
        h_ringbuff_init(buffer);

        context_t pctx = {.buf = buffer, .count = 0, .sum = 0, .stop = false};
        x_thread_t prodthr = x_thread_create(producer, &pctx);
        cr_assert_not_null(prodthr, "producer thread creation failed");

        context_t cctx = {.buf = buffer, .count = 0, .sum = 0, .stop = false};
        x_thread_t consthr = x_thread_create(consumer, &cctx);
        cr_assert_not_null(consthr, "consumer thread creation failed");

        x_thread_wait_infinite(prodthr);

        cctx.stop = true;

        x_thread_wait_infinite(consthr);

        x_thread_kill(prodthr);
        x_thread_kill(consthr);

        cr_expect(h_ringbuff_is_empty(buffer),
                  "Ringbuffer is not empty! (still has %i elem)",
                  16 - h_ringbuff_avail(buffer));

        cr_assert_eq(cctx.count, pctx.count,
                     "Element count is incorrect (%i of %i)", cctx.count,
                     pctx.count);

        cr_assert_eq(cctx.sum, pctx.sum, "Element sum is incorrect(%i of %i) ",
                     cctx.sum, pctx.sum);

        h_ringbuff_aligned_free(buffer);
}


typedef struct test_struct {
        uint64_t id;
} test_struct_t;

typedef struct list_element {
        h_ringbuff_dynl_hdr_t head;
        test_struct_t ptr;
} list_element_t;

Test(HELPERS_RINGBUFFER, ringbuffer_dynlist_enable)
{
        h_ringbuff_header_t *buffer =
            h_ringbuff_alloc(sizeof(list_element_t), 5, 0);
        cr_expect(buffer != NULL);
        h_ringbuff_init(buffer);

        cr_expect(!buffer->dynlist_enabled);
        h_ringbuff_set_dynlist_support(buffer, true);
        cr_expect(buffer->dynlist_enabled);

        h_ringbuff_free(buffer);
}

Test(HELPERS_RINGBUFFER, ringbuffer_dynlist_insert_before)
{
        void *buffer = h_ringbuff_alloc(sizeof(list_element_t), 5, 0);
        cr_expect(buffer != NULL);
        h_ringbuff_init(buffer);

        h_ringbuff_set_dynlist_support(buffer, true);

        list_element_t *a   = NULL;
        list_element_t elem = {.head = {0}, .ptr = {.id = 1}};
        a                   = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(a != NULL);
        elem.ptr.id       = 3;
        list_element_t *c = NULL;
        c                 = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(c != NULL);
        elem.ptr.id       = 2;
        list_element_t *b = NULL;
        b                 = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(b != NULL);
        /*Order important here, to get the next/prev pointers right!
         1. insert a before b
         2. link b with c
        */
        h_ringbuff_dynlist_insert_before(buffer, &c->head, &b->head);
        h_ringbuff_dynlist_insert_before(buffer, &b->head, &a->head);

        h_ringbuff_dynl_hdr_t *first = b->head.prev;
        cr_expect(first != NULL);
        cr_assert_eq(((list_element_t *)first)->ptr.id, 1);
        h_ringbuff_dynl_hdr_t *second = a->head.next;
        cr_expect(second != NULL);
        cr_assert_eq(((list_element_t *)second)->ptr.id, 2);
        h_ringbuff_dynl_hdr_t *third = second->next;
        cr_expect(third != NULL);
        cr_assert_eq(((list_element_t *)third)->ptr.id, 3);

        h_ringbuff_free(buffer);
}

Test(HELPERS_RINGBUFFER, ringbuffer_dynlist_insert_after)
{
        void *buffer = h_ringbuff_alloc(sizeof(list_element_t), 5, 0);
        cr_expect(buffer != NULL);
        h_ringbuff_init(buffer);

        h_ringbuff_set_dynlist_support(buffer, true);

        list_element_t *a   = NULL;
        list_element_t elem = {.head = {0}, .ptr = {.id = 1}};
        a                   = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(a != NULL);
        elem.ptr.id       = 3;
        list_element_t *c = NULL;
        c                 = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(c != NULL);
        elem.ptr.id       = 2;
        list_element_t *b = NULL;
        b                 = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(b != NULL);

        h_ringbuff_dynlist_insert_after(buffer, &a->head, &b->head);
        h_ringbuff_dynlist_insert_after(buffer, &b->head, &c->head);

        h_ringbuff_dynl_hdr_t *first = b->head.prev;
        cr_expect(first != NULL);
        cr_assert_eq(((list_element_t *)first)->ptr.id, 1);
        h_ringbuff_dynl_hdr_t *second = a->head.next;
        cr_expect(second != NULL);
        cr_assert_eq(((list_element_t *)second)->ptr.id, 2);
        h_ringbuff_dynl_hdr_t *third = second->next;
        cr_expect(third != NULL);
        cr_assert_eq(((list_element_t *)third)->ptr.id, 3);

        h_ringbuff_free(buffer);
}

Test(HELPERS_RINGBUFFER, ringbuffer_dynlist_remove)
{
        void *buffer = h_ringbuff_alloc(sizeof(list_element_t), 5, 0);
        cr_expect(buffer != NULL);
        h_ringbuff_init(buffer);

        h_ringbuff_set_dynlist_support(buffer, true);

        list_element_t *a   = NULL;
        list_element_t elem = {.head = {0}, .ptr = {.id = 1}};
        a                   = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(a != NULL);
        elem.ptr.id       = 3;
        list_element_t *c = NULL;
        c                 = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(c != NULL);
        elem.ptr.id       = 2;
        list_element_t *b = NULL;
        b                 = h_ringbuff_push_ref(buffer, &elem);
        cr_expect(b != NULL);

        h_ringbuff_dynlist_insert_after(buffer, &a->head, &b->head);
        h_ringbuff_dynlist_insert_after(buffer, &b->head, &c->head);


        h_ringbuff_dynlist_unlink(buffer, &a->head);
        cr_expect(a->head.ref_count == 0);
        cr_expect(a->head.next == NULL);
        cr_expect(a->head.prev == NULL);

        list_element_t *next = h_ringbuff_read(buffer, 0);
        cr_expect(a == next);

        h_ringbuff_pop(buffer, sizeof(list_element_t));

        cr_assert_eq(b->head.ref_count, 1);
        cr_expect(b->head.prev == NULL);
        h_ringbuff_dynlist_unlink(buffer, &b->head);
        cr_expect(b->head.ref_count == 0);
        cr_expect(b->head.next == NULL);
        cr_expect(b->head.prev == NULL);


        h_ringbuff_dynlist_unlink(buffer, &c->head);
        cr_expect(c->head.ref_count == 0);
        cr_expect(c->head.next == NULL);
        cr_expect(c->head.prev == NULL);

        next = h_ringbuff_read(buffer, 0);
        cr_assert_eq(next->head.ref_count, 0);

        h_ringbuff_pop(buffer, sizeof(list_element_t));
        next = h_ringbuff_read(buffer, 0);
        cr_assert_eq(next->head.ref_count, 0);

        h_ringbuff_pop(buffer, sizeof(list_element_t));

        h_ringbuff_free(buffer);
}
