#include <string.h>

#include "stackq.h"


void stackq_init(stackq_t *s)
{
        s->sp    = 0;
        s->count = 0;
        memset(s->mem, 0, STACKQ_T_MAX);
}


/* additionally to stack data, we push positive values as data type */
int stackq_push(stackq_t *s, char *data, size_t size, int custom_data)
{
        if (s->sp + size >= STACKQ_T_MAX) {
                return -1;
        }
        memcpy(&s->mem[s->sp], data, size);
        s->mem_size[s->count]    = size;
        s->custom_data[s->count] = custom_data;
        s->sp += size;
        s->count++;
        return 0;
}


int stackq_push_concat(stackq_t *s, char *prefix, char *data, size_t size,
                       int custom_data)
{
        if (s->sp + size + strlen(prefix) >= STACKQ_T_MAX) {
                return -1;
        }
        strcpy(&s->mem[s->sp], prefix);
        s->sp += strlen(prefix);
        memcpy(&s->mem[s->sp], data, size);
        s->mem_size[s->count]    = size + strlen(prefix);
        s->custom_data[s->count] = custom_data;
        s->sp += size;
        s->count++;
        return 0;
}


int stackq_pop(stackq_t *s, char **data, size_t *size, int *custom_data)
{
        if (s->count == 0) {
                return -1;
        }
        s->count--;
        *size = s->mem_size[s->count];
        if (custom_data) {
                *custom_data = s->custom_data[s->count];
        }
        s->sp -= *size;
        *data = &s->mem[s->sp];
        return 0;
}


void stackq_zero_last(stackq_t *s)
{
        memset(&s->mem[s->sp], 0, s->mem_size[s->count]);
}


int stackq_peek(stackq_t *s, char **ptr)
{
        /* returns positive user data or -1 if no element exists */
        /* if ptr is not NULL, a pointer to mem contents is returned */
        if (s->count == 0) {
                if (ptr) {
                        *ptr = NULL;
                }
                return -1;
        }
        if (ptr) {
                *ptr = &s->mem[s->sp - s->mem_size[s->count - 1]];
        }
        return s->custom_data[s->count - 1];
}
