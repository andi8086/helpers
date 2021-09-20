#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <external/pcre2/src/pcre2.h>
#include <regex/regex.h>
#include <stackq/stackq.h>

#include "ops.h"
#include "infix2rpn.h"

typedef enum {
        SDT_OP_BINARY,
        SDT_OP_UNARY,
        SDT_VALUE,
        SDT_VARIABLE,
        SDT_FUNCTION
} stackq_data_type_t;


static bool check_precedence(const char *op1, const char *op2, bool unary1,
                             bool unary2, int *s_idx)
{
        /* returns true if op2 has a greater precedence or
         * if precedence of op1 and op2 are equal and op1 is
         * right to left-associative */
        int op1_prec = -1;
        int op2_prec = -1;
        int ass1     = -1;
        *s_idx       = 1;

        for (int i = 0; i < sizeof(op_precs) / sizeof(op_prec_t); i++) {
                size_t len = op_precs[i].len;
                if (op1_prec == -1 && strncmp(op1, op_precs[i].op, len) == 0) {
                        op1_prec = op_precs[i].prec;
                        ass1     = op_precs[i].ass;
                        *s_idx   = i;
                }
                if (op2_prec == -1 && strncmp(op2, op_precs[i].op, len) == 0) {
                        op2_prec = op_precs[i].prec;
                }
                if (op1_prec != -1 && op2_prec != -1) {
                        break;
                }
        }

        if (op1 == "-" && unary1) {
                op1_prec += 20;
                ass1 = 1;
        }
        if (op2 == "-" && unary2) {
                op2_prec += 20;
        }

        /* The actual shunting yard algorithm also checks for associativity
         * in case of equal precedence, which is at least needed for ^ with
         * the meaning of 'power to'... We do not use this operator
         * and it did not work with unary operators...
         * Hence all the ass1, ass2 stuff is not needed and could be removed
         * It is still there for future expansion if needed */
        return op2_prec > op1_prec; // || (op2_prec == op1_prec && ass1 == 1);
}


static size_t handle_operator(const char *s, stackq_t *operator_stack,
                              stackq_t *output_queue, bool unary)
{
        char *ptr;
        size_t sz;
        stackq_data_type_t ptr_type;
        int s_idx;

        /* just to initialize s_idx */
        (void)check_precedence(s, s, false, false, &s_idx);

        ptr_type = stackq_peek(operator_stack, &ptr);
        while (ptr) {
                /* there is something in the operator stack */
                /* if it is an operator, check precedence */
                if (*s == '(') {
                        return 1;
                        break;
                }
                if (check_precedence(s, ptr, unary, ptr_type, &s_idx)) {
                        /* pop from operator stack and push into
                         * output queue */
                        stackq_pop(operator_stack, &ptr, &sz, NULL);
                        stackq_push(output_queue, ptr, sz,
                                    unary ? SDT_OP_UNARY : SDT_OP_BINARY);
                        stackq_zero_last(operator_stack);
                } else {
                        break;
                }
                ptr_type = stackq_peek(operator_stack, &ptr);
        }
        return op_precs[s_idx].len;
}


static bool is_number(const char *s)
{
        switch (*(s + 1)) {
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': return true;
        default: return false;
        }

        return false;
}


int infix_to_rpn(const char *expression, char *output, size_t buffsize)
{
        stackq_t operator_stack;
        stackq_t output_queue;
        char *s = (char *)expression;
        char *t;
        preg_match_ctx_t preg_match_ctx = {0};
        int itor_error;

        const char *identifier = "^[a-zA-Z_]+[a-zA-Z_0-9.]*";
        bool binop_ok          = false;

        int res;
        size_t sz, op_len;
        char *ptr;
        char *endptr;
        double num;

        stackq_init(&operator_stack);
        stackq_init(&output_queue);
        memset(output, 0, buffsize);

        while (*s) {
                if ((*s >= 'a' && *s <= 'z') ||
                    ((*s >= 'A' && *s <= 'Z') || *s == '_')) {
                        /* this is a token, so match from this position on */
                        t = preg_match(&preg_match_ctx, s, identifier);
                        if (!t) {
                                /* this is impossible, we have an error */
                                itor_error = ITOR_ERROR_UNKNOWN;
                                goto itor_error_exit;
                        }
                        // printf("%s\n", t);
                        stackq_push(&output_queue, t, strlen(t), SDT_VARIABLE);
                        s += strlen(t);
                        binop_ok = true;
                        continue;
                }

                switch (*s) {
                case '+':
                case '~':
                case '!':
                case '-':
                        if (binop_ok) {
                                /* this goto saves one indentation level */
                                goto use_binop;
                        }
                        /* check if a variable name follows */
                        t = preg_match(&preg_match_ctx, s + 1, identifier);
                        if (!t) {
                                /* No variable, but this might
                                 * start a number */
                                if (is_number(s)) {
                                        goto try_number;
                                }
                        }
                        /* this is a unary operator before a
                         * variable */
                        op_len = handle_operator(s, &operator_stack,
                                                 &output_queue, true);
                        if (*s == '-') {
                                if (stackq_push(&operator_stack, "#", 1,
                                                SDT_OP_UNARY) < 0) {
                                        itor_error = ITOR_STACK_OVERFLOW;
                                        goto itor_error_exit;
                                }
                        } else if (stackq_push(&operator_stack, s, 1,
                                               SDT_OP_UNARY) < 0) {
                                itor_error = ITOR_STACK_OVERFLOW;
                                goto itor_error_exit;
                        }
                        s += op_len;
                        break;
                case '/':
                case '*':
                case '^':
                case '&':
                case '|':
                case '=':
                case '<':
                case '>':
                use_binop:
                        if (!binop_ok) {
                                itor_error = ITOR_INVAL_OP;
                                goto itor_error_exit;
                        }
                        /* this is an operator, check precedence */
                        op_len = handle_operator(s, &operator_stack,
                                                 &output_queue, false);
                        /* push it into the operator stack */
                        if (stackq_push(&operator_stack, s, op_len,
                                        SDT_OP_BINARY) < 0) {
                                /* could not push, stack overflow */
                                itor_error = ITOR_STACK_OVERFLOW;
                                goto itor_error_exit;
                        }
                        binop_ok = false;
                        s += op_len;
                        break;
                case '(':
                        binop_ok = false;
                        if (stackq_push(&operator_stack, s, 1, SDT_OP_BINARY) <
                            0) {
                                itor_error = ITOR_STACK_OVERFLOW;
                                goto itor_error_exit;
                        }
                        s++;
                        break;
                case ')':
                par_loop:
                        res = stackq_pop(&operator_stack, &ptr, &sz, NULL);
                        /* if we could not pop here, this is an
                         * error, since we got no opening
                         * parenthesis */
                        if (res < 0) {
                                itor_error = ITOR_PAR_MISMATCH;
                                goto itor_error_exit;
                        }

                        if (*ptr == '(') {
                                /* we found the opening
                                 * parenthesis */

                                /* now check if it belongs to a
                                 * function */
                                if (stackq_peek(&operator_stack, NULL) ==
                                    SDT_FUNCTION) {
                                        res = stackq_pop(&operator_stack, &ptr,
                                                         &sz, NULL);
                                        if (stackq_push(&output_queue, ptr, sz,
                                                        SDT_FUNCTION) < 0) {
                                                itor_error =
                                                    ITOR_STACK_OVERFLOW;
                                                goto itor_error_exit;
                                        }
                                }
                                goto exit_par_loop;
                        }

                        /* push it into output queue, what we
                         * just popped */
                        if (stackq_push(&output_queue, ptr, sz, SDT_OP_BINARY) <
                            0) {
                                itor_error = ITOR_STACK_OVERFLOW;
                                goto itor_error_exit;
                        }
                        stackq_zero_last(&operator_stack);
                        /* save one indentation level with goto */
                        goto par_loop;
                exit_par_loop:
                        s++;
                        break;
                case '0':
                case '.':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                try_number:
                        errno = 0;
                        num   = strtod(s, &endptr);
                        if (num == 0.0 && errno == EINVAL) {
                                itor_error = ITOR_INVAL_NUM;
                                goto itor_error_exit;
                        }
                        if (errno == ERANGE) {
                                itor_error = ITOR_INVAL_NUM;
                                goto itor_error_exit;
                        }
                        stackq_push(&output_queue, s, endptr - s, SDT_VALUE);
                        s        = endptr;
                        binop_ok = true;
                        break;
                case '\r':
                case '\n':
                case ' ':
                default: s++;
                }
        }
        itor_error = ITOR_OK;
itor_error_exit:
        preg_match_end(&preg_match_ctx);

        /* pop everything from operator stack and push onto output queue */
        while (!stackq_pop(&operator_stack, &ptr, &sz, NULL)) {
                if (*ptr == '(') {
                        itor_error = ITOR_PAR_MISMATCH;
                        return itor_error;
                }
                stackq_push(&output_queue, ptr, sz, 0);
                stackq_zero_last(&operator_stack);
        }

        ptr = output_queue.mem;
        for (int i = 0; i < output_queue.count; i++) {
                /* check for buffer overrun, we get mem_size without
                 * null-termination, strlen also does not provide
                 * null termination. Buffersize must include null-termination,
                 * i.e. subtract 1 from buffersize for null-termination */
                if (strlen(output) + output_queue.mem_size[i] > buffsize - 1) {
                        itor_error = ITOR_BUFF_TOO_SMALL;
                        return itor_error;
                }
                sprintf(output, "%.*s ", output_queue.mem_size[i], ptr);
                ptr += output_queue.mem_size[i];
                output += strlen(output);
        }

        return itor_error;
}
