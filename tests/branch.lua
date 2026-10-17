setreg(20, 50)
setreg(5, 25)

write32(0x00, 0x4800001A) -- b 0x18 (absolute)
write32(0x18, 0x48000018) -- b +0x18 (relative)
write32(0x30, 0x7C742A14) -- add r3,r20,r5

assert(step() == 0)
assert(getpc() == 0x18)

assert(step() == 0)
assert(getpc() == 0x30)

assert(step() == 0)
assert(getreg(3) == 75)
assert(getpc() == 0x34)

resetcpu()

setcr(0x80000000)          -- CR0.LT = 1
write32(0x00, 0x41800008) -- bc 12,0,+8

assert(step() == 0)
assert(getpc() == 0x08)
resetcpu()

setcr(0)                  -- CR0.LT = 0
write32(0x00, 0x41800008) -- bc 12,0,+8

assert(step() == 0)
assert(getpc() == 0x04)
resetcpu()

setlr(0x20)
write32(0x00, 0x4E800020) -- blr (bclr 20,0)

assert(step() == 0)
assert(getpc() == 0x20)


