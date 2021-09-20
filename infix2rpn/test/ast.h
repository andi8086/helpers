#ifndef AST_H
#define AST_H


typedef enum { OPERATOR_UNARY, OPERATOR_BINARY, NUMBER, SYMBOL } node_type_t;


typedef struct ast_node {
        struct ast_node *parent;
        struct ast_node *left;
        struct ast_node *right;
        node_type_t type;
        union {
                const char *op;
                char sym[32];
                double num;
        } val;
} ast_node_t;


#endif
