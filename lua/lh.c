#include "lh.h"

#include <string.h>


uint64_t lh_traverse_onto_stack(lua_State *l, char *parent)
{
        const char delim[2] = ".";
        char par[LH_MAX_ID];
        char *token;

        if (strlen(parent) >= LH_MAX_ID) {
                return LH_ID_TOO_LONG;
        }
        strncpy(par, parent, LH_MAX_ID);

        token = strtok(par, delim);
        if (!token) {
                /* We were not given any valid parent, use global table */
                lua_pushglobaltable(l);
                return LH_OK;
        }
        if (!lua_getglobal(l, token)) {
                /* We were given a global name, but could not find it */
                return LH_NOT_FOUND;
        }
        token = strtok(NULL, delim);
        while (token) {
                /* Here the parent is on the top of the stack.
                 * Within the parent table we have to fetch the
                 * one described by the next token. */
                if (!lua_getfield(l, -1, token)) {
                        return LH_NOT_FOUND;
                }
                token = strtok(NULL, delim);
        }

        return LH_OK;
}


#define TRAVERSE_CHECK_PARENT(parent, old_top)            \
        uint64_t res = lh_traverse_onto_stack(l, parent); \
        if (res) {                                        \
                lua_settop(l, old_top);                   \
                return res;                               \
        }                                                 \
        if (lua_type(l, -1) != LUA_TTABLE) {              \
                return LH_PARENT_NOT_TABLE;               \
        }


uint64_t lh_set_table(lua_State *l, char *parent, char *name)
{
        int old_top = lua_gettop(l);
        if (!name || !strlen(name)) {
                return LH_INVALID_NAME;
        }

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushstring(l, name);
        lua_newtable(l);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_set_string(lua_State *l, char *parent, char *name, char *val)
{
        int old_top = lua_gettop(l);
        if (!name || !strlen(name)) {
                return LH_INVALID_NAME;
        }

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushstring(l, name);
        lua_pushstring(l, val);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_set_number(lua_State *l, char *parent, char *name, double val)
{
        int old_top = lua_gettop(l);
        if (!name || !strlen(name)) {
                return LH_INVALID_NAME;
        }

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushstring(l, name);
        lua_pushnumber(l, val);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_set_bool(lua_State *l, char *parent, char *name, int val)
{
        int old_top = lua_gettop(l);
        if (!name || !strlen(name)) {
                return LH_INVALID_NAME;
        }

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushstring(l, name);
        lua_pushboolean(l, (int)val);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_set_pointer(lua_State *l, char *parent, char *name, void *p)
{
        int old_top = lua_gettop(l);
        if (!name || !strlen(name)) {
                return LH_INVALID_NAME;
        }

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushstring(l, name);
        lua_pushlightuserdata(l, p);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_seti_table(lua_State *l, char *parent, double idx)
{
        int old_top = lua_gettop(l);

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushnumber(l, idx);
        lua_newtable(l);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_seti_string(lua_State *l, char *parent, double idx, char *val)
{
        int old_top = lua_gettop(l);

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushnumber(l, idx);
        lua_pushstring(l, val);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_seti_number(lua_State *l, char *parent, double idx, double val)
{
        int old_top = lua_gettop(l);

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushnumber(l, idx);
        lua_pushnumber(l, val);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_seti_bool(lua_State *l, char *parent, double idx, int val)
{
        int old_top = lua_gettop(l);

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushnumber(l, idx);
        lua_pushboolean(l, (int)val);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


uint64_t lh_seti_pointer(lua_State *l, char *parent, double idx, void *p)
{
        int old_top = lua_gettop(l);

        TRAVERSE_CHECK_PARENT(parent, old_top)

        lua_pushnumber(l, idx);
        lua_pushlightuserdata(l, p);
        lua_settable(l, -3);
        lua_settop(l, old_top);
        return LH_OK;
}


static void lh_print_table_sub(lua_State *L, int indent)
{
        const char *key;

        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
                lua_pushvalue(L, -2);
                /* copy key for tostring, otherwise lua_next would crash
                 * because tostring changes the data type on the stack */
                key = lua_tostring(L, -1);
                printf("%-*s%s : ", indent, " ", key);
                /* remove the copy of the key */
                lua_pop(L, 1);

                if (lua_type(L, -1) == LUA_TTABLE) {
                        printf("{\n");
                        lh_print_table_sub(L, indent + 4);
                        printf("%-*s}\n", indent, " ");
                } else if (lua_type(L, -1) == LUA_TSTRING) {
                        printf("'%s' \n", lua_tostring(L, -1));
                } else if (lua_type(L, -1) == LUA_TNUMBER) {
                        printf("%f \n", lua_tonumber(L, -1));
                } else if (lua_type(L, -1) == LUA_TBOOLEAN) {
                        printf("%s \n",
                               lua_toboolean(L, -1) ? "true" : "false");
                }
                /* TNIL values are not considered to be part of a table by Lua
                 * */
                lua_pop(L, 1);
        }
}


uint64_t lh_print_table(lua_State *l, char *name)
{
        int old_top  = lua_gettop(l);
        uint64_t res = lh_traverse_onto_stack(l, name);
        if (res) {
                lua_settop(l, old_top);
                return res;
        }
        lh_print_table_sub(l, 0);
        lua_settop(l, old_top);
        return LH_OK;
}
