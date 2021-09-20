#include "ops.h"

/* in the following mapping, the longer strings come
 * first for strncmp to work. Each entry contains
 * operator string and its precedence */


/*
        "-", "!", "~",      unary, right, 100
        "*", "/", "%",      multiplicative, left, 90
        "+", "-",           additive, left, 80
        "<<", ">>",         shift, left, 70
        "<", "<=", ">", ">=",  relational, left, 60
        "==", "~=", "!=", "=~",   equality, left, 50
        "&",                bitwise AND, left, 40
        "^",                bitwise XOR, left, 30
        "|",                bitwise OR, left, 20
        "&&",               logical AND, 10
        "||"                logical OR, 0
*/


/* IMPORTANT: if a unary operator is encountered, 20 is added
 * to the priority for - (minus) */
const op_prec_t op_precs[22] = {
    {.op = "||", .prec = 0, .len = 2, .ass = 0},  /* logical OR, left */
    {.op = "&&", .prec = 10, .len = 2, .ass = 0}, /* logical AND, left */
    {.op = "=~", .prec = 50, .len = 2, .ass = 0}, /* equ, left */
    {.op = "==", .prec = 50, .len = 2, .ass = 0}, /* equ, left */
    {.op = "~=", .prec = 50, .len = 2, .ass = 0}, /* equ, left */
    {.op = "<<", .prec = 70, .len = 2, .ass = 0}, /* shift, left */
    {.op = ">>", .prec = 70, .len = 2, .ass = 1}, /* shift, right */
    {.op = "<=", .prec = 60, .len = 2, .ass = 0}, /* rel, left */
    {.op = ">=", .prec = 60, .len = 2, .ass = 0}, /* rel, left */
    //        { .op = "-", prec = 100, .len = 1, .ass = 1}, /* unary, right */
    {.op = "!=", .prec = 50, .len = 2, .ass = 0}, /* equ, left */
    {.op = "!", .prec = 100, .len = 1, .ass = 1}, /* unary, right */
    {.op = "~", .prec = 100, .len = 1, .ass = 1}, /* unary, right */
    {.op = "*", .prec = 90, .len = 1, .ass = 0},  /* mult, left */
    {.op = "/", .prec = 90, .len = 1, .ass = 0},  /* mult, left */
    {.op = "%", .prec = 90, .len = 1, .ass = 0},  /* mult, left */
    {.op = "+", .prec = 80, .len = 1, .ass = 0},  /* add, left */
    {.op = "-", .prec = 80, .len = 1, .ass = 0},  /* add, left */
    {.op = "<", .prec = 60, .len = 1, .ass = 0},  /* rel, left */
    {.op = ">", .prec = 60, .len = 1, .ass = 0},  /* rel, left */
    {.op = "&", .prec = 40, .len = 1, .ass = 0},  /* bitwise AND, left */
    {.op = "^", .prec = 30, .len = 1, .ass = 0},  /* bitwise XOR, left */
    {.op = "|", .prec = 20, .len = 1, .ass = 0}   /* bitwise OR, left */
};
