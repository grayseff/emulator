#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
typedef enum {
	ADD,
	ADDI,
	ADDIS,
	SUBF,

	AND,
	ANDI,
	ANDIS,
	OR,
	ORI,
	ORIS,
	XOR,
	XORI,
	XORIS,

	CMP,
	CMPL,
	CMPI,
	CMPLI,

	B,
	BC,
	BCLR,

	LBZ,
	LBZU,
	LBZX,
	LBZUX,

	LHZ,
	LHZU,
	LHZX,
	LHZUX,

	LHA,
	LHAU,
	LHAX,
	LHAUX,

	LWZ,
	LWZU,
	LWZX,
	LWZUX,

	STB,
	STBU,
	STBX,
	STBUX,

	STH,
	STHU,
	STHX,
	STHUX,

	STW,
	STWU,
	STWX,
	STWUX,

	MTSPR,
	MFSPR,

	RLWINM,
	RLWIMI,
	RLWNM,

	UNKNOWN
} Opcode;

typedef struct {
	uint8_t rx;
	uint8_t ra;
	uint16_t imm;
} DForm;

typedef struct {
	uint8_t rd;
	uint8_t ra;
	uint8_t rb;
	bool rc;
} XForm;

typedef struct {
	uint8_t bf;
	uint8_t ra;
	uint8_t rb;
} CmpForm;

typedef struct {
	uint8_t bf;
	uint8_t ra;
	uint16_t imm;
} CmpiForm;

typedef struct {
	uint8_t rx;
	uint16_t spr;
} SprForm;

typedef struct {
	uint8_t bo;
	uint8_t bi;
	uint16_t bd;
	bool aa;
	bool lk;
} BcForm;

typedef struct {
	uint32_t li;
	bool aa;
	bool lk;
} BForm;

typedef struct {
	uint8_t bo;
	uint8_t bi;
	uint8_t bh;
	bool lk;
} BclrForm;

typedef struct {
	uint8_t rs;
	uint8_t ra;
	uint8_t rx;
	uint8_t mb;
	uint8_t me;
	bool rc;
} RlwForm;

typedef union {
	DForm d;
	XForm x;
	CmpForm cmp;
	CmpiForm cmpi;
	SprForm spr;
	BcForm bc;
	BForm b;
	BclrForm bclr;
	RlwForm rlw;
} Operands;

typedef struct {
	Opcode opcode;
	Operands op;
} Instruction;

#endif
