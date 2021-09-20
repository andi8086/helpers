#include "ringbuffer.h"

#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#        include <malloc.h>
#        include <windows.h>
#endif
#include <stdio.h>

#include "../threads/x-atomic.h"


void h_ringbuff_set_push_hook(h_ringbuff_header_t *h, ring_push_hook_t hook)
{
        h->push_hook = hook;
}


void h_ringbuff_init(void *buffer)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        if (!rbh) {
                return;
        }

        uint64_t header_size = rbh->header_size;
        if (!header_size) {
                header_size = sizeof(struct h_ringbuff_header);
        }

        uint64_t ring_size = rbh->ring_size;
        uint64_t el_size   = rbh->size_el;

        memset(buffer, 0, header_size);
        rbh->header_size  = header_size;
        rbh->ring_size    = ring_size;
        rbh->looping_read = false;
        rbh->w            = header_size;
        rbh->r            = header_size;
        rbh->size_el      = el_size;
        rbh->push_hook    = NULL;
}


void h_ringbuff_set_read_loop(void *buffer, bool loop)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        if (!rbh) {
                return;
        }

        rbh->looping_read = loop;
}


size_t h_ringbuff_alloc_size(uint64_t size_el, uint64_t num_el,
                             uint64_t header_size)
{
        uint64_t act_ring_size = size_el * (num_el + 1);

        if (header_size == 0) {
                header_size = sizeof(struct h_ringbuff_header);
        }

        return act_ring_size + header_size;
}


void h_ringbuff_size_init(h_ringbuff_header_t *rbh, uint64_t size_el,
                          uint64_t num_el, uint64_t header_size)
{
        rbh->header_size = header_size;
        rbh->ring_size   = size_el * (num_el + 1);
        rbh->size_el     = size_el;
}


void *h_ringbuff_alloc(uint64_t size_el, uint64_t num_el, uint64_t header_size)
{
        void *buff =
            malloc(h_ringbuff_alloc_size(size_el, num_el, header_size));

        if (!buff) {
                return buff;
        }

        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buff;
        h_ringbuff_size_init(rbh, size_el, num_el, header_size);

        return buff;
}


void *h_ringbuff_aligned_alloc(size_t alignment, uint64_t size_el,
                               uint64_t num_el, uint64_t header_size)
{
        void *buff;
#ifdef _WIN32
        buff = _aligned_malloc(
            h_ringbuff_alloc_size(size_el, num_el, header_size), alignment);
#elif defined(SOC_AM65XX)
        return NULL;
#else
        buff = aligned_alloc(
            alignment, h_ringbuff_alloc_size(size_el, num_el, header_size));
#endif

        if (!buff) {
                return buff;
        }

        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buff;
        h_ringbuff_size_init(rbh, size_el, num_el, header_size);

        return buff;
}


void h_ringbuff_aligned_free(void *buffer)
{
#ifdef _WIN32
        _aligned_free(buffer);
#elif defined(SOC_AM65XX)
        return;
#else
        free(buffer);
#endif
}


void h_ringbuff_free(void *buffer)
{
        free(buffer);
}


uint64_t h_ringbuff_align_size(void *buffer, uint64_t size_el)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        if (rbh->ring_size % size_el) {
                /* ring_size / size_el rounds towards zero, for positive
                 * numbers only still implementation independent, for
                 * negative numbers we would have a mess */
                rbh->ring_size = size_el * (rbh->ring_size / size_el);
        }

        return rbh->ring_size;
}


uint64_t h_ringbuff_avail(void *buffer)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        uint64_t cur_r = x_atomic_load64(&rbh->r);
        uint64_t cur_w = x_atomic_load64(&rbh->w);

        return ((cur_r - cur_w - rbh->size_el + rbh->ring_size) %
                rbh->ring_size) /
               rbh->size_el;
}


uint64_t h_ringbuff_push(void *buffer, void *data, uint64_t size)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        uint64_t old_w = rbh->w;
        uint64_t new_w = (old_w + size - rbh->header_size);


        if (new_w >= rbh->ring_size) {
                new_w = 0;
        }

        new_w += rbh->header_size;

        if (new_w == x_atomic_load64(&rbh->r)) {
                return 1;
        }

        memcpy((char *)buffer + old_w, data, size);

        x_atomic_store64(&(rbh->w), new_w);

        if (rbh->push_hook) {
                rbh->push_hook((uint8_t *)data, (size_t)size);
        }
        return 0;
}

void *h_ringbuff_push_ref(void *buffer, void *data)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        uint64_t old_w = rbh->w;
        uint64_t new_w = (old_w + rbh->size_el - rbh->header_size);


        if (new_w >= rbh->ring_size) {
                new_w = 0;
        }

        new_w += rbh->header_size;

        if (new_w == x_atomic_load64(&rbh->r)) {
                return NULL;
        }

        void *ptr_new_elem = memcpy((char *)buffer + old_w, data, rbh->size_el);

        x_atomic_store64(&(rbh->w), new_w);

        if (rbh->push_hook) {
                rbh->push_hook((uint8_t *)data, (size_t)rbh->size_el);
        }
        return ptr_new_elem;
}


void *h_ringbuff_read(void *buffer, uint64_t size)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        uint64_t cur_w = x_atomic_load64(&rbh->w);
        if (cur_w == rbh->r) {
                return NULL;
        }
        return (char *)buffer + rbh->r;
}


int h_ringbuff_pop(void *buffer, uint64_t size)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        uint64_t cur_w = x_atomic_load64(&rbh->w);

        /* if we have a non-read-looping ringbuffer, we use num_elements + 1 for
         * popping, whereas pushing is limited to num_elements, since the buffer
         * is full if w = r - 1    mod  size
         *
         * if we have a circular ringbuffer, we use num_elements for popping,
         *      since we must return to element #0 after reaching element
         * #(num_elements - 1)
         *
         */

        if (cur_w == rbh->r) {
                return RINGBUFF_EMPTY;
        }

        /* check if the element pointed to by the read pointer is referenced */
        if (rbh->dynlist_enabled) {
                h_ringbuff_dynl_hdr_t *dlhdr =
                    (h_ringbuff_dynl_hdr_t *)h_ringbuff_read(rbh, rbh->size_el);
                if (dlhdr && x_atomic_load64(&dlhdr->ref_count)) {
                        /* not allowed to fetch, ref_count > 0 */
                        return RINGBUFF_REFERENCED;
                }
        }

        uint64_t wrap_limit = rbh->ring_size;

        if (rbh->looping_read) {
                wrap_limit -= rbh->size_el;
        }

        uint64_t new_r =
            (rbh->r + size - rbh->header_size) % wrap_limit + rbh->header_size;

        x_atomic_store64(&rbh->r, new_r);

        return RINGBUFF_OK;
}


bool h_ringbuff_is_empty(void *buffer)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        return x_atomic_load64(&rbh->w) == x_atomic_load64(&rbh->r);
}


bool h_ringbuff_is_full(void *buffer)
{
        return h_ringbuff_avail(buffer) == 0;
}


void h_ringbuff_set_dynlist_support(void *buffer, bool enable)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;
        rbh->dynlist_enabled     = enable;
}


void h_ringbuff_dynlist_insert_after(void *buffer, h_ringbuff_dynl_hdr_t *where,
                                     h_ringbuff_dynl_hdr_t *what)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;
        if (!rbh->dynlist_enabled) {
                return;
        }

        x_atomic_fetch_add64(&what->ref_count, 1);
        x_atomic_fetch_add64(&where->ref_count, 1);
        what->next  = where->next;
        where->next = what;
        what->prev  = where;
}


void h_ringbuff_dynlist_insert_before(void *buffer,
                                      h_ringbuff_dynl_hdr_t *where,
                                      h_ringbuff_dynl_hdr_t *what)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;
        if (!rbh->dynlist_enabled) {
                return;
        }

        x_atomic_fetch_add64(&what->ref_count, 1);
        x_atomic_fetch_add64(&where->ref_count, 1);

        what->prev  = where->prev;
        where->prev = what;
        what->next  = where;
}


static uint64_t h_ringbuff_dynlist_refdec(void *buffer,
                                          h_ringbuff_dynl_hdr_t *what)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;

        if (!rbh->dynlist_enabled) {
                return 0;
        }

        if (!x_atomic_load64(&what->ref_count)) {
                return 0;
        }

        /* return new value after decrement */
        return x_atomic_fetch_sub64(&what->ref_count, 1) - 1;
}


void h_ringbuff_dynlist_unlink(void *buffer, h_ringbuff_dynl_hdr_t *what)
{
        h_ringbuff_header_t *rbh = (h_ringbuff_header_t *)buffer;
        if (!rbh->dynlist_enabled) {
                return;
        }

        if (!x_atomic_load64(&what->ref_count)) {
                return;
        }

        if (what->prev) {
                h_ringbuff_dynlist_refdec(buffer, what->prev);
                h_ringbuff_dynlist_refdec(buffer, what);
                what->prev->next = what->next;
                what->prev       = NULL;
        }

        if (what->next) {
                h_ringbuff_dynlist_refdec(buffer, what->next);
                h_ringbuff_dynlist_refdec(buffer, what);
                what->next->prev = what->prev;
                what->next       = NULL;
        }
}
