setlr(0x12345678)
write32(0x00, 0x7C8802A6) -- mflr r4

assert(step() == 0)
assert(getreg(4) == 0x12345678)
assert(getpc() == 0x04)
resetcpu()

setreg(3, 0x12345678)
write32(0x00, 0x7C6903A6) -- mtctr r3

assert(step() == 0)
assert(getctr() == 0x12345678)
assert(getpc() == 0x04)
