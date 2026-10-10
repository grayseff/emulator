#include "instruction.h"
#include "ppc.h"
#include "lua.h"

#define MEM_SIZE 1024

int
step(Cpu *cpu, Memory *mem)
{
	uint32_t value;
	Instruction inst;

	if (mem->size < sizeof(uint32_t) ||
		(size_t)cpu->pc > mem->size - sizeof(uint32_t))
		return 1;

	value = memread32(mem, cpu->pc);
	inst = decode(value);

	cpu->pc += 4;

	execute(cpu, inst, mem);

	return 0;
}

int
main(int argc, char *argv[])
{
	Cpu cpu = {0};
	Memory mem = {0};

	int status = meminit(&mem, MEM_SIZE);
	if (status)
		goto cleanup;

	lua_State *L = luainit(&cpu, &mem);
	if (L == NULL) {
		status = -1;
		goto cleanup_mem;
	}
	if (argc < 2)
		goto cleanup_lua;

	status = luarun(L, argv[1]);

	/* emulator */
cleanup_lua:
	luadestroy(L);
cleanup_mem:
	memfree(&mem);
cleanup:

	return status;
}
