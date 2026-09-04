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
fn arithmetic() {
    let (mut cpu, mut ram) = test_loader();
    ram.write32(0x00 , 0x7C742A14); // add r3,r20,r5 
    ram.write32(0x04, 0x38c30064); //addi r6,r3,100
    ram.write32(0x08, 0x7CE5a050); // subf r7,r20,r5
    ram.write32(0x0c, 0x3d140001); //addis r8,r20,1
    ram.write32(0x10, 0x7C742A15); // add. r3,r20,r5
    for _ in 0..5{
        cpu.step(&mut ram);
    }
    assert_eq!(cpu.gpr[3],75);
    assert_eq!(cpu.gpr[6],175);
    assert_eq!(cpu.gpr[7],25);
    assert_eq!(cpu.gpr[8],65586);
    assert_eq!((cpu.cr >> 28) & 0xF, 0b0100); // GT
    assert_eq!(cpu.pc,20);
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
    ram.write32(0x18, 0x6174000C); // ori  r11,r20,12
    ram.write32(0x1C,0x6594000C); // oris r12,r20,12
    ram.write32(0x20, 0x69B4000C); // xori r13,r20,12
    ram.write32(0x24, 0x6DD4000C); // xoris r14,r20,12


    ram.write32(0x28, 0x71F40032); // andi. r15,r20,0x32
    ram.write32(0x2C, 0x77F40032); // andis. r16,r20,0x32    


    for _ in 0.. 10{
        cpu.step(&mut ram);
    }

    // assert CR fields
    assert_eq!(cpu.cr, 0x8420_0000);
    assert_eq!(cpu.gpr[8], 8);
    assert_eq!(cpu.gpr[9], 14);
    assert_eq!(cpu.gpr[10], 6);
    assert_eq!(cpu.pc, 40);
    assert_eq!(cpu.gpr[11], 62);
    assert_eq!(cpu.gpr[12], 786482);
    assert_eq!(cpu.gpr[13], 62);
    assert_eq!(cpu.gpr[14], 786482);



    cpu.step(&mut ram);
    assert_eq!(cpu.gpr[15], 50);
    assert_eq!((cpu.cr >> 28) & 0xF, 0b0100); // GT

    cpu.step(&mut ram); // ANDIS.
    assert_eq!(cpu.gpr[16], 0);
    assert_eq!((cpu.cr >> 28) & 0xF, 0b0010); // EQ

    assert_eq!(cpu.pc, 0x30);

}

#[test]
fn cmp_copies_xer_summary_overflow() {
    let (mut cpu, mut ram) = test_loader();
    cpu.set_xer_so(true);
    ram.write32(0x00, 0x7c14a000); // cmp cr0,r20,r20

    cpu.step(&mut ram);

    assert_eq!((cpu.cr >> 28) & 0xF, 0b0011); // EQ | SO
}

#[test]
fn xer_flag_helpers_preserve_other_bits() {
    let (mut cpu, _) = test_loader();
    cpu.xer = 0x1234_5678;

    cpu.set_xer_so(true);
    cpu.set_xer_ov(true);
    cpu.set_xer_ca(true);
    assert!(cpu.xer_so());
    assert!(cpu.xer_ov());
    assert!(cpu.xer_ca());
    assert_eq!(cpu.xer & (XER_SO | XER_OV | XER_CA), XER_SO | XER_OV | XER_CA);

    cpu.set_xer_ov(false);
    assert!(cpu.xer_so());
    assert!(!cpu.xer_ov());
    assert!(cpu.xer_ca());
}

#[test]
fn branch_test() {
    let (mut cpu, mut ram) = test_loader();
    
    ram.write32(0x00, 0x4800001a); // B,AA=1,LI=6
    

    ram.write32(0x18, 0x48000018); // B, AA=0, LI=6


    ram.write32(0x30, 0x7c742a14);

    cpu.step(&mut ram);
    assert_eq!(cpu.pc, 0x18);
    cpu.step(&mut ram);
    assert_eq!(cpu.pc, 0x30);
    cpu.step(&mut ram);
    assert_eq!(cpu.gpr[3], 75);
    assert_eq!(cpu.pc,0x34);
}
