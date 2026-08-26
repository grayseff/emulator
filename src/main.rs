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
    ram.write32(0x40, add_instruct);
    ram.write32(0x44, addi_instruct);
    cpu.gpr[20] = 50;
    cpu.gpr[5] = 25;
    cpu.pc = 0x40;
    
    let value = cpu.fetch(&ram);
    let instruct = decode(value);
    cpu.add(instruct);

    let value = cpu.fetch(&ram);
    let instruct = decode(value);
    cpu.addi(instruct);

    assert_eq!(cpu.gpr[3],75);
    assert_eq!(cpu.gpr[6],175);
    assert_eq!(cpu.pc,0x48);
     
}
