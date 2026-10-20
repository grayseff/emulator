cpu.gpr[3] = 0xFFFFFFFB -- -5 as a signed 32-bit integer
cpu.gpr[4] = 10

write32(0x00, 0x2C03FFF6) -- cmpwi cr0,r3,-10
write32(0x04, 0x28040014) -- cmplwi cr0,r4,20

assert(step() == 0)
assert((cpu.cr >> 28) & 0xF == 0x4) -- -5 > -10

assert(step() == 0)
assert((cpu.cr >> 28) & 0xF == 0x8) -- 10 < 20

assert(cpu.pc == 0x08)
resetcpu()

cpu.gpr[3] = 0xFFFFFFFF
cpu.gpr[4] = 1

write32(0x00, 0x7C032040) -- cmplw cr0,r3,r4

assert(step() == 0)
assert((cpu.cr >> 28) & 0xF == 0x4) -- unsigned GT
assert(cpu.pc == 0x04)

resetcpu()

cpu.xer = 0x80000000        -- XER.SO = 1
cpu.gpr[20] = 50
write32(0x00, 0x7C14A000) -- cmpw cr0,r20,r20

assert(step() == 0)
assert((cpu.cr >> 28) & 0xF == 0x3) -- EQ | SO
assert(cpu.pc == 0x04)
