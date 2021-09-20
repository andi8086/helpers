#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "heap.h"

heapq_t *heapq_init(heapq_t *heap, size_t n, bool (*is_lt_func)(void *, void *),
                    void **buffer)
{
        heap->n   = 0;
        heap->mem = sizeof(void *) * n;
        if (!buffer) {
                heap->elements = malloc(heap->mem);
        } else {
                heap->elements = buffer;
        }
        if (!heap->elements) {
                return NULL;
        }

        heap->is_lt = is_lt_func;
        return heap;
}


void *heapq_append(heapq_t *heap, void *e)
{
        size_t new_size;
        void *dropped = NULL;

        if (!heap) {
                return dropped;
        }

        new_size = sizeof(void *) * (heap->n + 1);
        if (heap->mem < new_size) {
                /* We are size-bounded, so we need to do something with the
                 * non-inserted/dropped elements. We cannot silently drop them,
                 * as they might be pointing to heap-allocated memory which
                 * would leak. We also cannot simply free them, since it might
                 * be that we don't store pointers, but simply uintptr_t
                 * numbers. For this reason, return the dropped element, and
                 * leave it up to the caller to either ignore it or to free it,
                 * depending on what is stored in the heap. */
                if (heap->is_lt(e, heap->elements[heap->n - 1])) {
                        dropped = heap->elements[heap->n - 1];
                        heap->elements[heap->n - 1] = e;
                } else {
                        dropped = e;
                }
        } else {
                heap->elements[heap->n] = e;
                heap->n++;
        }
        return dropped;
}


void heapq_siftdown(heapq_t *h, size_t startpos, size_t pos)
{
        void **x = h->elements;

        void *newitem = x[pos];
        while (pos > startpos) {
                size_t parentpos = (pos - 1) >> 1;
                void *parent     = x[parentpos];
                if (h->is_lt(newitem, parent)) {
                        x[pos] = parent;
                        pos    = parentpos;
                        continue;
                }
                break;
        }
        x[pos] = newitem;
}


void heapq_siftup(heapq_t *h, size_t pos, size_t len)
{
        void **x = h->elements;

        size_t endpos   = len;
        size_t startpos = pos;
        void *newitem   = x[pos];

        /* Bubble up the smaller child until hitting a leaf */

        /* leftmost child position */
        size_t childpos = 2 * pos + 1;
        while (childpos < endpos) {
                size_t rightpos = childpos + 1;
                if ((rightpos < endpos) &&
                    !(h->is_lt(x[childpos], x[rightpos]))) {
                        childpos = rightpos;
                }
                x[pos]   = x[childpos];
                pos      = childpos;
                childpos = 2 * pos + 1;
        }

        x[pos] = newitem;
        heapq_siftdown(h, startpos, pos);
}


heapq_t *heapq_heapify(heapq_t *h, void **x, int len,
                       heapq_is_lt_func_t is_lt_func)
{
        heapq_t *heap;

        heap = heapq_init(h, len, is_lt_func, x);

        for (size_t i = 0; i < len; i++) {
                heapq_append(heap, x[i]);
        }

        for (ptrdiff_t i = (ptrdiff_t)floor(len / 2) - 1; i >= 0; i--) {
                heapq_siftup(heap, i, len);
        }

        return heap;
}


void *heapq_pop(heapq_t *h)
{
        if (h->n < 1) {
                return NULL;
        }
        void *lastlt = h->elements[h->n - 1];
        h->n--;
        void *returnitem = h->elements[0];
        h->elements[0]   = lastlt;
        heapq_siftup(h, 0, h->n);

        return returnitem;
}


void *heapq_push(heapq_t *h, void *e)
{
        void *dropped = heapq_append(h, e);
        heapq_siftdown(h, 0, h->n - 1);
        return dropped;
}
