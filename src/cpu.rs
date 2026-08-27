use crate::memory::Memory;
use crate::instruction::{Instruction,decode};


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

    pub fn step(&mut self, memory: &Memory) {
        let value = memory.read32(self.pc);
        let instruction = decode(value);
        let pc_changed = self.execute(instruction);

        if !pc_changed {
            self.pc += 4
        }
    }
// instruction set
    pub fn execute(&mut self, instruction: Instruction) -> bool {
        match instruction{
            Instruction::Add {rd, ra, rb }=> {
                self.add(rd, ra, rb);
                false
            }
            Instruction::Addi { rd, ra, immediate }=> {
                self.addi(rd, ra, immediate);
                false
            }
            Instruction::Subf { rd, ra, rb } => {
                self.subf(rd, ra, rb); 
                false
            }

            Instruction::And { rd, ra, rb } => {
                self.and(rd, ra, rb);
                false
            }

            Instruction::Or { rd, ra, rb } =>{
                self.or(rd, ra, rb);
                false
            }

            Instruction::Xor { rd, ra, rb } =>{
                self.xor(rd, ra, rb);
                false
            }

            Instruction::Cmp { bf, ra, rb } =>{
                self.cmp(bf, ra, rb);
                false
            }

            Instruction::B { li, aa, lk } =>{
                self.branch(li, aa, lk);
                true
            }
            Instruction::Unknown => {
                println!("unknown instruction:");
                    false
            }
        }
    }
    fn add(&mut self, rd: usize, ra: usize, rb: usize) {

        self.gpr[rd] = 
            self.gpr[ra].wrapping_add(self.gpr[rb]);
    }
    fn addi(&mut self, rd:usize, ra:usize, immediate:i32) {
        self.gpr[rd] = 
            self.gpr[ra].wrapping_add(immediate as u32);
    }
    fn subf(&mut self,rd:usize, ra:usize, rb:usize){
        self.gpr[rd] = 
            self.gpr[rb].wrapping_sub(self.gpr[ra]);
    }
    fn and(&mut self,rd:usize, ra:usize, rb:usize) {
        self.gpr[rd] = self.gpr[ra] & self.gpr[rb];
    }
    fn or(&mut self,rd:usize, ra:usize, rb:usize) {
        self.gpr[rd] = self.gpr[ra] | self.gpr[rb];
    } 
    fn xor(&mut self, rd:usize, ra:usize, rb:usize) {
        self.gpr[rd] = self.gpr[ra] ^ self.gpr[rb];
    }
    fn cmp(&mut self, bf:usize, ra:usize, rb:usize){
        let shift = (7 - bf) * 4;

        let result = if (self.gpr[ra] as i32) < (self.gpr[rb] as i32) {
            0b1000 
        } else if (self.gpr[ra] as i32) > (self.gpr[rb] as i32) {
            0b0100
        } else {
            0b0010
        };

        self.cr &= !(0xF << shift);
        self.cr |= result << shift;
    }
    fn branch(&mut self, li:i32, aa:bool, lk:bool) {
        let displacement = li << 2;
        if aa {
            self.pc = displacement as u32;
        } else {
            self.pc = self.pc.wrapping_add(displacement as u32);
        }
        // TODO: implement LK/LR
    }
}
