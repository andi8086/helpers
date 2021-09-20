#ifndef BT_RB_TOOLS
#define BT_RB_TOOLS

#include "btrb.h"
#include <stdio.h>

void btrb_print_dot(btrb_node_t *tree, char *filename);
void btrb_print_dot_aux(btrb_node_t *node, FILE *stream);
void btrb_print_dot_null(btrb_node_t *n, int nullcount, FILE *stream);


#endif
