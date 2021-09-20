#include "lh.h"


int main(int argc, char **argv)
{
        uint64_t res;
        lua_State *l = luaL_newstate();

        if (!l) {
                fprintf(stderr, "Could not create Lua VM\n");
                return -1;
        }

        printf("Lua stack is at %d\n", lua_gettop(l));

        res = lh_set_table(l, "", "ENV");
        if (res) {
                printf("lh_set_table returned %llu\n", res);
                goto error;
        }

        res = lh_set_table(l, "ENV", "A");
        if (res) {
                printf("lh_set_table returned %llu\n", res);
                goto error;
        }

        res = lh_set_table(l, "ENV.A", "B");
        if (res) {
                printf("lh_set_table returned %llu\n", res);
        }

        /* Add some associative values */
        res = lh_set_string(l, "ENV.A.B", "str1", "hallo");
        res = lh_set_string(l, "", "blah", NULL);
        res = lh_set_string(l, "ENV.A", "s", "ENV->A->s");
        res = lh_set_number(l, "ENV", "num1", 3.141);
        res = lh_set_bool(l, "ENV.A", "b", true);

        /* Add a value with number index */
        res = lh_seti_string(l, "", 1.414, "sqrt(2)");

        printf("\nAll globals:\n");
        lh_print_table(l, "");

        printf("\nTable ENV.A:\n");
        lh_print_table(l, "ENV.A");

        printf("\nLua stack is at %d\n", lua_gettop(l));
error:
        lua_close(l);

        return 0;
}
