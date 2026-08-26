#include "cpu.h"


void cpu_init(CPU *cpu)
{
    cpu->pc = 0;
    for (int i = 0; i<32 ; i++){
        cpu->gpr[i] = 0;
    }

   return ; 
}

uint32_t cpu_fetch(CPU *cpu , uint8_t *ram){
   uint32_t value = ((uint32_t)ram[cpu->pc] <<24) |
                    ((uint32_t)ram[cpu->pc+1] <<16) |
                    ((uint32_t)ram[cpu->pc+2] <<8) |
                    ram[cpu->pc+3];
    cpu->pc +=4;
   return value;
}


