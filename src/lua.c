#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>

#include "ppc.h"

static int
gprindex(lua_State *L)
{
	Cpu *cpu = *(Cpu **)lua_touserdata(L, 1);
	lua_Integer index = luaL_checkinteger(L, 2);

	if (index < 0 || index >= 32)
		return luaL_error(L, "invalid GPR index: %lld",
		(long long)index);

	lua_pushinteger(L, cpu->gpr[index]);
	return 1;
}

static int
gprnewindex(lua_State *L)
{
    Cpu *cpu = *(Cpu **)lua_touserdata(L, 1);
    lua_Integer index = luaL_checkinteger(L, 2);

    if (index < 0 || index >= 32)
        return luaL_error(L, "invalid GPR index: %lld",
            (long long)index);

    cpu->gpr[index] = (uint32_t)luaL_checkinteger(L, 3);
    return 0;
}

static int
cpuindex(lua_State *L)
{
	Cpu *cpu = *(Cpu **)lua_touserdata(L,1);
	const char *key = luaL_checkstring(L,2);
	if (strcmp(key, "gpr") == 0) {
	    Cpu **userdata = lua_newuserdata(L, sizeof(Cpu *));
	    *userdata = cpu;

	    luaL_getmetatable(L, "CpuGpr");
	    lua_setmetatable(L, -2);

	    return 1;
	}
	else if (strcmp(key, "pc") == 0)
	    lua_pushinteger(L, cpu->pc);
	else if (strcmp(key, "cr") == 0)
		lua_pushinteger(L, cpu->cr);
	else if (strcmp(key, "lr") == 0)
		lua_pushinteger(L, cpu->lr);
	else if (strcmp(key, "ctr") == 0)
		lua_pushinteger(L, cpu->ctr);
	else if (strcmp(key, "xer") == 0)
		lua_pushinteger(L, cpu->xer);
	else
		return luaL_error(L, "unknown CPU field: %s", key);

	return 1;
}

static int
cpunewindex(lua_State *L)
{
	Cpu *cpu = *(Cpu **)lua_touserdata(L, 1);
	const char *key = luaL_checkstring(L, 2);
	if (strcmp(key, "gpr") == 0)
	    return luaL_error(L, "cannot replace CPU register array");
	uint32_t value = (uint32_t)luaL_checkinteger(L, 3);

	if (strcmp(key, "pc") == 0)
		cpu->pc = value;
	else if (strcmp(key, "cr") == 0)
		cpu->cr = value;
	else if (strcmp(key, "lr") == 0)
		cpu->lr = value;
	else if (strcmp(key, "ctr") == 0)
		cpu->ctr = value;
	else if (strcmp(key, "xer") == 0)
		cpu->xer = value;
	else
		return luaL_error(L, "unknown CPU field: %s", key);

	return 0;
}


static int
resetcpu(lua_State *L)
{
    Cpu *cpu = lua_touserdata(L, lua_upvalueindex(1));
    *cpu = (Cpu){0};
    return 0;
}

static int
read8(lua_State *L)
{
	Memory *mem;
	uint32_t address;

	mem = lua_touserdata(L, lua_upvalueindex(1));
	address = (uint32_t)luaL_checkinteger(L, 1);
	lua_pushinteger(L, memread8(mem, address));

	return 1;
}

static int
read16(lua_State *L)
{
	Memory *mem;
	uint32_t address;

	mem = lua_touserdata(L, lua_upvalueindex(1));
	address = (uint32_t)luaL_checkinteger(L, 1);
	lua_pushinteger(L, memread16(mem, address));

	return 1;
}

static int
read32(lua_State *L)
{
	Memory *mem;
	uint32_t address;

	mem = lua_touserdata(L, lua_upvalueindex(1));
	address = (uint32_t)luaL_checkinteger(L, 1);
	lua_pushinteger(L, (lua_Integer)memread32(mem, address));

	return 1;
}

static int
write8(lua_State *L)
{
	Memory *mem;
	uint32_t address;
	lua_Integer value;

	mem = lua_touserdata(L, lua_upvalueindex(1));
	address = (uint32_t)luaL_checkinteger(L, 1);
	value = luaL_checkinteger(L, 2);
	memwrite8(mem, address, (uint8_t)value);

	return 0;
}

static int
write16(lua_State *L)
{
	Memory *mem;
	uint32_t address;
	lua_Integer value;

	mem = lua_touserdata(L, lua_upvalueindex(1));
	address = (uint32_t)luaL_checkinteger(L, 1);
	value = luaL_checkinteger(L, 2);
	memwrite16(mem, address, (uint16_t)value);

	return 0;
}

static int
write32(lua_State *L)
{
	Memory *mem;
	uint32_t address;
	lua_Integer value;

	mem = lua_touserdata(L, lua_upvalueindex(1));
	address = (uint32_t)luaL_checkinteger(L, 1);
	value = luaL_checkinteger(L, 2);
	memwrite32(mem, address, (uint32_t)value);

	return 0;
}

static int
luastep(lua_State *L)
{
	Cpu *cpu;
	Memory *mem;

	cpu = lua_touserdata(L, lua_upvalueindex(1));
	mem = lua_touserdata(L, lua_upvalueindex(2));
	int status = step(cpu, mem);
	lua_pushinteger(L, status);

	return 1;
}

void
luadestroy(lua_State *L)
{
	lua_close(L);
}

int
luarun(lua_State *L, const char *file)
{
	int status = luaL_dofile(L, file);
	if (status != LUA_OK) {
		const char *message = lua_tostring(L, -1);
		fprintf(stderr, "%s\n", message ? message :
			"Lua error (non-string error object)");
		lua_pop(L, 1);
	}
	return status;
}

lua_State *
luainit(Cpu *cpu, Memory *mem)
{
	lua_State *L;

	L = luaL_newstate();
	if (L == NULL)
		return NULL;

	luaL_openlibs(L);
	luaL_newmetatable(L, "Cpu");

	lua_pushcfunction(L, cpuindex);
	lua_setfield(L, -2, "__index");

	lua_pushcfunction(L, cpunewindex);
	lua_setfield(L, -2, "__newindex");

	lua_pop(L, 1);

	Cpu **userdata = lua_newuserdata(L, sizeof(Cpu *));
	*userdata = cpu;

	luaL_getmetatable(L, "Cpu");
	lua_setmetatable(L, -2);

	lua_setglobal(L, "cpu");

	luaL_newmetatable(L, "CpuGpr");

	lua_pushcfunction(L, gprindex);
	lua_setfield(L, -2, "__index");

	lua_pushcfunction(L, gprnewindex);
	lua_setfield(L, -2, "__newindex");

	lua_pop(L, 1);
	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, resetcpu, 1);
	lua_setglobal(L, "resetcpu");

	lua_pushlightuserdata(L, mem);
	lua_pushcclosure(L, read8, 1);
	lua_setglobal(L, "read8");

	lua_pushlightuserdata(L, mem);
	lua_pushcclosure(L, read16, 1);
	lua_setglobal(L, "read16");

	lua_pushlightuserdata(L, mem);
	lua_pushcclosure(L, read32, 1);
	lua_setglobal(L, "read32");

	lua_pushlightuserdata(L, mem);
	lua_pushcclosure(L, write8, 1);
	lua_setglobal(L, "write8");

	lua_pushlightuserdata(L, mem);
	lua_pushcclosure(L, write16, 1);
	lua_setglobal(L, "write16");

	lua_pushlightuserdata(L, mem);
	lua_pushcclosure(L, write32, 1);
	lua_setglobal(L, "write32");

	lua_pushlightuserdata(L, cpu);
	lua_pushlightuserdata(L, mem);
	lua_pushcclosure(L, luastep, 2);
	lua_setglobal(L, "step");

	return L;
}
