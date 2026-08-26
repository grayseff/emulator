#include <stdio.h>
#include <stdint.h>
#include "cpu.h"
#include "memory.h"



int main(void)
{
    CPU cpu;
    cpu_init(&cpu);

    uint32_t instruct1 = 
            (31  << 26) |
            (3  << 21) |
            (20 << 16) |
            (5  << 11) |
            (266 << 1);
    uint8_t ram[1024];
    cpu.pc = 0x00000040; 
    ram_write32(ram, 0x40, instruct1);   
    cpu.gpr[20] = 100;
    cpu.gpr[5] = 50;
    
    uint32_t value = cpu_fetch(&cpu, ram);
    
    Instruction instruction = cpu_decode(instruct1);

	printf("Instruction = 0x%08X\n", value);
	printf("Opcode      = %u\n", instruction.opcode);
	printf("rD          = r%u\n", instruction.rd);
	printf("rA          = r%u\n", instruction.ra);
	printf("rb   = %u\n", instruction.rb);
    printf("XO         = %u\n", instruction.xo);
    
    cpu_add(&cpu, instruction.rd , instruction.ra , instruction.rb);
    printf("value at %u is now %u", instruction.rd , cpu.gpr[instruction.rd]);
    return 0;
}
