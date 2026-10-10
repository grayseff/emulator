#include "instruction.h"

Instruction
decode(uint32_t value)
{
	uint8_t opcode = (uint8_t)(value >> 26);
	Instruction inst = {0};
	inst.opcode = UNKNOWN;

	switch (opcode) {
	/* D-form immediate arithmetic and logical instructions. */
	case 14: case 15: case 24: case 25:
	case 26: case 27: case 28: case 29:
		inst.op.d.rx = (uint8_t)((value >> 21) & 0x1f);
		inst.op.d.ra = (uint8_t)((value >> 16) & 0x1f);
		inst.op.d.imm = (uint16_t)(value & 0xffff);
		switch (opcode) {
		case 14: inst.opcode = ADDI; break;
		case 15: inst.opcode = ADDIS; break;
		case 24: inst.opcode = ORI; break;
		case 25: inst.opcode = ORIS; break;
		case 26: inst.opcode = XORI; break;
		case 27: inst.opcode = XORIS; break;
		case 28: inst.opcode = ANDI; break;
		case 29: inst.opcode = ANDIS; break;
		}
		break;

	/* Compare-immediate instructions. */
	case 10: case 11:
		inst.op.cmpi.bf = (uint8_t)((value >> 23) & 0x7);
		inst.op.cmpi.ra = (uint8_t)((value >> 16) & 0x1f);
		inst.op.cmpi.imm = (uint16_t)(value & 0xffff);
		inst.opcode = (opcode == 10) ? CMPLI : CMPI;
		break;

	/* Rotate and mask instructions. */
	case 20: case 21: case 23:
		inst.op.rlw.rs = (uint8_t)((value >> 21) & 0x1f);
		inst.op.rlw.ra = (uint8_t)((value >> 16) & 0x1f);
		inst.op.rlw.rx = (uint8_t)((value >> 11) & 0x1f);
		inst.op.rlw.mb = (uint8_t)((value >> 6) & 0x1f);
		inst.op.rlw.me = (uint8_t)((value >> 1) & 0x1f);
		inst.op.rlw.rc = (value & 1) != 0;
		if (opcode == 20)
			inst.opcode = RLWIMI;
		else if (opcode == 21)
			inst.opcode = RLWINM;
		else
			inst.opcode = RLWNM;
		break;

	/* Preserve the raw 24-bit branch displacement. */
	case 18:
		inst.op.b.li = (value >> 2) & 0x00ffffff;
		inst.op.b.aa = ((value >> 1) & 1) != 0;
		inst.op.b.lk = (value & 1) != 0;
		inst.opcode = B;
		break;

	/* Conditional branch. */
	case 16:
		inst.op.bc.bo = (uint8_t)((value >> 21) & 0x1f);
		inst.op.bc.bi = (uint8_t)((value >> 16) & 0x1f);
		inst.op.bc.bd = (uint16_t)((value >> 2) & 0x3fff);
		inst.op.bc.aa = ((value >> 1) & 1) != 0;
		inst.op.bc.lk = (value & 1) != 0;
		inst.opcode = BC;
		break;

	/* Branch to link register (extended opcode 16). */
	case 19:
		if (((value >> 1) & 0x3ff) == 16) {
			inst.op.bclr.bo = (uint8_t)((value >> 21) & 0x1f);
			inst.op.bclr.bi = (uint8_t)((value >> 16) & 0x1f);
			inst.op.bclr.bh = (uint8_t)((value >> 11) & 0x3);
			inst.op.bclr.lk = (value & 1) != 0;
			inst.opcode = BCLR;
		}
		break;

	/* D-form loads and stores. */
	case 32: case 33: case 34: case 35:
	case 36: case 37: case 38: case 39:
	case 40: case 41: case 42: case 43: case 44: case 45:
		inst.op.d.rx = (uint8_t)((value >> 21) & 0x1f);
		inst.op.d.ra = (uint8_t)((value >> 16) & 0x1f);
		inst.op.d.imm = (uint16_t)(value & 0xffff);
		switch (opcode) {
		case 32: inst.opcode = LWZ; break;
		case 33: inst.opcode = LWZU; break;
		case 34: inst.opcode = LBZ; break;
		case 35: inst.opcode = LBZU; break;
		case 36: inst.opcode = STW; break;
		case 37: inst.opcode = STWU; break;
		case 38: inst.opcode = STB; break;
		case 39: inst.opcode = STBU; break;
		case 40: inst.opcode = LHZ; break;
		case 41: inst.opcode = LHZU; break;
		case 42: inst.opcode = LHA; break;
		case 43: inst.opcode = LHAU; break;
		case 44: inst.opcode = STH; break;
		case 45: inst.opcode = STHU; break;
		}
		break;

	/* Register-register arithmetic, compares, indexed memory, and SPRs. */
	case 31: {
		uint16_t xo = (uint16_t)((value >> 1) & 0x3ff);
		uint8_t rd = (uint8_t)((value >> 21) & 0x1f);
		uint8_t ra = (uint8_t)((value >> 16) & 0x1f);
		uint8_t rb = (uint8_t)((value >> 11) & 0x1f);
		bool rc = (value & 1) != 0;

		switch (xo) {
		case 0: case 32: {
			bool l = ((value >> 21) & 1) != 0;
			bool reserved = ((value >> 22) & 1) != 0;
			if (!rc && !l && !reserved) {
				inst.op.cmp.bf = (uint8_t)((value >> 23) & 0x7);
				inst.op.cmp.ra = ra;
				inst.op.cmp.rb = rb;
				inst.opcode = (xo == 0) ? CMP : CMPL;
			}
			break;
		}
		case 40: inst.opcode = SUBF; break;
		case 266: inst.opcode = ADD; break;
		case 28: inst.opcode = AND; break;
		case 316: inst.opcode = XOR; break;
		case 444: inst.opcode = OR; break;
		case 23: inst.opcode = LWZX; break;
		case 55: inst.opcode = LWZUX; break;
		case 87: inst.opcode = LBZX; break;
		case 119: inst.opcode = LBZUX; break;
		case 279: inst.opcode = LHZX; break;
		case 311: inst.opcode = LHZUX; break;
		case 343: inst.opcode = LHAX; break;
		case 375: inst.opcode = LHAUX; break;
		case 151: inst.opcode = STWX; break;
		case 183: inst.opcode = STWUX; break;
		case 215: inst.opcode = STBX; break;
		case 247: inst.opcode = STBUX; break;
		case 407: inst.opcode = STHX; break;
		case 439: inst.opcode = STHUX; break;
		case 339: case 467: {
			uint16_t encoded = (uint16_t)((value >> 11) & 0x3ff);
			inst.op.spr.rx = rd;
			inst.op.spr.spr = (uint16_t)(((encoded & 0x1f) << 5) |
				((encoded >> 5) & 0x1f));
			inst.opcode = (xo == 339) ? MFSPR : MTSPR;
			break;
		}
		default:
			break;
		}

		if (inst.opcode == ADD || inst.opcode == SUBF ||
			inst.opcode == AND || inst.opcode == OR ||
			inst.opcode == XOR ||
			inst.opcode == LWZX || inst.opcode == LWZUX ||
			inst.opcode == LBZX || inst.opcode == LBZUX ||
			inst.opcode == LHZX || inst.opcode == LHZUX ||
			inst.opcode == LHAX || inst.opcode == LHAUX ||
			inst.opcode == STWX || inst.opcode == STWUX ||
			inst.opcode == STBX || inst.opcode == STBUX ||
			inst.opcode == STHX || inst.opcode == STHUX) {
			inst.op.x.rd = rd;
			inst.op.x.ra = ra;
			inst.op.x.rb = rb;
			inst.op.x.rc = rc;
		}
		break;
	}
	}

	return inst;
}
