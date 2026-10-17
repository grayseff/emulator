#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "ppc.h"

static int
getreg(lua_State *L)
{
	Cpu *cpu;
	lua_Integer reg;

	cpu = lua_touserdata(L, lua_upvalueindex(1));
	reg = luaL_checkinteger(L, 1);
	luaL_argcheck(L, reg >= 0 && reg < 32, 1,
		"register index must be between 0 and 31");

	lua_pushinteger(L, (lua_Integer)cpu->gpr[reg]);
	return 1;
}

static int
setreg(lua_State *L)
{
	Cpu *cpu;
	lua_Integer reg;
	lua_Integer value;

	cpu = lua_touserdata(L, lua_upvalueindex(1));
	reg = luaL_checkinteger(L, 1);
	luaL_argcheck(L, reg >= 0 && reg < 32, 1,
		"register index must be between 0 and 31");
	value = luaL_checkinteger(L, 2);
	cpu->gpr[reg] = (uint32_t)value;

	return 0;
}
static int
getcr(lua_State *L)
{
	Cpu *cpu = lua_touserdata(L, lua_upvalueindex(1));
	lua_pushinteger(L, cpu->cr);
	return 1;
}
static int
setcr(lua_State *L)
{
    Cpu *cpu = lua_touserdata(L, lua_upvalueindex(1));
    cpu->cr = (uint32_t)luaL_checkinteger(L, 1);
    return 0;
}
static int
getpc(lua_State *L)
{
    Cpu *cpu = lua_touserdata(L, lua_upvalueindex(1));
    lua_pushinteger(L, cpu->pc);
    return 1;
}
static int
setlr(lua_State *L)
{
    Cpu *cpu = lua_touserdata(L, lua_upvalueindex(1));
    cpu->lr = (uint32_t)luaL_checkinteger(L, 1);
    return 0;
}
static int
getctr(lua_State *L)
{
    Cpu *cpu = lua_touserdata(L, lua_upvalueindex(1));
    lua_pushinteger(L, cpu->ctr);
    return 1;
}
static int
setxer(lua_State *L)
{
    Cpu *cpu = lua_touserdata(L, lua_upvalueindex(1));
    cpu->xer = (uint32_t)luaL_checkinteger(L, 1);
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

	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, getreg, 1);
	lua_setglobal(L, "getreg");

	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, setreg, 1);
	lua_setglobal(L, "setreg");

	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, getcr, 1);
	lua_setglobal(L, "getcr");

	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, setcr, 1);
	lua_setglobal(L, "setcr");

	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, getpc, 1);
	lua_setglobal(L, "getpc");

	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, setlr, 1);
	lua_setglobal(L, "setlr");

	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, getctr, 1);
	lua_setglobal(L, "getctr");

	lua_pushlightuserdata(L, cpu);
	lua_pushcclosure(L, setxer, 1);
	lua_setglobal(L, "setxer");

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
