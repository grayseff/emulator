#include "memory.h"


void ram_write32(uint8_t *ram, uint32_t address, uint32_t value)
{
    ram[address]     = (value >> 24) & 0xFF;
    ram[address + 1] = (value >> 16) & 0xFF;
    ram[address + 2] = (value >> 8)  & 0xFF;
    ram[address + 3] = value & 0xFF;
}
uint32_t ram_read32(uint8_t *ram, uint32_t address)
{
    return ((uint32_t)ram[address] << 24) |
           ((uint32_t)ram[address + 1] << 16) |
           ((uint32_t)ram[address + 2] << 8) |
           ram[address + 3];
}
