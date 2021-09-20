#include "btrb.h"
#include "btrb_tools.h"
#include <inttypes.h>


void btrb_print_dot_null(btrb_node_t *n, int nullcount, FILE *stream)
{
        fprintf(stream, "       null%d [shape=point];\n", nullcount);
        fprintf(stream, "       \"%" PRIx64 "\" -> null%d;\n", n->val,
                nullcount);
        fprintf(stream, "       \"%" PRIx64 "\" [color=\"%s\"];\n", n->val,
                n->color == RED ? "red" : "black");
}


void btrb_print_dot_aux(btrb_node_t *node, FILE *stream)
{
        static int nullcount = 0;

        if (btrb_is_nil(node)) {
                btrb_print_dot_null(node, nullcount++, stream);
                return;
        }

        if (!btrb_is_nil(node->left)) {
                fprintf(stream, "       \"%" PRIx64 "\" -> \"%" PRIx64 "\";\n",
                        node->val, node->left->val);
                fprintf(stream, "       \"%" PRIx64 "\" [color=\"%s\"];\n",
                        node->val, node->color == RED ? "red" : "black");
                btrb_print_dot_aux(node->left, stream);
        } else {
                btrb_print_dot_null(node, nullcount++, stream);
        }

        if (!btrb_is_nil(node->right)) {
                fprintf(stream, "       \"%" PRIx64 "\" -> \"%" PRIx64 "\";\n",
                        node->val, node->right->val);
                fprintf(stream, "       \"%" PRIx64 "\" [color=\"%s\"];\n",
                        node->val, node->color == RED ? "red" : "black");
                btrb_print_dot_aux(node->right, stream);
        } else {
                btrb_print_dot_null(node, nullcount++, stream);
        }
}


void btrb_print_dot(btrb_node_t *tree, char *filename)
{
        FILE *stream;

        stream = fopen(filename, "w");
        if (!stream) {
                return;
        }
        fprintf(stream, "digraph BST {\n");
        fprintf(stream, "       node [fontname=\"Arial\"];\n");

        if (!tree && !btrb_is_nil(tree)) {
                fprintf(stream, "\n");
        } else if (btrb_is_nil(tree->right) && btrb_is_nil(tree->left)) {
                fprintf(stream, "       %d;\n", tree->val);
        } else {
                btrb_print_dot_aux(tree, stream);
        }
        fprintf(stream, "}\n");
        fclose(stream);
}
