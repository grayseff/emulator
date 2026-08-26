#include <stdio.h>
#include <stdint.h>
#include "cpu.h"

int main(void)
{
    CPU cpu;
    cpu_init(&cpu);

    uint32_t instruction = 
            (4  << 26) |
            (3  << 21) |
            (20 << 16) |
            42;

    printf("Instruction = 0x%08X\n", instruction);

    uint8_t ram[1024];
    cpu.pc = 0x00000040; 
    ram[0] = 0x12;
    ram[0x40] = 0x10;
    ram[0x41] = 0x74;
    ram[0x42] = 0x00;
    ram[0x43] = 0x2A;
    
    cpu.gpr[20] = 100;

    uint32_t value = cpu_fetch(&cpu, ram);
    uint32_t opcode = value >> 26;
    uint32_t rd = (value >> 21) & 0x1F;
    uint32_t ra = (value >> 16) & 0x1F;
    uint32_t immediate = value & 0xFFFF;
    printf("Opcode is 0x%08X\n", opcode);

printf("Instruction = 0x%08X\n", value);
printf("Opcode      = %u\n", opcode);
printf("rD          = r%u\n", rd);
printf("rA          = r%u\n", ra);
printf("Immediate   = %u\n", immediate);

    cpu.gpr[rd] = cpu.gpr[ra] + immediate;

printf("r17 = %u\n", cpu.gpr[rd]);

    return 0;
}
