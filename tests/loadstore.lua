cpu.gpr[1] = 0x40
cpu.gpr[5] = 0x12345678
cpu.gpr[6] = 0x0FFF8001

write32(0x00, 0x90A10000) -- stw
write32(0x04, 0x94A10008) -- stwu
write32(0x08, 0x98C10004) -- stb
write32(0x0C, 0x9CC1000C) -- stbu
write32(0x10, 0xB0C10002) -- sth
write32(0x14, 0xB4C10004) -- sthu

for i = 1, 6 do
    assert(step() == 0)
end

assert(read32(0x40) == 0x12345678)
assert(read32(0x48) == 0x12345678)
assert(read8(0x4C) == 1)
assert(read8(0x54) == 1)
assert(read16(0x56) == 0x8001)
assert(read16(0x58) == 0x8001)
assert(cpu.gpr[1] == 0x58)
cpu.gpr[1] = 0x40

write32(0x18, 0x81010000) -- lwz
write32(0x1C, 0x85210008) -- lwzu
write32(0x20, 0x89410004) -- lbz
write32(0x24, 0x8D61000C) -- lbzu
write32(0x28, 0xA1810002) -- lhz
write32(0x2C, 0xA5A10004) -- lhzu
write32(0x30, 0xA9C1FFFE) -- lha
write32(0x34, 0xADE10000) -- lhau

for i = 1, 8 do
    assert(step() == 0)
end

assert(cpu.gpr[8] == 0x12345678)
assert(cpu.gpr[9] == 0x12345678)
assert(cpu.gpr[10] == 1)
assert(cpu.gpr[11] == 1)
assert(cpu.gpr[12] == 0x8001)
assert(cpu.gpr[13] == 0x8001)
assert(cpu.gpr[14] == 0xFFFF8001)
assert(cpu.gpr[15] == 0xFFFF8001)
assert(cpu.gpr[1] == 0x58)

resetcpu()
assert(cpu.gpr[1] == 0)
assert(cpu.gpr[5] == 0)
assert(cpu.gpr[6] == 0)
assert(cpu.cr == 0)

cpu.gpr[1] = 0x40
cpu.gpr[5] = 0x12345678
cpu.gpr[6] = 0x0FFF8001

cpu.gpr[16] = 0
cpu.gpr[17] = 8
cpu.gpr[18] = 4
cpu.gpr[19] = 12
cpu.gpr[20] = 2
cpu.gpr[21] = 0xFFFFFFFE

write32(0x00, 0x7CA1812E) -- stwx
write32(0x04, 0x7CA1896E) -- stwux
write32(0x08, 0x7CC191AE) -- stbx
write32(0x0C, 0x7CC199EE) -- stbux
write32(0x10, 0x7CC1A32E) -- sthx
write32(0x14, 0x7CC1936E) -- sthux

for i = 1, 6 do
    assert(step() == 0)
end

assert(read32(0x40) == 0x12345678)
assert(read32(0x48) == 0x12345678)
assert(read8(0x4C) == 1)
assert(read8(0x54) == 1)
assert(read16(0x56) == 0x8001)
assert(read16(0x58) == 0x8001)
assert(cpu.gpr[1] == 0x58)

cpu.gpr[1] = 0x40

write32(0x18, 0x7D01802E) -- lwzx
write32(0x1C, 0x7D21886E) -- lwzux
write32(0x20, 0x7D4190AE) -- lbzx
write32(0x24, 0x7D6198EE) -- lbzux
write32(0x28, 0x7D81A22E) -- lhzx
write32(0x2C, 0x7DA1926E) -- lhzux
write32(0x30, 0x7DC1AAAE) -- lhax
write32(0x34, 0x7DE182EE) -- lhaux

for i = 1, 8 do
    assert(step() == 0)
end

assert(cpu.gpr[8] == 0x12345678)
assert(cpu.gpr[9] == 0x12345678)
assert(cpu.gpr[10] == 1)
assert(cpu.gpr[11] == 1)
assert(cpu.gpr[12] == 0x8001)
assert(cpu.gpr[13] == 0x8001)
assert(cpu.gpr[14] == 0xFFFF8001)
assert(cpu.gpr[15] == 0xFFFF8001)
assert(cpu.gpr[1] == 0x58)
