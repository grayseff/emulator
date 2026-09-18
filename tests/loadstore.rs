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
#[test]
fn test_loadstore_indexed() {
    let (mut cpu, mut ram) = test_loader();

    // Indexed offsets
    cpu.gpr[16] = 0x00;
    cpu.gpr[17] = 0x08;
    cpu.gpr[18] = 0x04;
    cpu.gpr[19] = 0x0C;
    cpu.gpr[20] = 0x02;
    cpu.gpr[21] = (-2i32) as u32;

    // Stores
    ram.write32(0x00, 0x7ca1812e); // STWX  r5,r1,r16
    ram.write32(0x04, 0x7ca1896e); // STWUX r5,r1,r17
    ram.write32(0x08, 0x7cc191ae); // STBX  r6,r1,r18
    ram.write32(0x0C, 0x7cc199ee); // STBUX r6,r1,r19
    ram.write32(0x10, 0x7cc1a32e); // STHX  r6,r1,r20
    ram.write32(0x14, 0x7cc1936e); // STHUX r6,r1,r18

    // Loads
    ram.write32(0x18, 0x7d01802e); // LWZX  r8,r1,r16
    ram.write32(0x1C, 0x7d21886e); // LWZUX r9,r1,r17
    ram.write32(0x20, 0x7d4190ae); // LBZX  r10,r1,r18
    ram.write32(0x24, 0x7d6198ee); // LBZUX r11,r1,r19
    ram.write32(0x28, 0x7d81a22e); // LHZX  r12,r1,r20
    ram.write32(0x2C, 0x7da1926e); // LHZUX r13,r1,r18
    ram.write32(0x30, 0x7dc1aaae); // LHAX  r14,r1,r21
    ram.write32(0x34, 0x7de182ee); // LHAUX r15,r1,r16

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

    // Reset base register before testing loads
    cpu.gpr[1] = 0x40;

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
