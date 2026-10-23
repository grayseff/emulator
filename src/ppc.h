#ifndef PPC_H
#define PPC_H

#include <stdlib.h>
#include <limits.h>
#include "instruction.h"

#if CHAR_BIT != 8 || USHRT_MAX !=0xffffU || UINT_MAX != 0xffffffffU
#error Unsupported integer widths
#endif
/*------------------------------------------------------*/
/*------------------------Structs-----------------------*/
/*------------------------------------------------------*/

typedef struct {
	unsigned char *data;
	size_t size;
} Memory;
typedef struct {
	unsigned int gpr[32];
	unsigned int pc;

	unsigned int cr;
	unsigned int lr;
	unsigned int ctr;
	unsigned int xer;
} Cpu;

/*------------------------------------------------------*/
/*---------------------Memory---------------------------*/
/*------------------------------------------------------*/
int meminit(Memory *mem, size_t size);
void memfree(Memory *mem);

unsigned char memread8(Memory *, unsigned int);
unsigned short memread16(Memory *, unsigned int);
unsigned int memread32(Memory *, unsigned int);

void memwrite8(Memory *, unsigned int, unsigned char);
void memwrite16(Memory *, unsigned int, unsigned short);
void memwrite32(Memory *, unsigned int, unsigned int);

/*--------------------------------------------------------*/
/*---------------------decode/exec-----------------------*/
/*------------------------------------------------------*/

int pefload(const char *, Memory *);
int step(Cpu *, Memory *);

#endif
