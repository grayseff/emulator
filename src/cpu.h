#ifndef CPU_H
#define CPU_H

#include <stdint.h>


typedef struct {
    uint32_t gpr[32];
    uint32_t pc;

} CPU;

void cpu_init(CPU *cpu);
uint32_t cpu_fetch(CPU *cpu, uint8_t *ram);

#endif
