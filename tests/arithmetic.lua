-- Initial register state
setreg(20, 50)
setreg(5, 25)
-- PowerPC instructions

write32(0x00, 0x7C742A14) -- add   r3,r20,r5
write32(0x04, 0x38C30064) -- addi  r6,r3,100
write32(0x08, 0x7CE5A050) -- subf  r7,r20,r5
write32(0x0C, 0x3D140001) -- addis r8,r20,1
write32(0x10, 0x7C742A15) -- add.  r3,r20,r5

-- Execute five instructions
for i = 1, 5 do
    assert(step() == 0)
end
-- Verify results
assert(getreg(3) == 75)
assert(getreg(6) == 175)
assert(getreg(7) == 25)
assert(getreg(8) == 65586)
assert((getcr() >> 28) & 0xF == 0x4) -- CR0 = GT
