#ifndef EAS_OPS_H
#define EAS_OPS_H

#include <stddef.h>

typedef struct {
        char *op;
        int prec;
        size_t len;
        int ass;
} op_prec_t;


extern const op_prec_t op_precs[22];

#endif
