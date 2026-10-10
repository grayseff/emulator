#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "ppc.h"

#define XER_SO (1u << 31)

static uint32_t
signextend16(uint16_t value)
{
	if (value & 0x8000u)
		return 0xffff0000u | value;
	return value;
}

static uint32_t
signextend24(uint32_t value)
{
	value &= 0x00ffffffu;
	if (value & 0x00800000u)
		return value | 0xff000000u;
	return value;
}

static uint32_t
signextend14(uint16_t value)
{
	value &= 0x3fffu;
	if (value & 0x2000u)
		return (uint32_t)value | 0xffffc000u;
	return value;
}

static uint32_t
dformaddress(const Cpu *cpu, uint8_t ra, uint16_t immediate)
{
	uint32_t base = ra == 0 ? 0 : cpu->gpr[ra];
	return base + signextend16(immediate);
}

static uint32_t
xformaddress(const Cpu *cpu, uint8_t ra, uint8_t rb)
{
	uint32_t base = ra == 0 ? 0 : cpu->gpr[ra];
	return base + cpu->gpr[rb];
}

static void
setcrfield(Cpu *cpu, uint8_t bf, uint32_t value)
{
	uint32_t shift = (7u - bf) * 4u;
	value |= (cpu->xer & XER_SO) != 0;
	cpu->cr = (cpu->cr & ~(0xfu << shift)) | (value << shift);
}

static void
setcr0(Cpu *cpu, uint32_t result)
{
	uint32_t value;
	if (result & 0x80000000u)
		value = 0x8;
	else if (result != 0)
		value = 0x4;
	else
		value = 0x2;
	setcrfield(cpu, 0, value);
}

static void
setcompare(Cpu *cpu, uint8_t bf, bool less, bool greater)
{
	setcrfield(cpu, bf, less ? 0x8 : greater ? 0x4 : 0x2);
}

static bool
branchcondition(Cpu *cpu, uint8_t bo, uint8_t bi)
{
	bool ctrok;
	bool crok;

	if (bo & 0x04u) {
		ctrok = true;
	} else {
		--cpu->ctr;
		ctrok = (bo & 0x02u) ? cpu->ctr == 0 : cpu->ctr != 0;
	}

	uint32_t crbit = (cpu->cr >> (31u - bi)) & 1u;
	crok = (bo & 0x10u) ? true : crbit == ((bo >> 3) & 1u);
	return ctrok && crok;
}

static void
branch(Cpu *cpu, uint32_t signedoffset, bool aa)
{
	uint32_t displacement = signedoffset << 2;

	if (aa)
		cpu->pc = displacement;
	else
		/* PC already points past this instruction. */
		cpu->pc = cpu->pc - 4u + displacement;
}

static uint32_t
rotateleft32(uint32_t value, uint8_t shift)
{
	shift &= 0x1fu;
	if (shift == 0)
		return value;
	return (value << shift) | (value >> (32u - shift));
}

static uint32_t
rotatemask(uint8_t mb, uint8_t me)
{
	uint32_t mask = 0;
	for (uint8_t bit = 0; bit < 32; ++bit) {
		if (mb <= me ? bit >= mb && bit <= me : bit >= mb || bit <= me)
			mask |= 1u << (31u - bit);
	}
	return mask;
}

void
execute(Cpu *cpu, Instruction ins, Memory *memory)
{
	/* The caller advances PC before execution. */
	uint32_t address;
	uint32_t result;
	uint32_t value;

	switch (ins.opcode) {
	case ADD:
		result = cpu->gpr[ins.op.x.ra] + cpu->gpr[ins.op.x.rb];
		cpu->gpr[ins.op.x.rd] = result;
		if (ins.op.x.rc)
			setcr0(cpu, result);
		break;

	case ADDI:
		value = ins.op.d.ra == 0 ? 0 : cpu->gpr[ins.op.d.ra];
		cpu->gpr[ins.op.d.rx] = value + signextend16(ins.op.d.imm);
		break;

	case ADDIS:
		value = ins.op.d.ra == 0 ? 0 : cpu->gpr[ins.op.d.ra];
		cpu->gpr[ins.op.d.rx] =
			value + (signextend16(ins.op.d.imm) << 16);
		break;

	case SUBF:
		result = cpu->gpr[ins.op.x.rb] - cpu->gpr[ins.op.x.ra];
		cpu->gpr[ins.op.x.rd] = result;
		if (ins.op.x.rc)
			setcr0(cpu, result);
		break;

	/* Logical instructions use the first field as RS and RA as destination;
	 * arithmetic instructions use the first field as RD. */
	case AND:
		result = cpu->gpr[ins.op.x.rd] & cpu->gpr[ins.op.x.rb];
		cpu->gpr[ins.op.x.ra] = result;
		if (ins.op.x.rc)
			setcr0(cpu, result);
		break;

	case ANDI:
		result = cpu->gpr[ins.op.d.rx] & ins.op.d.imm;
		cpu->gpr[ins.op.d.ra] = result;
		setcr0(cpu, result);
		break;

	case ANDIS:
		result = cpu->gpr[ins.op.d.rx] & ((uint32_t)ins.op.d.imm << 16);
		cpu->gpr[ins.op.d.ra] = result;
		setcr0(cpu, result);
		break;

	case OR:
		result = cpu->gpr[ins.op.x.rd] | cpu->gpr[ins.op.x.rb];
		cpu->gpr[ins.op.x.ra] = result;
		if (ins.op.x.rc)
			setcr0(cpu, result);
		break;

	case ORI:
		cpu->gpr[ins.op.d.ra] = cpu->gpr[ins.op.d.rx] | ins.op.d.imm;
		break;

	case ORIS:
		cpu->gpr[ins.op.d.ra] = cpu->gpr[ins.op.d.rx] |
			((uint32_t)ins.op.d.imm << 16);
		break;

	case XOR:
		result = cpu->gpr[ins.op.x.rd] ^ cpu->gpr[ins.op.x.rb];
		cpu->gpr[ins.op.x.ra] = result;
		if (ins.op.x.rc)
			setcr0(cpu, result);
		break;

	case XORI:
		cpu->gpr[ins.op.d.ra] = cpu->gpr[ins.op.d.rx] ^ ins.op.d.imm;
		break;

	case XORIS:
		cpu->gpr[ins.op.d.ra] = cpu->gpr[ins.op.d.rx] ^
			((uint32_t)ins.op.d.imm << 16);
		break;

	case CMP: {
		uint32_t left = cpu->gpr[ins.op.cmp.ra];
		uint32_t right = cpu->gpr[ins.op.cmp.rb];
		setcompare(cpu, ins.op.cmp.bf, (int32_t)left < (int32_t)right,
			(int32_t)left > (int32_t)right);
		break;
	}

	case CMPL: {
		uint32_t left = cpu->gpr[ins.op.cmp.ra];
		uint32_t right = cpu->gpr[ins.op.cmp.rb];
		setcompare(cpu, ins.op.cmp.bf, left < right, left > right);
		break;
	}

	case CMPI: {
		uint32_t left = cpu->gpr[ins.op.cmpi.ra];
		uint32_t right = signextend16(ins.op.cmpi.imm);
		setcompare(cpu, ins.op.cmpi.bf, (int32_t)left < (int32_t)right,
			(int32_t)left > (int32_t)right);
		break;
	}

	case CMPLI: {
		uint32_t left = cpu->gpr[ins.op.cmpi.ra];
		uint32_t right = ins.op.cmpi.imm;
		setcompare(cpu, ins.op.cmpi.bf, left < right, left > right);
		break;
	}

	case B:
		if (ins.op.b.lk)
			cpu->lr = cpu->pc;
		branch(cpu, signextend24(ins.op.b.li), ins.op.b.aa);
		break;

	case BC:
		if (ins.op.bc.lk)
			cpu->lr = cpu->pc;
		if (branchcondition(cpu, ins.op.bc.bo, ins.op.bc.bi))
			branch(cpu, signextend14(ins.op.bc.bd), ins.op.bc.aa);
		break;

	case BCLR:
		value = cpu->lr;
		if (ins.op.bclr.lk)
			cpu->lr = cpu->pc;
		if (branchcondition(cpu, ins.op.bclr.bo, ins.op.bclr.bi)) {
			cpu->pc = value & ~3u;
		}
		break;

	case LWZ:
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		cpu->gpr[ins.op.d.rx] = memread32(memory, address);
		break;
	case LWZU:
		/* TODO: Raise a program exception for RA == 0 or RA == RT. */
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		cpu->gpr[ins.op.d.rx] = memread32(memory, address);
		cpu->gpr[ins.op.d.ra] = address;
		break;
	case LBZ:
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		cpu->gpr[ins.op.d.rx] = memread8(memory, address);
		break;
	case LBZU:
		/* TODO: Raise a program exception for RA == 0 or RA == RT. */
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		cpu->gpr[ins.op.d.rx] = memread8(memory, address);
		cpu->gpr[ins.op.d.ra] = address;
		break;
	case LHZ:
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		cpu->gpr[ins.op.d.rx] = memread16(memory, address);
		break;
	case LHZU:
		/* TODO: Raise a program exception for RA == 0 or RA == RT. */
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		cpu->gpr[ins.op.d.rx] = memread16(memory, address);
		cpu->gpr[ins.op.d.ra] = address;
		break;
	case LHA:
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		cpu->gpr[ins.op.d.rx] =
			signextend16(memread16(memory, address));
		break;
	case LHAU:
		/* TODO: Raise a program exception for RA == 0 or RA == RT. */
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		cpu->gpr[ins.op.d.rx] =
			signextend16(memread16(memory, address));
		cpu->gpr[ins.op.d.ra] = address;
		break;
	case STW:
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		memwrite32(memory, address, cpu->gpr[ins.op.d.rx]);
		break;
	case STWU:
		/* TODO: Raise a program exception when RA is 0. */
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		memwrite32(memory, address, cpu->gpr[ins.op.d.rx]);
		cpu->gpr[ins.op.d.ra] = address;
		break;
	case STB:
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		memwrite8(memory, address, (uint8_t)cpu->gpr[ins.op.d.rx]);
		break;
	case STBU:
		/* TODO: Raise a program exception when RA is 0. */
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		memwrite8(memory, address, (uint8_t)cpu->gpr[ins.op.d.rx]);
		cpu->gpr[ins.op.d.ra] = address;
		break;
	case STH:
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		memwrite16(memory, address, (uint16_t)cpu->gpr[ins.op.d.rx]);
		break;
	case STHU:
		/* TODO: Raise a program exception when RA is 0. */
		address = dformaddress(cpu, ins.op.d.ra, ins.op.d.imm);
		memwrite16(memory, address, (uint16_t)cpu->gpr[ins.op.d.rx]);
		cpu->gpr[ins.op.d.ra] = address;
		break;

	case LWZX:
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		cpu->gpr[ins.op.x.rd] = memread32(memory, address);
		break;
	case LWZUX:
		/* TODO: Raise a program exception for RA == 0 or RA == RT. */
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		cpu->gpr[ins.op.x.rd] = memread32(memory, address);
		cpu->gpr[ins.op.x.ra] = address;
		break;
	case LBZX:
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		cpu->gpr[ins.op.x.rd] = memread8(memory, address);
		break;
	case LBZUX:
		/* TODO: Raise a program exception for RA == 0 or RA == RT. */
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		cpu->gpr[ins.op.x.rd] = memread8(memory, address);
		cpu->gpr[ins.op.x.ra] = address;
		break;
	case LHZX:
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		cpu->gpr[ins.op.x.rd] = memread16(memory, address);
		break;
	case LHZUX:
		/* TODO: Raise a program exception for RA == 0 or RA == RT. */
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		cpu->gpr[ins.op.x.rd] = memread16(memory, address);
		cpu->gpr[ins.op.x.ra] = address;
		break;
	case LHAX:
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		cpu->gpr[ins.op.x.rd] =
			signextend16(memread16(memory, address));
		break;
	case LHAUX:
		/* TODO: Raise a program exception for RA == 0 or RA == RT. */
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		cpu->gpr[ins.op.x.rd] =
			signextend16(memread16(memory, address));
		cpu->gpr[ins.op.x.ra] = address;
		break;
	case STWX:
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		memwrite32(memory, address, cpu->gpr[ins.op.x.rd]);
		break;
	case STWUX:
		/* TODO: Raise a program exception when RA is 0. */
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		memwrite32(memory, address, cpu->gpr[ins.op.x.rd]);
		cpu->gpr[ins.op.x.ra] = address;
		break;
	case STBX:
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		memwrite8(memory, address, (uint8_t)cpu->gpr[ins.op.x.rd]);
		break;
	case STBUX:
		/* TODO: Raise a program exception when RA is 0. */
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		memwrite8(memory, address, (uint8_t)cpu->gpr[ins.op.x.rd]);
		cpu->gpr[ins.op.x.ra] = address;
		break;
	case STHX:
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		memwrite16(memory, address, (uint16_t)cpu->gpr[ins.op.x.rd]);
		break;
	case STHUX:
		/* TODO: Raise a program exception when RA is 0. */
		address = xformaddress(cpu, ins.op.x.ra, ins.op.x.rb);
		memwrite16(memory, address, (uint16_t)cpu->gpr[ins.op.x.rd]);
		cpu->gpr[ins.op.x.ra] = address;
		break;

	case MFSPR:
		switch (ins.op.spr.spr) {
		case 1: cpu->gpr[ins.op.spr.rx] = cpu->xer; break;
		case 8: cpu->gpr[ins.op.spr.rx] = cpu->lr; break;
		case 9: cpu->gpr[ins.op.spr.rx] = cpu->ctr; break;
		default:
			fprintf(stderr, "unsupported MFSPR number: %u\n",
				(unsigned)ins.op.spr.spr);
			abort();
		}
		break;

	case MTSPR:
		switch (ins.op.spr.spr) {
		case 1: cpu->xer = cpu->gpr[ins.op.spr.rx]; break;
		case 8: cpu->lr = cpu->gpr[ins.op.spr.rx]; break;
		case 9: cpu->ctr = cpu->gpr[ins.op.spr.rx]; break;
		default:
			fprintf(stderr, "unsupported MTSPR number: %u\n",
				(unsigned)ins.op.spr.spr);
			abort();
		}
		break;

	case RLWINM:
		result = rotateleft32(cpu->gpr[ins.op.rlw.rs], ins.op.rlw.rx) &
			rotatemask(ins.op.rlw.mb, ins.op.rlw.me);
		cpu->gpr[ins.op.rlw.ra] = result;
		if (ins.op.rlw.rc)
			setcr0(cpu, result);
		break;

	case RLWIMI:
		value = rotateleft32(cpu->gpr[ins.op.rlw.rs], ins.op.rlw.rx);
		result = rotatemask(ins.op.rlw.mb, ins.op.rlw.me);
		cpu->gpr[ins.op.rlw.ra] =
			(cpu->gpr[ins.op.rlw.ra] & ~result) | (value & result);
		if (ins.op.rlw.rc)
			setcr0(cpu, cpu->gpr[ins.op.rlw.ra]);
		break;

	case RLWNM:
		value = cpu->gpr[ins.op.rlw.rx] & 0x1fu;
		result = rotateleft32(cpu->gpr[ins.op.rlw.rs], (uint8_t)value) &
			rotatemask(ins.op.rlw.mb, ins.op.rlw.me);
		cpu->gpr[ins.op.rlw.ra] = result;
		if (ins.op.rlw.rc)
			setcr0(cpu, result);
		break;

	case UNKNOWN:
		/* TODO: Raise an illegal-instruction exception. */
		puts("unknown instruction:");
		break;
	}
}
