use emulator::cpu::Cpu;
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
fn arithmetic() {
    let (mut cpu, mut ram) = test_loader();
    ram.write32(0x00 , 0x7C742A14); // add r3,r20,r5 
    ram.write32(0x04, 0x38c30064); //addi r6,r3,100
    ram.write32(0x08, 0x7CE5a050); // subf r7,r20,r5
    for _ in 0..3{
        cpu.step(&ram);
    }
    assert_eq!(cpu.gpr[3],75);
    assert_eq!(cpu.gpr[6],175);
    assert_eq!(cpu.gpr[7],25);
    assert_eq!(cpu.pc,12);
}

#[test]
fn logical_and_compare() {
    let (mut cpu, mut ram) = test_loader();
    
    ram.write32(0x00, 0x7d15b038); // and
    ram.write32(0x04, 0x7d35b378); // or
    ram.write32(0x08, 0x7d55b278); // xor
    ram.write32(0x0C, 0x7c15b000); // cmp <
    ram.write32(0x10, 0x7c942800); // cmp >
    ram.write32(0x14, 0x7d14a000); // cmp =

    for _ in 0..6 {
        cpu.step(&ram);
    }

    // assert CR fields
    assert_eq!(cpu.cr, 0x8420_0000);
    assert_eq!(cpu.gpr[8], 8);
    assert_eq!(cpu.gpr[9], 14);
    assert_eq!(cpu.gpr[10], 6);
    assert_eq!(cpu.pc, 24);

}

#[test]
fn branch_test() {
    let (mut cpu, mut ram) = test_loader();
    
    ram.write32(0x00, 0x4800001a); // B,AA=1,LI=6
    

    ram.write32(0x18, 0x48000018); // B, AA=0, LI=6


    ram.write32(0x30, 0x7c742a14);

    cpu.step(&ram);
    assert_eq!(cpu.pc, 0x18);
    cpu.step(&ram);
    assert_eq!(cpu.pc, 0x30);
    cpu.step(&ram);
    assert_eq!(cpu.gpr[3], 75);
    assert_eq!(cpu.pc,0x34);
}
