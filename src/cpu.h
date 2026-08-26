#ifndef CPU_H
#define CPU_H

#include <stdint.h>


typedef struct {
    uint32_t gpr[32];
    uint32_t pc;

} CPU;
typedef enum {
    INST_UNKNOWN,
    INST_ADDI,
    INST_ADD
} InstructionType;

typedef struct {
    InstructionType type;

    uint32_t opcode;
    uint32_t rd;
    uint32_t ra;
    uint32_t rb;
    uint32_t xo;
    int32_t immediate;
} Instruction;


void cpu_init(CPU *cpu);
uint32_t cpu_fetch(CPU *cpu, uint8_t *ram);
void cpu_addi(CPU *cpu, uint32_t rd, uint32_t ra, int32_t immediate);
void cpu_add(CPU *cpu, uint32_t rd, uint32_t ra, uint32_t rb);
Instruction cpu_decode(uint32_t value);

#endif
