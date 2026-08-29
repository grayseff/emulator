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

    pub fn step(&mut self, memory: &mut Memory) {
        let value = memory.read32(self.pc);
        let instruction = decode(value);
        let pc_changed = self.execute(instruction, memory);

        if !pc_changed {
            self.pc += 4
        }
    }
// instruction set
    pub fn execute(&mut self, instruction: Instruction, memory: &mut Memory) -> bool {
        match instruction{
            Instruction::Add {rd, ra, rb }=> {
                self.gpr[rd] = 
            self.gpr[ra].wrapping_add(self.gpr[rb]);

                false
            }
            Instruction::Addi { rd, ra, immediate }=> {
                self.gpr[rd] = self.gpr[ra].wrapping_add(immediate as u32);
                false
            }
            Instruction::Addis { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra].wrapping_add((immediate << 16) as u32);
                false
            }
            Instruction::Subf { rd, ra, rb } => {
                self.gpr[rd] = self.gpr[rb].wrapping_sub(self.gpr[ra]);
                false
            }

            Instruction::And { rd, ra, rb } => {
                self.gpr[rd] = self.gpr[ra] & self.gpr[rb];
                false
            }
            Instruction::Andi { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] & immediate as u32;
                self.set_cr0(self.gpr[rd]);
                false
            }
            Instruction::Andis {rd, ra, immediate} => {
                self.gpr[rd] = self.gpr[ra] & ((immediate as u32) << 16);
                self.set_cr0(self.gpr[rd]);
                false
            }

            Instruction::Or { rd, ra, rb } =>{
                self.gpr[rd] = self.gpr[ra] | self.gpr[rb];
                false
            }

            Instruction::Ori { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] | immediate as u32;
                false
            }
            Instruction::Oris { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] | ((immediate as u32) << 16 ) ;
                false
            }

            Instruction::Xor { rd, ra, rb } =>{
                self.gpr[rd] = self.gpr[ra] ^ self.gpr[rb];
                false
            }

            Instruction::Xori { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] ^ immediate as u32;
                false
            }

            Instruction::Xoris { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] ^ ((immediate as u32) << 16);
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

            Instruction::Lwz { rd, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        self.gpr[rd] = memory.read32(address);
		        false
		    }
		
		    Instruction::Lwzu { rd, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        self.gpr[rd] = memory.read32(address);
		        self.gpr[ra] = address;
		        false
		    }
		
		    Instruction::Stw { rs, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        memory.write32(address, self.gpr[rs]);
		        false
		    }
		    
		    Instruction::Stwu { rs, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        memory.write32(address, self.gpr[rs]);
		        self.gpr[ra] = address;
		        false
		    } 
		    Instruction::Lbz { rd, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        self.gpr[rd] = memory.read8(address) as u32;
		        false
		    }
		    
		    Instruction::Lbzu { rd, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        self.gpr[rd] = memory.read8(address) as u32;
		        self.gpr[ra] = address;
		        false
		    }
		    
		    Instruction::Lhz { rd, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        self.gpr[rd] = memory.read16(address) as u32;
		        false
		    }
		    
		    Instruction::Lhzu { rd, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        self.gpr[rd] = memory.read16(address) as u32;
		        self.gpr[ra] = address;
		        false
		    }
		    
		    Instruction::Lha { rd, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        self.gpr[rd] = memory.read16(address) as i16 as i32 as u32;
		        false
		    }
		    
		    Instruction::Lhau { rd, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        self.gpr[rd] = memory.read16(address) as i16 as i32 as u32;
		        self.gpr[ra] = address;
		        false
		    }
		    
		    Instruction::Stb { rs, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        memory.write8(address, self.gpr[rs] as u8);
		        false
		    }
		    
		    Instruction::Stbu { rs, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        memory.write8(address, self.gpr[rs] as u8);
		        self.gpr[ra] = address;
		        false
		    }
		    
		    Instruction::Sth { rs, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        memory.write16(address, self.gpr[rs] as u16);
		        false
		    }
		    
		    Instruction::Sthu { rs, ra, immediate } => {
		        let address = self.gpr[ra].wrapping_add(immediate as u32);
		        memory.write16(address, self.gpr[rs] as u16);
		        self.gpr[ra] = address;
		        false
		    }




            Instruction::Unknown => {
                println!("unknown instruction:");
                    false
            }
        }
    }
    fn set_cr0(&mut self, result: u32) {
        // set CR0 from result
        let cr_val = if (result as i32) < 0 {
            0b1000
        } else if result != 0 {
            0b0100
        } else {
            0b0010
        };

        self.cr &= !(0xF << 28);
        self.cr |= cr_val << 28;

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
