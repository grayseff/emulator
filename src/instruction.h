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
	unsigned char rx;
	unsigned char ra;
	unsigned short imm;
} DForm;

typedef struct {
	unsigned char rd;
	unsigned char ra;
	unsigned char rb;
	bool rc;
} XForm;

typedef struct {
	unsigned char bf;
	unsigned char ra;
	unsigned char rb;
} CmpForm;

typedef struct {
	unsigned char bf;
	unsigned char ra;
	unsigned short imm;
} CmpiForm;

typedef struct {
	unsigned char rx;
	unsigned short spr;
} SprForm;

typedef struct {
	unsigned char bo;
	unsigned char bi;
	unsigned short bd;
	bool aa;
	bool lk;
} BcForm;

typedef struct {
	unsigned int li;
	bool aa;
	bool lk;
} BForm;

typedef struct {
	unsigned char bo;
	unsigned char bi;
	unsigned char bh;
	bool lk;
} BclrForm;

typedef struct {
	unsigned char rs;
	unsigned char ra;
	unsigned char rx;
	unsigned char mb;
	unsigned char me;
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
