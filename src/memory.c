#include <stdint.h>
#include <stdlib.h>

#include "ppc.h"

int
meminit(Memory *mem, size_t size)
{
	mem->data = calloc(size, sizeof(unsigned char));
	if (mem->data == NULL)
		return -1;
	mem->size = size;

	return 0;
}

void
memfree(Memory *mem)
{
	free(mem->data);
	mem->data = NULL;
	mem->size = 0;
}

void
memwrite8(Memory *mem, unsigned int address, unsigned char value)
{
	mem->data[address] = value;
}

void
memwrite16(Memory *mem, unsigned int address, unsigned short value)
{
	memwrite8(mem, address, (unsigned char)(value >> 8));
	memwrite8(mem, address + 1, (unsigned char)value);
}

void
memwrite32(Memory *mem, unsigned int address, unsigned int value)
{
	memwrite16(mem, address, (unsigned short)(value >> 16));
	memwrite16(mem, address + 2, (unsigned short)value);
}

unsigned char
memread8(Memory *mem, unsigned int address)
{
	return mem->data[address];
}

unsigned short
memread16(Memory *mem, unsigned int address)
{
	return ((unsigned short)memread8(mem, address) << 8) |
		memread8(mem, address + 1);
}

unsigned int
memread32(Memory *mem, unsigned int address)
{
	return ((unsigned int)memread16(mem, address) << 16) |
		memread16(mem, address + 2);
}
