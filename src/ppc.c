#include "instruction.h"
#include "ppc.h"

#define MEM_SIZE 1024

int
main(int argc, char *argv[])
{
	Cpu cpu = {0};
	Memory mem = {0};

	int status = meminit(&mem, MEM_SIZE);
	if (status)
		goto cleanup;
cleanup_mem:
	memfree(&mem);
cleanup:

	return status;
}


