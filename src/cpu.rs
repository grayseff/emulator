use crate::memory::Memory;
use crate::instruction::{Instruction,decode};

pub const XER_SO: u32 = 1 << 31;
pub const XER_OV: u32 = 1 << 30;
pub const XER_CA: u32 = 1 << 29;

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
        self.pc = self.pc.wrapping_add(4);
        self.execute(instruction, memory);

        
    }

    fn d_form_address(&self, ra: usize, immediate: i32) -> u32 {
        let base = if ra == 0 { 0 } else { self.gpr[ra] };
        base.wrapping_add(immediate as u32)
    }

    pub fn xer_so(&self) -> bool {
        self.xer & XER_SO != 0
    }

    pub fn set_xer_so(&mut self, value: bool) {
        self.set_xer_flag(XER_SO, value);
    }

    pub fn xer_ov(&self) -> bool {
        self.xer & XER_OV != 0
    }

    pub fn set_xer_ov(&mut self, value: bool) {
        self.set_xer_flag(XER_OV, value);
    }

    pub fn xer_ca(&self) -> bool {
        self.xer & XER_CA != 0
    }

    pub fn set_xer_ca(&mut self, value: bool) {
        self.set_xer_flag(XER_CA, value);
    }

    fn set_xer_flag(&mut self, flag: u32, value: bool) {
        if value {
            self.xer |= flag;
        } else {
            self.xer &= !flag;
        }
    }

// instruction set
    pub fn execute(&mut self, instruction: Instruction, memory: &mut Memory) {
        match instruction{
            Instruction::Add {rd, ra, rb, rc }=> {
                self.gpr[rd] = self.gpr[ra].wrapping_add(self.gpr[rb]);
                if rc {
                    self.set_cr0(self.gpr[rd]);
                }
            }
            Instruction::Addi { rd, ra, immediate }=> {
                self.gpr[rd] = (if ra == 0 { 0 } else { self.gpr[ra] }).wrapping_add(immediate as u32);
            }
            Instruction::Addis { rd, ra, immediate } => {
                self.gpr[rd] = (if ra == 0 { 0 } else { self.gpr[ra] }).wrapping_add((immediate << 16) as u32);
            }
            Instruction::Subf { rd, ra, rb, rc } => {
                self.gpr[rd] = self.gpr[rb].wrapping_sub(self.gpr[ra]);
                if rc {
                    self.set_cr0(self.gpr[rd]);
                }
            }

            Instruction::And { rd, ra, rb, rc } => {
                self.gpr[rd] = self.gpr[ra] & self.gpr[rb];
                if rc {
                    self.set_cr0(self.gpr[rd]);
                }
            }
            Instruction::Andi { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] & immediate as u32;
                self.set_cr0(self.gpr[rd]);
            }
            Instruction::Andis {rd, ra, immediate} => {
                self.gpr[rd] = self.gpr[ra] & ((immediate as u32) << 16);
                self.set_cr0(self.gpr[rd]);
            }

            Instruction::Or { rd, ra, rb, rc } =>{
                self.gpr[rd] = self.gpr[ra] | self.gpr[rb];
                if rc {
                    self.set_cr0(self.gpr[rd]);
                }
            }

            Instruction::Ori { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] | immediate as u32;
            }
            Instruction::Oris { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] | ((immediate as u32) << 16 ) ;
            }

            Instruction::Xor { rd, ra, rb, rc } =>{
                self.gpr[rd] = self.gpr[ra] ^ self.gpr[rb];
                if rc {
                    self.set_cr0(self.gpr[rd]);
                }
            }

            Instruction::Xori { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] ^ immediate as u32;
            }

            Instruction::Xoris { rd, ra, immediate } => {
                self.gpr[rd] = self.gpr[ra] ^ ((immediate as u32) << 16);
            }

            Instruction::Cmp { bf, ra, rb } =>{
                self.cmp(bf, ra, rb);
            }
            Instruction::CMPL { bf, ra, rb} => {
                self.cmp_logical(bf, ra, rb);   
            }

            Instruction::B { li, aa, lk } =>{
                self.branch(li, aa, lk);
            }

            Instruction::Lwz { rd, ra, immediate } => {
		        let address = self.d_form_address(ra, immediate);
		        self.gpr[rd] = memory.read32(address);
		    }
		
		    Instruction::Lwzu { rd, ra, immediate } => {
		        // if ra == 0 || rd == ra { /* raise a program exception */ }
		        let address = self.d_form_address(ra, immediate);
		        self.gpr[rd] = memory.read32(address);
		        self.gpr[ra] = address;
		    }
		
		    Instruction::Stw { rs, ra, immediate } => {
		        let address = self.d_form_address(ra, immediate);
		        memory.write32(address, self.gpr[rs]);
		    }
		    
		    Instruction::Stwu { rs, ra, immediate } => {
		        // if ra == 0 { /* raise a program exception */ }
		        let address = self.d_form_address(ra, immediate);
		        memory.write32(address, self.gpr[rs]);
		        self.gpr[ra] = address;
		    } 
		    Instruction::Lbz { rd, ra, immediate } => {
		        let address = self.d_form_address(ra, immediate);
		        self.gpr[rd] = memory.read8(address) as u32;
		    }
		    
		    Instruction::Lbzu { rd, ra, immediate } => {
		        // if ra == 0 || rd == ra { /* raise a program exception */ }
		        let address = self.d_form_address(ra, immediate);
		        self.gpr[rd] = memory.read8(address) as u32;
		        self.gpr[ra] = address;
		    }
		    
		    Instruction::Lhz { rd, ra, immediate } => {
		        let address = self.d_form_address(ra, immediate);
		        self.gpr[rd] = memory.read16(address) as u32;
		    }
		    
		    Instruction::Lhzu { rd, ra, immediate } => {
		        // if ra == 0 || rd == ra { /* raise a program exception */ }
		        let address = self.d_form_address(ra, immediate);
		        self.gpr[rd] = memory.read16(address) as u32;
		        self.gpr[ra] = address;
		    }
		    
		    Instruction::Lha { rd, ra, immediate } => {
		        let address = self.d_form_address(ra, immediate);
		        self.gpr[rd] = memory.read16(address) as i16 as i32 as u32;
		    }
		    
		    Instruction::Lhau { rd, ra, immediate } => {
		        // if ra == 0 || rd == ra { /* raise a program exception */ }
		        let address = self.d_form_address(ra, immediate);
		        self.gpr[rd] = memory.read16(address) as i16 as i32 as u32;
		        self.gpr[ra] = address;
		    }
		    
		    Instruction::Stb { rs, ra, immediate } => {
		        let address = self.d_form_address(ra, immediate);
		        memory.write8(address, self.gpr[rs] as u8);
		    }
		    
		    Instruction::Stbu { rs, ra, immediate } => {
		        // if ra == 0 { /* raise a program exception */ }
		        let address = self.d_form_address(ra, immediate);
		        memory.write8(address, self.gpr[rs] as u8);
		        self.gpr[ra] = address;
		    }
		    
		    Instruction::Sth { rs, ra, immediate } => {
		        let address = self.d_form_address(ra, immediate);
		        memory.write16(address, self.gpr[rs] as u16);
		    }
		    
		    Instruction::Sthu { rs, ra, immediate } => {
		        // if ra == 0 { /* raise a program exception */ }
		        let address = self.d_form_address(ra, immediate);
		        memory.write16(address, self.gpr[rs] as u16);
		        self.gpr[ra] = address;
		    }




            Instruction::Unknown => {
                println!("unknown instruction:");
            }
        }
    }
    fn set_cr0(&mut self, result: u32) {
        // set CR0 from result
         let cr_val = (if (result as i32) < 0 { 
             0b1000 
         } else if result != 0 {
             0b0100 
         } else { 0b0010 
         }) | u32::from(self.xer_so());

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
        } | u32::from(self.xer_so());

        self.cr &= !(0xF << shift);
        self.cr |= result << shift;
    }
    
    fn cmp_logical(&mut self, bf: usize, ra: usize, rb: usize) {
        let shift = (7 - bf) * 4;

        let result = if self.gpr[ra] < self.gpr[rb] {
            0b1000
        } else if self.gpr[ra] > self.gpr[rb] {
            0b0100
        } else {
            0b0010
        } | u32::from(self.xer_so());

        self.cr &= !(0xF << shift);
        self.cr |= result << shift;
    }   

    fn branch_condition(&mut self, bo: u8, bi: u8) -> bool {
	    let ctr_ok = if bo & 0b00100 != 0 {
	        true
	    } else {
	        self.ctr = self.ctr.wrapping_sub(1);
	
	        if bo & 0b00010 != 0 {
	            self.ctr == 0
	        } else {
	            self.ctr != 0
	        }
	    };
	
	    let cr_bit = (self.cr >> (31 - bi)) & 1;
	
	    let cr_ok = if bo & 0b00001 != 0 {
	        true
	    } else {
	        cr_bit == ((bo >> 3) & 1) as u32
	    };
	
	    ctr_ok && cr_ok
	}
    fn branch(&mut self, li:i32, aa:bool, lk:bool) {
        let displacement = li << 2;
        if lk { self.lr = self.pc; }
        if aa {
            self.pc = displacement as u32;
        } else {
            self.pc = self.pc.wrapping_add(displacement as u32)
                .wrapping_sub(4);
        }
    }
}
