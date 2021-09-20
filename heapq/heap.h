#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct {
        size_t n;
        size_t mem;
        void **elements;
        bool (*is_lt)(void *, void *);
} heapq_t;

typedef bool (*heapq_is_lt_func_t)(void *, void *);


heapq_t *heapq_init(heapq_t *heap, size_t n, bool (*is_lt_func)(void *, void *),
                    void **buffer);
void *heapq_pop(heapq_t *h);
void *heapq_push(heapq_t *h, void *e);
heapq_t *heapq_heapify(heapq_t *h, void **x, int len,
                       heapq_is_lt_func_t is_lt_func);


#endif
