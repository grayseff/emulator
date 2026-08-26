#ifndef MEMORY_H
#define MEMORY_H


#include <stdint.h>



void ram_write32(uint8_t *ram, uint32_t address, uint32_t value);
uint32_t ram_read32(uint8_t *ram, uint32_t address);
#endif
