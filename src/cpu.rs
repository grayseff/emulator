use crate::memory::Memory;
use crate::instruction::{Instruction,InstructionType};


pub struct Cpu {
    pub gpr: [u32; 32],
    pub pc: u32,

    pub cr: u32,
    pub lr: u32,
    pub ctr: u32,
    pub xer: u32,
}

impl Cpu {
    pub fn new() -> Self {
        Self {
            gpr:  [0; 32],
            pc: 0,

            cr: 0,
            lr: 0,
            ctr: 0,
            xer: 0,
        }
    }

    pub fn fetch(&mut self, memory: &Memory) -> u32 {
        let value = memory.read32(self.pc);
        self.pc += 4;
        value
    }

// instruction set
    pub fn execute(&mut self, instruction: Instruction){
        match instruction.instruction_type{
            InstructionType::Add => self.add(instruction),
            InstructionType::Addi => self.addi(instruction),
            InstructionType::Subf => self.subf(instruction),
            InstructionType::Unknown => {
                println!("unknown instruction: opcode {} xo {}", 
                    instruction.opcode, instruction.xo);
            }
        }
    }
    fn add(&mut self, instruction: Instruction) {
        let rd = instruction.rd as usize;
        let ra = instruction.ra as usize;
        let rb = instruction.rb as usize;

        self.gpr[rd] = 
            self.gpr[ra].wrapping_add(self.gpr[rb]);
    }
    fn addi(&mut self, instruction: Instruction) {
        let rd = instruction.rd as usize;
        let ra = instruction.ra as usize;
        self.gpr[rd] = 
            self.gpr[ra].wrapping_add(instruction.immediate as u32);
    }
    fn subf(&mut self, instruction: Instruction){
        let rd = instruction.rd as usize;
        let ra = instruction.ra as usize;
        let rb = instruction.rb as usize;

        self.gpr[rd] = 
            self.gpr[rb].wrapping_sub(self.gpr[ra]);
    }


}
