#ifndef IFIX2RPN_H
#define IFIX2RPN_H


#define ITOR_OK             0
#define ITOR_PAR_MISMATCH   -1
#define ITOR_STACK_OVERFLOW -2
#define ITOR_ERROR_UNKNOWN  -3
#define ITOR_INVAL_NUM      -4
#define ITOR_INVAL_OP       -5
#define ITOR_BUFF_TOO_SMALL -6


/* This function performs a conversion from infix notation to reverse polnish
 * notation. Syntax or semantics are **NOT** checked by this function, although
 * a few cases are detected automatically, like a parenthesis mismatch.
 *
 * The output buffer is cleared by the function. Definitions of operators is
 * in ops.h */
int infix_to_rpn(const char *expression, char *output_buffer, size_t buffsize);

#endif
