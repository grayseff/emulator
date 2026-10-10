#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H
#include <lua.h>
#include "ppc.h"

lua_State *luainit(Cpu *, Memory *);
void luadestroy(lua_State *);
int luarun(lua_State *, const char *);

#endif
