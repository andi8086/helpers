#ifndef STACKQ_H
#define STACKQ_H


#include <stddef.h>

#ifndef STACKQ_T_MAX
#        define STACKQ_T_MAX 8192
#endif


typedef struct {
        size_t sp;
        int count;
        char mem[STACKQ_T_MAX];
        int custom_data[STACKQ_T_MAX];
        size_t mem_size[STACKQ_T_MAX];
} stackq_t;


void stackq_init(stackq_t *s);
int stackq_push(stackq_t *s, char *data, size_t size, int custom_data);
int stackq_push_concat(stackq_t *s, char *prefix, char *data, size_t size,
                       int custom_data);
int stackq_pop(stackq_t *s, char **data, size_t *size, int *custom_data);
void stackq_zero_last(stackq_t *s);
int stackq_peek(stackq_t *s, char **ptr);


#endif
