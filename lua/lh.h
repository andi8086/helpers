#ifndef LUA_HELPER_H
#define LUA_HELPER_H

#include <stdint.h>
#include <stdbool.h>
#include <lua.h>
#include <lauxlib.h>

/* here we use identifiers like a.b.c.d.e...
 * to simplify referencing existing elements of
 * tables inside tables inside tables, etc...
 */

#define LH_MAX_ID 1024

#define LH_OK               0
#define LH_NOT_FOUND        1
#define LH_ID_TOO_LONG      2
#define LH_CANNOT_CREATE    3
#define LH_INVALID_NAME     4
#define LH_PARENT_NOT_TABLE 5


uint64_t lh_traverse_onto_stack(lua_State *l, char *parent);
uint64_t lh_set_table(lua_State *l, char *parent, char *name);
uint64_t lh_set_string(lua_State *l, char *parent, char *name, char *val);
uint64_t lh_set_number(lua_State *l, char *parent, char *name, double val);
uint64_t lh_set_bool(lua_State *l, char *parent, char *name, int val);
uint64_t lh_set_pointer(lua_State *l, char *parent, char *name, void *p);
uint64_t lh_seti_table(lua_State *l, char *parent, double idx);
uint64_t lh_seti_string(lua_State *l, char *parent, double idx, char *val);
uint64_t lh_seti_number(lua_State *l, char *parent, double idx, double val);
uint64_t lh_seti_bool(lua_State *l, char *parent, double idx, int val);
uint64_t lh_seti_pointer(lua_State *l, char *parent, double idx, void *p);
uint64_t lh_print_table(lua_State *l, char *name);

#endif
