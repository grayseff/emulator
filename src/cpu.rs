use crate::memory::Memory;
use crate::instruction::{Instruction,InstructionType};


pub struct Cpu {
    pub gpr: [u32; 32],
    pub pc: u32,
}

impl Cpu {
    pub fn new() -> Self {
        Self {
            gpr:  [0; 32],
            pc: 0,
        }
    }

    pub fn fetch(&mut self, memory: &Memory) -> u32 {
        let value = memory.read32(self.pc);
        self.pc += 4;
        value
    }

// instruction set
    pub fn add(&mut self, instruction: Instruction) {
        let rd = instruction.rd as usize;
        let ra = instruction.ra as usize;
        let rb = instruction.rb as usize;

        self.gpr[rd] = 
            self.gpr[ra].wrapping_add(self.gpr[rb]);
    }
    pub fn addi(&mut self, instruction: Instruction) {
        let rd = instruction.rd as usize;
        let ra = instruction.ra as usize;
        self.gpr[rd] = 
            self.gpr[ra].wrapping_add(instruction.immediate as u32);
    }


}
