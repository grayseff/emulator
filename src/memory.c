#include <stdint.h>
#include <stdlib.h>

#include "ppc.h"

int
meminit(Memory *mem, size_t size)
{
	mem->data = calloc(size, sizeof(uint8_t));
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
memwrite8(Memory *mem, uint32_t address, uint8_t value)
{
	mem->data[address] = value;
}

void
memwrite16(Memory *mem, uint32_t address, uint16_t value)
{
	memwrite8(mem, address, (uint8_t)(value >> 8));
	memwrite8(mem, address + 1, (uint8_t)value);
}

void
memwrite32(Memory *mem, uint32_t address, uint32_t value)
{
	memwrite16(mem, address, (uint16_t)(value >> 16));
	memwrite16(mem, address + 2, (uint16_t)value);
}

uint8_t
memread8(Memory *mem, uint32_t address)
{
	return mem->data[address];
}

uint16_t
memread16(Memory *mem, uint32_t address)
{
	return ((uint16_t)memread8(mem, address) << 8) |
		memread8(mem, address + 1);
}

uint32_t
memread32(Memory *mem, uint32_t address)
{
	return ((uint32_t)memread16(mem, address) << 16) |
		memread16(mem, address + 2);
}
