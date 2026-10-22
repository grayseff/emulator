#ifndef PPC_H
#define PPC_H

#include <stdint.h>
#include <stdlib.h>

#include "instruction.h"
/*------------------------------------------------------*/
/*------------------------Structs-----------------------*/
/*------------------------------------------------------*/

typedef struct {
	uint8_t *data;
	size_t size;
} Memory;
typedef struct {
	uint32_t gpr[32];
	uint32_t pc;

	uint32_t cr;
	uint32_t lr;
	uint32_t ctr;
	uint32_t xer;
} Cpu;

/*------------------------------------------------------*/
/*---------------------Memory---------------------------*/
/*------------------------------------------------------*/
int meminit(Memory *mem, size_t size);
void memfree(Memory *mem);

uint8_t memread8(Memory *, uint32_t);
uint16_t memread16(Memory *, uint32_t);
uint32_t memread32(Memory *, uint32_t);

void memwrite8(Memory *, uint32_t, uint8_t);
void memwrite16(Memory *, uint32_t, uint16_t);
void memwrite32(Memory *, uint32_t, uint32_t);

/*--------------------------------------------------------*/
/*---------------------decode/exec-----------------------*/
/*------------------------------------------------------*/

int pefload(const char *);
int step(Cpu *, Memory *);

#endif
