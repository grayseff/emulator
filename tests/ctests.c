#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ppc.h"

#define MEM_SIZE 1024

static int arithmetic(Memory *);
static int branch(Memory *);
static int compare(Memory *);
static int loadstore(Memory *);
static int loadstoreindexed(Memory *);
static int logical(Memory *);
static int rotate(Memory *);
static int spr(Memory *);

int
main(void)
{
	Memory mem = {0};
	int status;
	
	status = meminit(&mem, MEM_SIZE);
	if (status) {
		printf("Could not initialise memory\n");
		return status;
	}
	if ((status = arithmetic(&mem))) {
		printf("arithmetic failed at %d\n", status);
		goto cleanup;
	}
	printf("arithmetic passed\n");
	if ((status = branch(&mem))) {
		printf("branch failed at %d\n", status);
		goto cleanup;
	}
	printf("branch passed\n");
	if ((status = compare(&mem))) {
		printf("compare failed at %d\n", status);
		goto cleanup;
	}
	printf("compare passed\n");
	if ((status = loadstore(&mem))) {
		printf("loadstore failed at %d\n", status);
		goto cleanup;
	}
	printf("loadstore passed\n");
	if ((status = loadstoreindexed(&mem))) {
		printf("loadstoreindexed failed at %d\n", status);
		goto cleanup;
	}
	printf("indexed loadstore passed\n");
	if ((status = logical(&mem))) {
		printf("logical failed at %d\n", status);
		goto cleanup;
	}
	printf("logical passed\n");
	if ((status = rotate(&mem))) {
		printf("rotate failed at %d\n", status);
		goto cleanup;
	}
	printf("rotate passed\n");
	if ((status = spr(&mem))) {
		printf("spr failed at %d\n", status);
		goto cleanup;
	}
	printf("spr passed\n");

	printf("All tests passed\n");

cleanup:
	memfree(&mem);
	return status;

}

static int
arithmetic(Memory *mem)
{
	Cpu cpu = {0};
	memset(mem->data, 0, mem->size);


	cpu.gpr[20] = 50;
	cpu.gpr[5] = 25;
	/* load insructions */
	memwrite32(mem, 0x00, 0x7C742A14); /* add   r3,r20,r5 */
	memwrite32(mem, 0x04, 0x38C30064); /* addi  r6,r3,100 */
	memwrite32(mem, 0x08, 0x7CE5A050); /* subf  r7,r5,r20 */
	memwrite32(mem, 0x0C, 0x3D140001); /* addis r8,r20,1  */
	memwrite32(mem, 0x10, 0x7C742A15); /* add.  r3,r20,r5 */

	if (step(&cpu, mem) || cpu.gpr[3] != 75)
		return 1; /* add */

	if (step(&cpu, mem) || cpu.gpr[6] != 175)
		return 2; /* addi */

	if (step(&cpu, mem) || cpu.gpr[7] != 25)
		return 3; /* subf */

	if (step(&cpu, mem) || cpu.gpr[8] != 65586)
		return 4; /* addis */

	if (step(&cpu, mem) ||
		cpu.gpr[3] != 75 ||
		((cpu.cr >> 28) & 0xF) != 0x4)
		return 5; /* add. */

	return 0;
}

static int
branch(Memory *mem)
{
	Cpu cpu = {0};
	memset(mem->data, 0, mem->size);

	cpu.gpr[20] = 50;
	cpu.gpr[5] = 25;

	memwrite32(mem, 0x00, 0x4800001A); /* b 0x18 (absolute) */
	memwrite32(mem, 0x18, 0x48000018); /* b +0x18 (relative) */
	memwrite32(mem, 0x30, 0x7C742A14); /* add r3,r20,r5 */

	if (step(&cpu, mem) || cpu.pc != 0x18)
		return 1;
	if (step(&cpu, mem) || cpu.pc != 0x30)
		return 2;
	if (step(&cpu, mem) || cpu.gpr[3] != 75 || cpu.pc != 0x34)
		return 3;

	cpu = (Cpu){0};
	cpu.cr = 0x80000000; /* CR0.LT = 1 */
	memwrite32(mem, 0x00, 0x41800008);

	if (step(&cpu, mem) || cpu.pc != 0x08)
		return 4;

	cpu = (Cpu){0};
	cpu.cr = 0;          /* CR0.LT = 0 */
	memwrite32(mem, 0x00, 0x41800008);

	if (step(&cpu, mem) || cpu.pc != 0x04)
		return 5;

	cpu = (Cpu){0};
	cpu.lr = 0x20;
	memwrite32(mem, 0x00, 0x4E800020); /* blr */

	if (step(&cpu, mem) || cpu.pc != 0x20)
		return 6;

	return 0;
}

static int
compare(Memory *mem)
{
	Cpu cpu = {0};
	memset(mem->data, 0, mem->size);

	cpu.gpr[3] = 0xFFFFFFFB; /* -5 as signed int */
	cpu.gpr[4] = 10;

	memwrite32(mem, 0x00, 0x2C03FFF6); /* cmpwi cr0,r3,-10 */
	memwrite32(mem, 0x04, 0x28040014); /* cmplwi cr0,r4,20 */
	
	if (step(&cpu, mem) || ((cpu.cr >> 28) & 0xF) != 0x4)
		return 1; /* -5 > -10 */

	if (step(&cpu, mem) || ((cpu.cr >> 28) & 0xF) != 0x8)
		return 2; /* 10 < 20 */

	if (cpu.pc != 0x08)
		return 3;

	cpu = (Cpu){0};
	cpu.gpr[3] = 0xFFFFFFFF;
	cpu.gpr[4] = 1;

	memwrite32(mem, 0x00, 0x7C032040); /* cmplw cr0,r3,r4 */

	if (step(&cpu, mem) || ((cpu.cr >> 28) & 0xF) != 0x4)
		return 4; /* unsigned GT */

	if (cpu.pc != 0x04)
		return 5;

	cpu = (Cpu){0};
	cpu.xer = 0x80000000; /* XER.SO = 1 */
	cpu.gpr[20] = 50;

	memwrite32(mem, 0x00, 0x7C14A000); /* cmpw cr0,r20,r20 */

	if (step(&cpu, mem) || ((cpu.cr >> 28) & 0xF) != 0x3)
		return 6; /* EQ | SO */

	if (cpu.pc != 0x04)
		return 7;

	return 0;
}

static int
loadstore(Memory *mem)
{
	Cpu cpu = {0};
	memset(mem->data, 0, mem->size);

	/* Immediate-offset stores */
	cpu.gpr[1] = 0x40;
	cpu.gpr[5] = 0x12345678;
	cpu.gpr[6] = 0x0FFF8001;

	memwrite32(mem, 0x00, 0x90A10000); /* stw */
	memwrite32(mem, 0x04, 0x94A10008); /* stwu */
	memwrite32(mem, 0x08, 0x98C10004); /* stb */
	memwrite32(mem, 0x0C, 0x9CC1000C); /* stbu */
	memwrite32(mem, 0x10, 0xB0C10002); /* sth */
	memwrite32(mem, 0x14, 0xB4C10004); /* sthu */

	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;

	if (memread32(mem, 0x40) != 0x12345678) return 2;
	if (memread32(mem, 0x48) != 0x12345678) return 3;
	if (memread8(mem, 0x4C) != 1) return 4;
	if (memread8(mem, 0x54) != 1) return 5;
	if (memread16(mem, 0x56) != 0x8001) return 6;
	if (memread16(mem, 0x58) != 0x8001) return 7;
	if (cpu.gpr[1] != 0x58) return 8;

	/* Immediate-offset loads */
	cpu.gpr[1] = 0x40;

	memwrite32(mem, 0x18, 0x81010000); /* lwz */
	memwrite32(mem, 0x1C, 0x85210008); /* lwzu */
	memwrite32(mem, 0x20, 0x89410004); /* lbz */
	memwrite32(mem, 0x24, 0x8D61000C); /* lbzu */
	memwrite32(mem, 0x28, 0xA1810002); /* lhz */
	memwrite32(mem, 0x2C, 0xA5A10004); /* lhzu */
	memwrite32(mem, 0x30, 0xA9C1FFFE); /* lha */
	memwrite32(mem, 0x34, 0xADE10000); /* lhau */

	if (step(&cpu, mem))
		return 9;
	if (step(&cpu, mem))
		return 9;
	if (step(&cpu, mem))
		return 9;
	if (step(&cpu, mem))
		return 9;
	if (step(&cpu, mem))
		return 9;
	if (step(&cpu, mem))
		return 9;
	if (step(&cpu, mem))
		return 9;
	if (step(&cpu, mem))
		return 9;

	if (cpu.gpr[8] != 0x12345678) return 10;
	if (cpu.gpr[9] != 0x12345678) return 11;
	if (cpu.gpr[10] != 1) return 12;
	if (cpu.gpr[11] != 1) return 13;
	if (cpu.gpr[12] != 0x8001) return 14;
	if (cpu.gpr[13] != 0x8001) return 15;
	if (cpu.gpr[14] != 0xFFFF8001) return 16;
	if (cpu.gpr[15] != 0xFFFF8001) return 17;
	if (cpu.gpr[1] != 0x58) return 18;

	return 0;
}
static int
loadstoreindexed(Memory *mem)
{
	Cpu cpu = {0};
	memset(mem->data, 0, mem->size);

	cpu.gpr[1] = 0x40;
	cpu.gpr[5] = 0x12345678;
	cpu.gpr[6] = 0x0FFF8001;

	cpu.gpr[16] = 0;
	cpu.gpr[17] = 8;
	cpu.gpr[18] = 4;
	cpu.gpr[19] = 12;
	cpu.gpr[20] = 2;
	cpu.gpr[21] = 0xFFFFFFFE;

	memwrite32(mem, 0x00, 0x7CA1812E); /* stwx */
	memwrite32(mem, 0x04, 0x7CA1896E); /* stwux */
	memwrite32(mem, 0x08, 0x7CC191AE); /* stbx */
	memwrite32(mem, 0x0C, 0x7CC199EE); /* stbux */
	memwrite32(mem, 0x10, 0x7CC1A32E); /* sthx */
	memwrite32(mem, 0x14, 0x7CC1936E); /* sthux */

	if (step(&cpu, mem))
		return 19;
	if (step(&cpu, mem))
		return 19;
	if (step(&cpu, mem))
		return 19;
	if (step(&cpu, mem))
		return 19;
	if (step(&cpu, mem))
		return 19;
	if (step(&cpu, mem))
		return 19;

	if (memread32(mem, 0x40) != 0x12345678) return 20;
	if (memread32(mem, 0x48) != 0x12345678) return 21;
	if (memread8(mem, 0x4C) != 1) return 22;
	if (memread8(mem, 0x54) != 1) return 23;
	if (memread16(mem, 0x56) != 0x8001) return 24;
	if (memread16(mem, 0x58) != 0x8001) return 25;
	if (cpu.gpr[1] != 0x58) return 26;

	/* Indexed loads */
	cpu.gpr[1] = 0x40;

	memwrite32(mem, 0x18, 0x7D01802E); /* lwzx */
	memwrite32(mem, 0x1C, 0x7D21886E); /* lwzux */
	memwrite32(mem, 0x20, 0x7D4190AE); /* lbzx */
	memwrite32(mem, 0x24, 0x7D6198EE); /* lbzux */
	memwrite32(mem, 0x28, 0x7D81A22E); /* lhzx */
	memwrite32(mem, 0x2C, 0x7DA1926E); /* lhzux */
	memwrite32(mem, 0x30, 0x7DC1AAAE); /* lhax */
	memwrite32(mem, 0x34, 0x7DE182EE); /* lhaux */

	if (step(&cpu, mem))
		return 27;
	if (step(&cpu, mem))
		return 27;
	if (step(&cpu, mem))
		return 27;
	if (step(&cpu, mem))
		return 27;
	if (step(&cpu, mem))
		return 27;
	if (step(&cpu, mem))
		return 27;
	if (step(&cpu, mem))
		return 27;
	if (step(&cpu, mem))
		return 27;

	if (cpu.gpr[8] != 0x12345678) return 28;
	if (cpu.gpr[9] != 0x12345678) return 29;
	if (cpu.gpr[10] != 1) return 30;
	if (cpu.gpr[11] != 1) return 31;
	if (cpu.gpr[12] != 0x8001) return 32;
	if (cpu.gpr[13] != 0x8001) return 33;
	if (cpu.gpr[14] != 0xFFFF8001) return 34;
	if (cpu.gpr[15] != 0xFFFF8001) return 35;
	if (cpu.gpr[1] != 0x58) return 36;

	return 0;
}

static int
logical(Memory *mem)
{
	Cpu cpu = {0};
	memset(mem->data, 0, mem->size);

	/* Initial register state */
	cpu.gpr[20] = 50;
	cpu.gpr[5] = 25;
	cpu.gpr[21] = 0xA;
	cpu.gpr[22] = 0xC;

	/* Register logical operations */
	memwrite32(mem, 0x00, 0x7EA8B038); /* and */
	memwrite32(mem, 0x04, 0x7EA9B378); /* or */
	memwrite32(mem, 0x08, 0x7EAAB278); /* xor */

	/* Comparisons */
	memwrite32(mem, 0x0C, 0x7C15B000);
	memwrite32(mem, 0x10, 0x7C942800);
	memwrite32(mem, 0x14, 0x7D14A000);

	/* Immediate logical operations */
	memwrite32(mem, 0x18, 0x628B000C); /* ori */
	memwrite32(mem, 0x1C, 0x668C000C); /* oris */
	memwrite32(mem, 0x20, 0x6A8D000C); /* xori */
	memwrite32(mem, 0x24, 0x6E8E000C); /* xoris */
	memwrite32(mem, 0x28, 0x728F0032); /* andi. */
	memwrite32(mem, 0x2C, 0x768F0032); /* andis. */

	/* First ten instructions */
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;
	if (step(&cpu, mem))
		return 1;

	if (cpu.gpr[8] != 8) return 2;
	if (cpu.gpr[9] != 14) return 3;
	if (cpu.gpr[10] != 6) return 4;
	if (cpu.gpr[11] != 62) return 5;
	if (cpu.gpr[12] != 786482) return 6;
	if (cpu.gpr[13] != 62) return 7;
	if (cpu.gpr[14] != 786482) return 8;
	if (cpu.cr != 0x84200000) return 9;

	/* ANDI. */
	if (step(&cpu, mem))
		return 10;
	if (cpu.gpr[15] != 50)
		return 11;
	if (((cpu.cr >> 28) & 0xF) != 0x4)
		return 12;

	/* ANDIS. */
	if (step(&cpu, mem))
		return 13;
	if (cpu.gpr[15] != 0)
		return 14;
	if (((cpu.cr >> 28) & 0xF) != 0x2)
		return 15;

	return 0;
}

static int
rotate(Memory *mem)
{
	Cpu cpu = {0};
	memset(mem->data, 0, mem->size);

	/* rlwinm: rotate left immediate, then mask */
	cpu.gpr[3] = 0x12345678;
	memwrite32(mem, 0x00, 0x5464403E);

	if (step(&cpu, mem) || cpu.gpr[4] != 0x34567812)
		return 1;

	/* rlwnm: rotate left by register, then mask */
	cpu.gpr[3] = 0x12345678;
	cpu.gpr[5] = 8;
	memwrite32(mem, 0x04, 0x5C64283E);

	if (step(&cpu, mem) || cpu.gpr[4] != 0x34567812)
		return 2;

	/* rlwimi: rotate and insert masked bits */
	cpu.gpr[3] = 0x12345678;
	cpu.gpr[4] = 0xFFFF0000;
	memwrite32(mem, 0x08, 0x5064043E);

	if (step(&cpu, mem) || cpu.gpr[4] != 0xFFFF5678)
		return 3;

	/* rlwinm: wraparound mask */
	cpu.gpr[3] = 0x12345678;
	memwrite32(mem, 0x0C, 0x54640706);

	if (step(&cpu, mem) || cpu.gpr[4] != 0x10000008)
		return 4;

	return 0;
}

static int
spr(Memory *mem)
{
	Cpu cpu = {0};
	memset(mem->data, 0, mem->size);

	/* mflr r4 */
	cpu.lr = 0x12345678;
	memwrite32(mem, 0x00, 0x7C8802A6);

	if (step(&cpu, mem) ||
		cpu.gpr[4] != 0x12345678 ||
		cpu.pc != 0x04)
		return 1;

	/* mtctr r3 */
	cpu = (Cpu){0};
	cpu.gpr[3] = 0x12345678;
	memwrite32(mem, 0x00, 0x7C6903A6);

	if (step(&cpu, mem) ||
		cpu.ctr != 0x12345678 ||
		cpu.pc != 0x04)
		return 2;

	return 0;
}
