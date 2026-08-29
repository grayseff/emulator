use emulator::cpu::Cpu;
use emulator::memory::Memory;

fn test_loader() -> (Cpu, Memory) {
    let mut cpu = Cpu::new();
    let ram = Memory::new(128);

    cpu.gpr[1] = 0x40;
    cpu.gpr[5] = 0x12345678;
    cpu.gpr[6] = 0x0fff8001;
   return (cpu, ram) 
}


#[test]
fn test_loadstore() {
    let (mut cpu, mut ram) = test_loader();
    ram.write32(0x00, 0x90a10000); // STW
    ram.write32(0x04, 0x94a10008); // STWU
    ram.write32(0x08, 0x98c10004); // STB
    ram.write32(0x0C, 0x9cc1000c); // STBU
    ram.write32(0x10, 0xb0c10002); // STH
    ram.write32(0x14, 0xb4c10004); // STHU
    ram.write32(0x18, 0x81010000); // LWZ
    ram.write32(0x1c, 0x85210008); // LWZU
    ram.write32(0x20, 0x89410004); // LBZ
    ram.write32(0x24, 0x8d61000c); // LBZU
    ram.write32(0x28, 0xa1810002); // LHZ
    ram.write32(0x2c, 0xa5a10004); // LHZU
    ram.write32(0x30, 0xa9c1fffe); // LHA
    ram.write32(0x34, 0xade10000); // LHAU

    for _ in 0..6 {
        cpu.step(&mut ram);
    }
    assert_eq!(ram.read32(0x40), 0x12345678);
    assert_eq!(ram.read32(0x48), 0x12345678);

    assert_eq!(ram.read8(0x4C), 0x01);
    assert_eq!(ram.read8(0x54), 0x01);

    assert_eq!(ram.read16(0x56), 0x8001);
    assert_eq!(ram.read16(0x58), 0x8001);

    assert_eq!(cpu.gpr[1], 0x58);
    cpu.gpr[1] = 0x40; //reset stack pointer to 64


    for _ in 0..8 {
        cpu.step(&mut ram);
    }

    assert_eq!(cpu.gpr[8],  0x12345678);
    assert_eq!(cpu.gpr[9],  0x12345678);
    assert_eq!(cpu.gpr[10], 0x00000001);
    assert_eq!(cpu.gpr[11], 0x00000001);
    assert_eq!(cpu.gpr[12], 0x00008001);
    assert_eq!(cpu.gpr[13], 0x00008001);
    assert_eq!(cpu.gpr[14], 0xFFFF8001);
    assert_eq!(cpu.gpr[15], 0xFFFF8001);
    
    assert_eq!(cpu.gpr[1], 0x58);
}

