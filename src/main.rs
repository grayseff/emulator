mod cpu;
mod memory; 
mod instruction;

use crate::instruction::decode;

fn main() {
    let mut cpu = cpu::Cpu::new();
    let mut ram = memory::Memory::new(1024);

    let add_instruction = // add to gpr3 content of gpr 20 and gpr 5
        (31 << 26) | (3 << 21) | (20 <<16) | (5 << 11) | (266 << 1);
    let addi_instruction = // add to gpr 6 the content of gpr 3 and value 100
        (14 << 26) | (6 << 21) | (3 << 16) | 100; 
    let subf_instruction = // subtract contents of 5 from 20 and put into 7
        (31 << 26) | (7 << 21) | (5 << 16) | (20 << 11) | (40 << 1);
    let and_instruction =
        (31 << 26) | (8 << 21) | (21 << 16) | (22 << 11) | (28 << 1);
    let or_instruction =
        (31 << 26) | (9 << 21) | (21 << 16) | (22 << 11) | (444 << 1);
    let xor_instruction =
        (31 << 26) | (10 << 21) | (21 << 16) | (22 << 11) | (316 << 1);
    let branch_instruction =
        (18 << 26) | (10 << 2);
    let cmp_lt =
        (31 << 26) | (0 << 21) | (21 << 16) | (22 << 11);

    let cmp_gt =
        (31 << 26) | (4 << 21) | (20 << 16) | (5 << 11);

    let cmp_eq =
        (31 << 26) | (8 << 21) | (20 << 16) | (20 << 11);



    cpu.gpr[20] = 50;
    cpu.gpr[5] = 25;

    cpu.gpr[21] = 0b1010;
    cpu.gpr[22] = 0b1100;
    

    ram.write32(0x40, add_instruction);
    ram.write32(0x44, addi_instruction);
    ram.write32(0x48, subf_instruction);
    ram.write32(0x4c, and_instruction);
    ram.write32(0x50, or_instruction);
    ram.write32(0x54, xor_instruction);
    ram.write32(0x58, branch_instruction);
    ram.write32(0x80, cmp_lt);
    ram.write32(0x84, cmp_gt);
    ram.write32(0x88, cmp_eq);


    cpu.pc = 0x40;
    
    for _ in 0..10 {
        cpu.step(&ram);
    }

    assert_eq!(cpu.gpr[3],75);
    assert_eq!(cpu.gpr[6],175);
    assert_eq!(cpu.gpr[7],25);
    assert_eq!(cpu.gpr[8], 0b1000);
    assert_eq!(cpu.gpr[9], 0b1110);
    assert_eq!(cpu.gpr[10], 0b0110);
    println!("cmp reads {:#010X}",cpu.cr);
}
