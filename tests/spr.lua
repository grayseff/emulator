cpu.lr = 0x12345678
write32(0x00, 0x7C8802A6) -- mflr r4

assert(step() == 0)
assert(cpu.gpr[4] == 0x12345678)
assert(cpu.pc == 0x04)
resetcpu()

cpu.gpr[3] = 0x12345678
write32(0x00, 0x7C6903A6) -- mtctr r3

assert(step() == 0)
assert(cpu.ctr == 0x12345678)
assert(cpu.pc == 0x04)
