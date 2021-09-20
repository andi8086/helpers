#include "btrb.h"
#include "btrb_compact.h"
#include <stdio.h>

#define CHECK(x, s)                         \
        {                                   \
                printf("%-44s", s);         \
                if (x)                      \
                        printf("ok\n");     \
                else {                      \
                        printf("failed\n"); \
                        return -1;          \
                }                           \
        }


btrbc_node_t static_mem[64];


int main(void)
{
        btrb_node_t *root = NULL;
        btrb_node_t nodes[6];

        printf("start with empty tree...\n");
        btrb_insert(&root, 3, NULL, &nodes[0]);
        CHECK(root == &nodes[0], "Can insert root...");
        CHECK(root->color == BLACK, "... and root is black...");

        btrb_insert(&root, 5, NULL, &nodes[1]);
        CHECK(root == &nodes[0] && root->right == &nodes[1],
              "Add larger key to right...");
        CHECK(root->color == BLACK, "... and root is black...");
        CHECK(nodes[1].color == RED, "... and new node is red...");

        btrb_insert(&root, 2, NULL, &nodes[2]);
        CHECK(root == &nodes[0] && root->left == &nodes[2],
              "Add smaller key to left...");
        CHECK(root->color == BLACK, "... and root is black...");
        CHECK(nodes[2].color == RED, "... and new node is red...");

        btrb_left_rotate(&root, &nodes[0]);
        CHECK(root == &nodes[1], "New root after left rotate...");
        CHECK(btrb_is_nil(root->right) && &nodes[0] == root->left &&
                  &nodes[2] == root->left->left &&
                  btrb_is_nil(root->left->right) &&
                  btrb_is_nil(root->left->left->right),
              "Tree structure after left rotate...");
        CHECK(root->color == RED, "... and root is red...");

        btrb_right_rotate(&root, &nodes[1]);
        CHECK(root == &nodes[0] && root->left == &nodes[2] &&
                  root->right == &nodes[1],
              "right_rotate(left_rotate(tree)) == tree...");
        CHECK(root->color == BLACK, "... and root is black...");

        btrb_right_rotate(&root, &nodes[0]);
        CHECK(root == &nodes[2], "New root after left rotate...");
        CHECK(btrb_is_nil(root->left) && &nodes[0] == root->right &&
                  &nodes[1] == root->right->right &&
                  btrb_is_nil(root->right->left) &&
                  btrb_is_nil(root->right->right->right),
              "Tree structure after left rotate...");
        CHECK(root->color == RED, "... and root is red...");

        btrb_left_rotate(&root, &nodes[2]);
        CHECK(root == &nodes[0] && root->left == &nodes[2] &&
                  root->right == &nodes[1],
              "left_rotate(right_rotate(tree)) == tree...");
        CHECK(root->color == BLACK, "... and root is black...");

        btrb_insert(&root, 17, NULL, &nodes[3]);
        btrb_insert(&root, 1, NULL, &nodes[4]);
        btrb_insert(&root, 13, NULL, &nodes[5]);

        CHECK(root->color == BLACK && root == &nodes[0] &&
                  root->left == &nodes[2] && root->right == &nodes[5] &&
                  root->left->left == &nodes[4] &&
                  btrb_is_nil(root->left->right) &&
                  root->right->left == &nodes[1] &&
                  root->right->right == &nodes[3] &&
                  btrb_is_nil(root->right->right->right),
              "Tree structure after multiple inserts...");

        btrb_delete(&root, &nodes[5]);

        CHECK(root->color == BLACK && root == &nodes[0] &&
                  root->left == &nodes[2] && root->right == &nodes[3] &&
                  root->right->left == &nodes[1] &&
                  btrb_is_nil(root->right->right) &&
                  btrb_is_nil(root->right->left->right) &&
                  root->right->left->color == RED &&
                  root->right->color == BLACK,
              "Tree structure after delete...");

        btrb_delete(&root, root);

        CHECK(root == &nodes[1], "New root after delete root...");

        root->left  = root->parent;
        root->right = root->parent;

        btrb_delete(&root, root);
        CHECK(btrb_is_nil(root), "Delete root if only root exists...");

        btrb_insert(&root, 3, NULL, &nodes[0]);
        btrb_insert(&root, 1, NULL, &nodes[1]);
        btrb_insert(&root, 5, NULL, &nodes[2]);
        btrb_delete(&root, root);
        CHECK(root == &nodes[2], "Delete root if only three elements...");

        btrb_delete(&root, &nodes[1]);
        CHECK(root == &nodes[2] && btrb_is_nil(root->left) &&
                  btrb_is_nil(root->right),
              "Delete all children from root...");
        btrb_delete(&root, &nodes[2]);

        root = NULL;

        btrb_insert(&root, 3, NULL, &nodes[0]);
        btrb_insert(&root, 1, NULL, &nodes[1]);
        btrb_insert(&root, 5, NULL, &nodes[2]);
        btrb_insert(&root, 7, NULL, &nodes[3]);
        btrb_insert(&root, 9, NULL, &nodes[4]);

        btrb_node_t *min = btrb_min(&root);
        CHECK(min == &nodes[1], "smallest of 1,3,5,7,9 == 1...");

        btrb_node_t *max = btrb_max(&root);
        CHECK(max == &nodes[4], "largest of 1,3,5,7,9 == 9...");

        btrb_node_t *tmp = max;
        printf("Starting at largest...\n");
        CHECK((tmp = btrb_next_smaller(tmp)) == &nodes[3],
              "... next smaller == 7 ...");
        CHECK((tmp = btrb_next_smaller(tmp)) == &nodes[2],
              "... next smaller == 5 ...");
        CHECK((tmp = btrb_next_smaller(tmp)) == &nodes[0],
              "... next smaller == 3 ...");
        CHECK((tmp = btrb_next_smaller(tmp)) == &nodes[1],
              "... next smaller == 1 ...");
        CHECK((tmp = btrb_next_smaller(tmp)) == NULL,
              "... next smaller == NULL ...");

        tmp = min;
        printf("Starting at smallest...\n");
        CHECK((tmp = btrb_next_larger(tmp)) == &nodes[0],
              "... next larger == 3 ...");
        CHECK((tmp = btrb_next_larger(tmp)) == &nodes[2],
              "... next larger == 5 ...");
        CHECK((tmp = btrb_next_larger(tmp)) == &nodes[3],
              "... next larger == 7 ...");
        CHECK((tmp = btrb_next_larger(tmp)) == &nodes[4],
              "... next larger == 9 ...");
        CHECK((tmp = btrb_next_larger(tmp)) == NULL,
              "... next larger == NULL ...");

        tmp = btrb_max_at_most(&root, 6);
        CHECK(tmp == &nodes[2], "max_at_most(6) == 5 ...");
        tmp = btrb_min_at_least(&root, 6);
        CHECK(tmp == &nodes[3], "min_at_least(6) == 7 ...");
        tmp = btrb_max_at_most(&root, 5);
        CHECK(tmp == &nodes[2], "max_at_most(5) == 5 ...");
        tmp = btrb_min_at_least(&root, 7);
        CHECK(tmp == &nodes[3], "min_at_least(7) == 7 ...");


        printf("Testing compact tree with 32-bit internal pointers...\n");


        btrbc_ctx_t ctx;
        btrbc_node_t *croot;
        /* initialize with nil node as static_mem[0] */

        btrbc_init(&ctx, &croot, (uintptr_t)static_mem, &static_mem[0]);

#define P32(x) ((uint32_t)((uintptr_t)x - ctx.base))
#define P64(x) ((btrbc_node_t *)((uintptr_t)x + ctx.base))

        printf("start with empty tree...\n");
        btrbc_insert(&ctx, 3, NULL, &static_mem[1]);
        CHECK(croot == &static_mem[1], "Can insert root...");
        CHECK(croot->color == BLACK, "... and root is black...");

        btrbc_insert(&ctx, 5, NULL, &static_mem[2]);

        CHECK(croot == &static_mem[1] && croot->right == P32(&static_mem[2]),
              "Add larger key to right...");
        CHECK(croot->color == BLACK, "... and root is black...");
        CHECK(static_mem[2].color == RED, "... and new node is red...");

        btrbc_insert(&ctx, 2, NULL, &static_mem[3]);
        CHECK(croot == &static_mem[1] && croot->left == P32(&static_mem[3]),
              "Add smaller key to left...");
        CHECK(croot->color == BLACK, "... and root is black...");
        CHECK(static_mem[3].color == RED, "... and new node is red...");

        btrbc_left_rotate(&ctx, &static_mem[1]);
        CHECK(croot == &static_mem[2], "New root after left rotate...");
        CHECK(croot->right == ctx.nil && P32(&static_mem[1]) == croot->left &&
                  P32(&static_mem[3]) == P64(croot->left)->left &&
                  P64(croot->left)->right == ctx.nil &&
                  P64(P64(croot->left)->left)->right == ctx.nil,
              "Tree structure after left rotate...");
        CHECK(croot->color == RED, "... and root is red...");

        btrbc_right_rotate(&ctx, &static_mem[2]);
        CHECK(croot == &static_mem[1] && P64(croot->left) == &static_mem[3] &&
                  croot->right == P32(&static_mem[2]),
              "right_rotate(left_rotate(tree)) == tree...");
        CHECK(root->color == BLACK, "... and root is black...");

        btrbc_right_rotate(&ctx, &static_mem[1]);
        CHECK(croot == &static_mem[3], "New root after left rotate...");
        CHECK(croot->left == ctx.nil && &static_mem[1] == P64(croot->right) &&
                  P32(&static_mem[2]) == P64(croot->right)->right &&
                  P64(croot->right)->left == ctx.nil &&
                  P64(P64(croot->right)->right)->right == ctx.nil,
              "Tree structure after left rotate...");
        CHECK(croot->color == RED, "... and root is red...");

        btrbc_left_rotate(&ctx, &static_mem[3]);
        CHECK(croot == &static_mem[1] && croot->left == P32(&static_mem[3]) &&
                  croot->right == P32(&static_mem[2]),
              "left_rotate(right_rotate(tree)) == tree...");
        CHECK(croot->color == BLACK, "... and root is black...");

        btrbc_insert(&ctx, 17, NULL, &static_mem[4]);
        btrbc_insert(&ctx, 1, NULL, &static_mem[5]);
        btrbc_insert(&ctx, 13, NULL, &static_mem[6]);

        CHECK(croot->color == BLACK && croot == &static_mem[1] &&
                  croot->left == P32(&static_mem[3]) &&
                  croot->right == P32(&static_mem[6]) &&
                  P64(croot->left)->left == P32(&static_mem[5]) &&
                  P64(croot->left)->right == ctx.nil &&
                  P64(croot->right)->left == P32(&static_mem[2]) &&
                  P64(croot->right)->right == P32(&static_mem[4]) &&
                  P64(P64(croot->right)->right)->right == ctx.nil,
              "Tree structure after multiple inserts...");


        btrbc_delete(&ctx, &static_mem[6]);

        CHECK(croot->color == BLACK && croot == &static_mem[1] &&
                  croot->left == P32(&static_mem[3]) &&
                  croot->right == P32(&static_mem[4]) &&
                  P64(croot->right)->left == P32(&static_mem[2]) &&
                  P64(croot->right)->right == ctx.nil &&
                  P64(P64(croot->right)->left)->right == ctx.nil &&
                  P64(P64(croot->right)->left)->color == RED &&
                  P64(croot->right)->color == BLACK,
              "Tree structure after delete...");

        btrbc_delete(&ctx, croot);

        CHECK(croot == &static_mem[2], "New root after delete root...");

        croot->left  = croot->parent;
        croot->right = croot->parent;

        btrbc_delete(&ctx, croot);
        CHECK(btrbc_is_nil(&ctx, croot), "Delete root if only root exists...");


        btrbc_insert(&ctx, 3, NULL, &static_mem[1]);
        btrbc_insert(&ctx, 1, NULL, &static_mem[2]);
        btrbc_insert(&ctx, 5, NULL, &static_mem[3]);
        btrbc_delete(&ctx, croot);
        CHECK(croot == &static_mem[3], "Delete root if only three elements...");

        btrbc_delete(&ctx, &static_mem[2]);
        CHECK(croot == &static_mem[3] && croot->left == ctx.nil &&
                  croot->right == ctx.nil,
              "Delete all children from root...");
        btrbc_delete(&ctx, &static_mem[3]);

        croot = P64(ctx.nil);

        btrbc_insert(&ctx, 3, NULL, &static_mem[1]);
        btrbc_insert(&ctx, 1, NULL, &static_mem[2]);
        btrbc_insert(&ctx, 5, NULL, &static_mem[3]);
        btrbc_insert(&ctx, 7, NULL, &static_mem[4]);
        btrbc_insert(&ctx, 9, NULL, &static_mem[5]);

        btrbc_node_t *cmin = btrbc_min(&ctx);
        CHECK(cmin == &static_mem[2], "smallest of 1,3,5,7,9 == 1...");


        btrbc_node_t *cmax = btrbc_max(&ctx);
        CHECK(cmax == &static_mem[5], "largest of 1,3,5,7,9 == 9...");

        btrbc_node_t *ctmp = cmax;
        printf("Starting at largest...\n");
        CHECK((ctmp = btrbc_next_smaller(&ctx, ctmp)) == &static_mem[4],
              "... next smaller == 7");
        CHECK((ctmp = btrbc_next_smaller(&ctx, ctmp)) == &static_mem[3],
              "... next smaller == 5");
        CHECK((ctmp = btrbc_next_smaller(&ctx, ctmp)) == &static_mem[1],
              "... next smaller == 3");
        CHECK((ctmp = btrbc_next_smaller(&ctx, ctmp)) == &static_mem[2],
              "... next smaller == 1");
        CHECK((ctmp = btrbc_next_smaller(&ctx, ctmp)) == NULL,
              "... next smaller == NULL");

        ctmp = cmin;
        printf("Starting at smallest...\n");
        CHECK((ctmp = btrbc_next_larger(&ctx, ctmp)) == &static_mem[1],
              "... next larger == 3");
        CHECK((ctmp = btrbc_next_larger(&ctx, ctmp)) == &static_mem[3],
              "... next larger == 5");
        CHECK((ctmp = btrbc_next_larger(&ctx, ctmp)) == &static_mem[4],
              "... next larger == 7");
        CHECK((ctmp = btrbc_next_larger(&ctx, ctmp)) == &static_mem[5],
              "... next larger == 9");
        CHECK((ctmp = btrbc_next_larger(&ctx, ctmp)) == NULL,
              "... next larger == NULL");

        ctmp = btrbc_max_at_most(&ctx, 6);
        CHECK(ctmp == &static_mem[3], "max_at_most(6) == 5...");

        ctmp = btrbc_min_at_least(&ctx, 6);
        CHECK(ctmp == &static_mem[4], "min_at_least(6) == 7...");

        ctmp = btrbc_max_at_most(&ctx, 5);
        CHECK(ctmp == &static_mem[3], "max_at_most(5) == 5...");

        ctmp = btrbc_min_at_least(&ctx, 7);
        CHECK(ctmp == &static_mem[4], "min_at_least(7) == 7...");

        return 0;
}
