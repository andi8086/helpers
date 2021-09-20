#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "ast.h"
#include "ops.h"

#include "../infix2rpn.h"


#define NUM_TESTS 100000

const char *eqrels[] = {"==", "!=", "~=", "=~", ">=", "<=", ">", "<"};


const char *binops[] = {"+", "-", "*", "/", "^", "&", "|"};


const char *unops[] = {"-", "!", "~"};


void rand_sym(char *sym)
{
        const char *charset =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.";
        memset(sym, 0, 32);

        char s      = charset[((uint8_t)rand()) % 52];
        sym[0]      = s;
        bool no_dot = false;
        for (int i = 1; i < 31; i++) {
                s = charset[((uint8_t)rand()) % sizeof(charset)];
                if (s == '.') {
                        if (no_dot) {
                                i--;
                                continue;
                        }
                        no_dot = true;
                }
                sym[i] = s;
                no_dot = false;
                if (((uint8_t)rand()) % 100 > 50) {
                        break;
                }
        }
}


ast_node_t *create_random_node(int max_level)
{
        /* with a probability of 25%, generate nothing */
        /* don't exceed the maximum level */
        int r = ((uint8_t)rand()) % 100;

        /* if generating something: */
        /* with a probability of 33%, we generate a binary operator */
        /* with a probability of 20%, we generate a unary operator */
        /* with a probability of 23%, we generate symbol */
        /* with a probability of 24%, we generate a number */

        ast_node_t *n = malloc(sizeof(ast_node_t));
        if (!n) {
                return NULL;
        }

        if (r <= 24 || (max_level - 1 == 0)) {
                r = 99;
        } else {
                r = ((uint8_t)rand()) % 100;
        }
        if (r <= 32) {
                /* generate binary operator */
                n->type = OPERATOR_BINARY;
                n->val.op =
                    binops[((uint8_t)rand()) % sizeof(binops) / sizeof(void *)];
                n->right = create_random_node(max_level - 1);
                n->left  = create_random_node(max_level - 1);
        } else if (r <= 52) {
                /* generate unary operator */
                n->type = OPERATOR_UNARY;
                n->val.op =
                    unops[((uint8_t)rand()) % sizeof(unops) / sizeof(void *)];
                /* unary operators only have a left child */
                n->left  = create_random_node(max_level - 1);
                n->right = NULL;
        } else if (r <= 77) {
                /* generate symbol */
                n->type = SYMBOL;
                rand_sym(n->val.sym);
                n->left  = NULL;
                n->right = NULL;
        } else { // (r <= 99)
                /* generate number */
                n->type    = NUMBER;
                n->val.num = rand() / ((uint8_t)rand() + 0.01);
                n->left    = NULL;
                n->right   = NULL;
        }
        return n;
}


ast_node_t *create_random_tree(void)
{
        /* create root node */
        ast_node_t *root = (ast_node_t *)malloc(sizeof(ast_node_t));
        if (!root) {
                errno = ENOMEM;
                return NULL;
        }

        root->parent = NULL;
        root->type   = OPERATOR_BINARY;
        root->val.op =
            eqrels[((uint8_t)rand()) % sizeof(eqrels) / sizeof(void *)];
        root->left  = create_random_node(20);
        root->right = create_random_node(20);
        return root;
}


int get_prec(const char *op)
{
        int prec = 0;
        for (int i = 0; i < sizeof(op_precs) / sizeof(op_prec_t); i++) {
                size_t len = op_precs[i].len;
                if (strncmp(op, op_precs[i].op, len) == 0) {
                        return op_precs[i].prec;
                }
        }
        return prec;
}


void tree_traverse_in_order(ast_node_t *t, int parent_prec, char *buffer)
{
        if (!t) {
                return;
        }

        bool par = false;

        if (t->type == OPERATOR_UNARY) {
                if (*t->val.op == '-') {
                        sprintf(buffer, "(");
                        buffer += strlen(buffer);
                        par = true;
                }
                sprintf(buffer, "%s ", t->val.op);
                buffer += strlen(buffer);
        }

        /* if this is a binary operator and its precedence is less
         * than the current one, we have to print a parenthesis */

        int this_prec = 0;
        if (t->type == OPERATOR_BINARY || t->type == OPERATOR_UNARY) {
                this_prec = get_prec(t->val.op);
                if (this_prec <= parent_prec) {
                        /* the minus unary operator is a special case,
                         * we always want a bracket BEFORE it */
                        if (!(t->type == OPERATOR_UNARY && *t->val.op == '-')) {
                                par = true;
                                sprintf(buffer, "(");
                        }
                }
        }

        tree_traverse_in_order(t->left, this_prec, buffer + strlen(buffer));

        buffer += strlen(buffer);

        switch (t->type) {
        case OPERATOR_BINARY: sprintf(buffer, " %s ", t->val.op); break;
        case SYMBOL: sprintf(buffer, "%s", t->val.sym); break;
        case NUMBER: sprintf(buffer, "%lg", t->val.num); return;
        default:
                if (par) {
                        sprintf(buffer, ")");
                }
                return;
        }

        tree_traverse_in_order(t->right, this_prec, buffer + strlen(buffer));
        buffer += strlen(buffer);
        if (par) {
                sprintf(buffer, ")");
        }
}


void tree_to_infix(ast_node_t *t, char *buffer)
{
        /* in order traversal */
        tree_traverse_in_order(t, 0, buffer);
}


void tree_traverse_postorder(ast_node_t *t, char *buffer)
{
        if (!t) {
                return;
        }

        tree_traverse_postorder(t->left, buffer + strlen(buffer));
        tree_traverse_postorder(t->right, buffer + strlen(buffer));

        buffer += strlen(buffer);
        char s = ' ';

        switch (t->type) {
        case OPERATOR_UNARY:
                s = *t->val.op;
                if (s == '-') {
                        s = '#';
                }
        case OPERATOR_BINARY:
                if (s == '#') {
                        sprintf(buffer, "# ");
                } else {
                        sprintf(buffer, "%s ", t->val.op);
                }
                break;
        case SYMBOL: sprintf(buffer, "%s ", t->val.sym); break;
        case NUMBER: sprintf(buffer, "%lg ", t->val.num); break;
        default: break;
        }
}


void tree_to_rpn(ast_node_t *t, char *buffer)
{
        tree_traverse_postorder(t, buffer);
}


void traverse_free(ast_node_t *n)
{
        if (n->left) {
                traverse_free(n->left);
                free(n->left);
        }
        if (n->right) {
                traverse_free(n->right);
                free(n->right);
        }
}


char rpn_buffer[1024];
char rpn2_buffer[1024];
char ifix_buffer[1024];

int main(int argc, char **argv)
{
        srand(11);

        int pass = 0;
        int fail = 0;

        for (int i = 0; i < NUM_TESTS; i++) {
                memset(rpn_buffer, 0, sizeof(rpn_buffer));
                memset(ifix_buffer, 0, sizeof(ifix_buffer));

                ast_node_t *root = create_random_tree();
                tree_to_infix(root, ifix_buffer);
                tree_to_rpn(root, rpn_buffer);
                infix_to_rpn(ifix_buffer, rpn2_buffer, sizeof(rpn2_buffer));
                if (strcmp(rpn2_buffer, rpn_buffer) == 0) {
                        printf("infix: %s\n", ifix_buffer);
                        printf("ast->rpn: %s\n", rpn_buffer);
                        printf("infix2rpn: %s\n", rpn2_buffer);
                        printf("[PASS]\n\n");
                        pass++;
                } else {
                        if (i < 900) {
                                continue;
                        }
                        printf("infix: %s\n", ifix_buffer);
                        printf("ast->rpn: %s\n", rpn_buffer);
                        printf("infix2rpn: %s\n", rpn2_buffer);
                        printf("[FAIL]\n\n");
                        fail++;
                }
                traverse_free(root);
                free(root);
        }
        printf("%d succeeded, %d failed\n", pass, fail);
        return EXIT_SUCCESS;
}
