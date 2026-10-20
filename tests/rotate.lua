-- rlwinm: rotate left immediate, then mask
cpu.gpr[3] = 0x12345678
write32(0x00, 0x5464403E)
assert(step() == 0)
assert(cpu.gpr[4] == 0x34567812)

-- rlwnm: rotate left by register, then mask
cpu.gpr[3] = 0x12345678
cpu.gpr[5] = 8
write32(0x04, 0x5C64283E)
assert(step() == 0)
assert(cpu.gpr[4] == 0x34567812)

-- rlwimi: rotate and insert masked bits
cpu.gpr[3] = 0x12345678
cpu.gpr[4] = 0xFFFF0000
write32(0x08, 0x5064043E)

assert(step() == 0)
assert(cpu.gpr[4] == 0xFFFF5678)

-- rlwinm: wraparound mask
cpu.gpr[3] = 0x12345678
write32(0x0C, 0x54640706)
assert(step() == 0)
assert(cpu.gpr[4] == 0x10000008)

