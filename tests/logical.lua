-- Initial register state
setreg(20, 50)
setreg(5, 25)
setreg(21, 0xA)
setreg(22, 0xC)

-- Register logical operations
write32(0, 0x7EA8B038) -- and r8,r21,r22
write32(0x04, 0x7EA9B378) -- or  r9,r21,r22  = 14
write32(0x08, 0x7EAAB278) -- xor r10,r21,r22 = 6
-- Comparisons
write32(0x0C, 0x7C15B000) -- cmp
write32(0x10, 0x7C942800) -- cmp
write32(0x14, 0x7D14A000) -- cmp

-- Immediate logical operations
write32(0x18, 0x628B000C) -- ori    r11,r20,12
write32(0x1C, 0x668C000C) -- oris r12,r20,12
write32(0x20, 0x6A8D000C) -- xori r13,r20,12
write32(0x24, 0x6E8E000C) -- xoris r14,r20,12
write32(0x28, 0x728F0032) -- andi. r15,r20,0x32
write32(0x2C, 0x768F0032) -- andis. r15,r20,0x32


-- First ten instructions
for i = 1, 10 do
    assert(step() == 0)
end

assert(getreg(8) == 8)
assert(getreg(9) == 14)
assert(getreg(10) == 6)
assert(getreg(11) == 62)
assert(getreg(12) == 786482)
assert(getreg(13) == 62)
assert(getreg(14) == 786482)

assert(getcr() == 0x84200000)
-- ANDI.
assert(step() == 0)
assert(getreg(15) == 50)
assert((getcr() >> 28) & 0xF == 0x4) -- CR0 = GT
-- ANDIS.
assert(step() == 0)
assert(getreg(16) == 0)
assert((getcr() >> 28) & 0xF == 0x2) -- CR0 = EQ
