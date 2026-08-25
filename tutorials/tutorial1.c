#include <stdio.h>
#include <stdint.h>


int main(void)
{
    uint32_t gpr[32];
    uint32_t pc;
    uint8_t ram[1024];

    gpr[3] = 42;
    pc = 0x00000040;
    ram[0] = 0x12;
    ram[0x40] = 0x12;
    ram[0x41] = 0x34;
    ram[0x42] = 0x56;
    ram[0x43] = 0x78;

    uint32_t value = ((uint32_t)ram[pc] << 24) |
                     ((uint32_t)ram[pc+1] << 16) |
                     ((uint32_t)ram[pc+2] <<  8) |
                     ram[pc+3];

    printf("r3 = %u\n",gpr[3]);
    printf("PC= 0x%08X\n", pc);
    printf("Value = 0x%08X\n", value);
    printf(" RAM[0] = %u\n", ram[0]);

    return 0;
}
