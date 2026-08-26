mod cpu;
mod memory; 
mod instruction;

use crate::instruction::decode;

fn main() {
    let mut cpu = cpu::Cpu::new();
    let mut ram = memory::Memory::new(1024);

    let add_instruct = // add to gpr3 content of gpr 20 and gpr 5
        (31 << 26) | (3 << 21) | (20 <<16) | (5 << 11) | (266 << 1);
    let addi_instruct = // add to gpr 6 the content of gpr 3 and value 100
        (14 << 26) | (6 << 21) | (3 << 16) | 100; 
    let subf_instruction = // subtract contents of 5 from 20 and put into 7
        (31 << 26) | (7 << 21) | (5 << 16) | (20 << 11) | (40 << 1);
    ram.write32(0x40, add_instruct);
    ram.write32(0x44, addi_instruct);
    ram.write32(0x48, subf_instruction);
    cpu.gpr[20] = 50;
    cpu.gpr[5] = 25;
    cpu.pc = 0x40;
    
    let value = cpu.fetch(&ram);
    let instruct = decode(value);
    cpu.execute(instruct);

    let value = cpu.fetch(&ram);
    let instruct = decode(value);
    cpu.execute(instruct);
    
    let value = cpu.fetch(&ram);
    let instruct = decode(value);
    cpu.execute(instruct);

    assert_eq!(cpu.gpr[3],75);
    assert_eq!(cpu.gpr[6],175);
    assert_eq!(cpu.gpr[7],25);
     
}
