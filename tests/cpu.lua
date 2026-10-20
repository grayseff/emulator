cpu.pc = 0x100
cpu.cr = 0x80000000
cpu.lr = 0x20
cpu.ctr = 42
cpu.xer = 0x80000000

assert(cpu.pc == 0x100)
assert(cpu.cr == 0x80000000)
assert(cpu.lr == 0x20)
assert(cpu.ctr == 42)
assert(cpu.xer == 0x80000000)

resetcpu()

assert(cpu.pc == 0)
assert(cpu.cr == 0)
assert(cpu.lr == 0)
assert(cpu.ctr == 0)
assert(cpu.xer == 0)

-- General-purpose registers
cpu.gpr[0] = 123
cpu.gpr[3] = 0x12345678
cpu.gpr[31] = 0xFFFFFFFF

assert(cpu.gpr[0] == 123)
assert(cpu.gpr[3] == 0x12345678)
assert(cpu.gpr[31] == 0xFFFFFFFF)

-- Invalid register indices
assert(not pcall(function()
    cpu.gpr[32] = 1
end))

assert(not pcall(function()
    return cpu.gpr[-1]
end))

-- The register array itself cannot be replaced
assert(not pcall(function()
    cpu.gpr = 42
end))

-- Reset must clear the underlying C registers
resetcpu()

assert(cpu.gpr[0] == 0)
assert(cpu.gpr[3] == 0)
assert(cpu.gpr[31] == 0)
