
use emulator::cpu::{Cpu, XER_CA, XER_OV, XER_SO};
use emulator::memory::Memory;
use emulator::instruction::{Instruction,decode};

fn test_loader() -> (Cpu, Memory) {
    let mut cpu = Cpu::new();
    let mut ram = Memory::new(64);

    cpu.gpr[20] = 50;
    cpu.gpr[5] = 25;
    cpu.gpr[21] = 0b1010;
    cpu.gpr[22] = 0b1100;

    (cpu,ram)
    
}
#[test]
fn test_rlwinm() {
    let (mut cpu, mut ram) = test_loader();

    cpu.gpr[3] = 0x12345678;

    // rlwinm r4,r3,8,0,31
    // Full mask: simply rotate r3 left by 8
    ram.write32(0x00, 0x5464403E);

    cpu.step(&mut ram);

    assert_eq!(cpu.gpr[4], 0x34567812);
    assert_eq!(cpu.pc, 0x04);
}
#[test]
fn test_rlwnm() {
    let (mut cpu, mut ram) = test_loader();

    cpu.gpr[3] = 0x12345678;
    cpu.gpr[5] = 8;

    // rlwnm r4,r3,r5,0,31
    ram.write32(0x00, 0x5C64283E);

    cpu.step(&mut ram);

    assert_eq!(cpu.gpr[4], 0x34567812);
    assert_eq!(cpu.pc, 0x04);
}
#[test]
fn test_rlwimi() {
    let (mut cpu, mut ram) = test_loader();

    cpu.gpr[3] = 0x12345678;
    cpu.gpr[4] = 0xFFFF0000;

    // rlwimi r4,r3,0,16,31
    ram.write32(0x00, 0x5064043E);

    cpu.step(&mut ram);

    assert_eq!(cpu.gpr[4], 0xFFFF5678);
    assert_eq!(cpu.pc, 0x04);
}
#[test]
fn test_rlwinm_wraparound_mask() {
    let (mut cpu, mut ram) = test_loader();

    cpu.gpr[3] = 0x12345678;

    // rlwinm r4,r3,0,28,3
    // mask = 0xF000000F
    ram.write32(0x00, 0x54640706);

    cpu.step(&mut ram);

    assert_eq!(cpu.gpr[4], 0x10000008);
    assert_eq!(cpu.pc, 0x04);
}
