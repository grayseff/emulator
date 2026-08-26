#include "cpu.h"
#include "memory.h"
#include <stdio.h>

void cpu_init(CPU *cpu)
{
    cpu->pc = 0;
    for (int i = 0; i<32 ; i++){
        cpu->gpr[i] = 0;
    }

   return ; 
}

uint32_t cpu_fetch(CPU *cpu , uint8_t *ram){
   uint32_t value = ram_read32(ram, cpu->pc);
    cpu->pc +=4;
   return value;
}

void cpu_addi(CPU *cpu, uint32_t rd, uint32_t ra, int32_t immediate)
{
    cpu->gpr[rd] = cpu->gpr[ra] + immediate;
}
void cpu_add(CPU *cpu, uint32_t rd, uint32_t ra, uint32_t rb)
{
    cpu->gpr[rd] = cpu->gpr[ra] + cpu->gpr[rb];
}

Instruction cpu_decode(uint32_t value)
{
    Instruction instruction;
    instruction.opcode = value >> 26;
    switch (instruction.opcode){
        case 14:
            // D form
            printf("D form\n");
            instruction.type = INST_ADDI;
            instruction.rd = (value >> 21) & 0x1F;
            instruction.ra = (value >> 16) & 0x1F;
            instruction.immediate = value & 0xFFFF;

            break;

        case 31:
            // X form
            printf("X form\n");
            instruction.rd = (value >> 21) & 0x1f;
            instruction.ra = (value >> 16) & 0x1f;
            instruction.rb = (value >> 11) & 0x1f;
            instruction.xo = (value >> 1) & 0x3ff;
            if (instruction.xo == 266)
                instruction.type = INST_ADD;

            break;
        default:
            printf("unknown opcode \n");
            break;
    }
    return instruction;
}
