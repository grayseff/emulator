#[derive(Debug)]
pub enum Instruction {
    Add  { rd: usize, ra: usize, rb: usize, rc: bool },
    Addi { rd: usize, ra: usize, immediate: i32 },
    Addis { rd: usize, ra: usize, immediate: i32 },
    Subf { rd: usize, ra: usize, rb: usize, rc: bool },

    And  { rd: usize, ra: usize, rb: usize, rc: bool },
    Andi { rd: usize, ra: usize, immediate: u16 },
    Andis { rd: usize, ra: usize, immediate: u16 },
    Or   { rd: usize, ra: usize, rb: usize, rc: bool },
    Ori  { rd: usize, ra: usize, immediate: u16 },
    Oris { rd: usize, ra: usize, immediate: u16 },
    Xor  { rd: usize, ra: usize, rb: usize, rc: bool },
    Xori { rd: usize, ra: usize, immediate: u16 },
    Xoris { rd: usize, ra: usize, immediate: u16 },

    Cmp { bf: usize, ra: usize, rb: usize },
    CMPL { bf: usize, ra: usize, rb: usize },
    B { li: i32, aa: bool, lk: bool },
    BC { bo: u8, bi: u8, bd: i32, aa: bool, lk: bool },
    BCLR { bo: u8, bi: u8, bh: u8, lk: bool },

    Lbz  { rd: usize, ra: usize, immediate: i32 },
    Lbzu { rd: usize, ra: usize, immediate: i32 },
    Lhz  { rd: usize, ra: usize, immediate: i32 },
    Lhzu { rd: usize, ra: usize, immediate: i32 },
    Lha  { rd: usize, ra: usize, immediate: i32 },
    Lhau { rd: usize, ra: usize, immediate: i32 },
    Lwz  { rd: usize, ra: usize, immediate: i32 },
    Lwzu { rd: usize, ra: usize, immediate: i32 },

    Stb  { rs: usize, ra: usize, immediate: i32 },
    Stbu { rs: usize, ra: usize, immediate: i32 },
    Sth  { rs: usize, ra: usize, immediate: i32 },
    Sthu { rs: usize, ra: usize, immediate: i32 },
    Stw  { rs: usize, ra: usize, immediate: i32 },
    Stwu { rs: usize, ra: usize, immediate: i32 },

    Unknown,
}

pub fn decode(value: u32) -> Instruction {
    let opcode = value >> 26;

    let instruction = match opcode {
        // D-form family
        14 | 15 | 24 | 25 | 26 | 27 | 28 | 29 => {
            let rd = ((value >> 21) & 0x1F) as usize;
            let ra = ((value >> 16) & 0x1F) as usize;
            let immediate = (value & 0xFFFF) as u16;
            match opcode {
                14 => Instruction::Addi { rd, ra, immediate: immediate as i16 as i32 },
                15 => Instruction::Addis { rd, ra, immediate: immediate as i16 as i32 },
                24 => Instruction::Ori { rd, ra, immediate },
                25 => Instruction::Oris { rd, ra, immediate },
                26 => Instruction::Xori { rd, ra, immediate },
                27 => Instruction::Xoris { rd, ra, immediate },
                28 => Instruction::Andi { rd, ra, immediate },
                29 => Instruction::Andis { rd, ra, immediate },

                _ => Instruction::Unknown,
            }
        }

        31 => {
            let rd = ((value >> 21) & 0x1F) as usize;
            let ra = ((value >> 16) & 0x1F) as usize;
            let rb = ((value >> 11) & 0x1F) as usize;
            let xo = (value >> 1) & 0x3FF;
            let rc = (value & 0x1) != 0;

            match xo {
                0 => {
                    let bf = ((value >> 23) & 0x7) as usize;
                    let l = ((value >> 21) & 0x1) != 0;
                    let reserved = ((value >> 22) & 0x1) != 0;

                    if !rc && !l && !reserved { Instruction::Cmp { bf, ra, rb } }
                    else { Instruction::Unknown }
                }
                32 => {
                    let bf = ((value >> 23) & 0x7) as usize;
                    let l = ((value >> 21) & 0x1) != 0;
                    let reserved = ((value >> 22) & 0x1) != 0;
                
                    if !rc && !l && !reserved {
                        Instruction::CMPL { bf, ra, rb }
                    } else {
                        Instruction::Unknown
                    }

                }
                40 => Instruction::Subf { rd, ra, rb, rc },
                266 => Instruction::Add { rd, ra, rb, rc },
                28 => Instruction::And { rd, ra, rb, rc },
                316 => Instruction::Xor { rd, ra, rb, rc },
                444 => Instruction::Or { rd, ra, rb, rc },

                _ => Instruction::Unknown,
            }
        }
		16 => {
		    let bo = ((value >> 21) & 0x1F) as u8;
		    let bi = ((value >> 16) & 0x1F) as u8;
		    let bd = ((value >> 2) & 0x3FFF) as i32;
		    let aa = (value & 0x2) != 0;
		    let lk = (value & 0x1) != 0;
		
		    Instruction::BC { bo, bi, bd, aa, lk }
		}

        18 => {
            let li = ((value >> 2) & 0x0FF_FFFF) as u32;
            let li = ((li << 8) as i32) >> 8;
            let aa = ((value >> 1) & 0x1) != 0;
            let lk = (value & 0x1) != 0;
            Instruction::B { li, aa, lk }
        }
        19 => {
            let bo = ((value >> 21) & 0x1F) as u8;
            let bi = ((value >> 16) & 0x1F) as u8;
            let bh = ((value >> 11) & 0x3) as u8;
            let xo = ((value >> 1) & 0x3FF) as u16;
            let lk = (value & 0x1) != 0;

            match xo {
                16 => Instruction::BCLR { bo, bi, bh, lk },
                _ => Instruction::Unknown,
            }
        }

        32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 | 40 | 41 | 42 | 43 | 44 | 45 => {
            let rd = ((value >> 21) & 0x1F) as usize;
            let ra = ((value >> 16) & 0x1F) as usize;
            let immediate = (value & 0xFFFF) as u16 as i16 as i32;
    
            match opcode {
                32 => Instruction::Lwz { rd, ra, immediate },
                33 => Instruction::Lwzu { rd, ra, immediate },
                34 => Instruction::Lbz { rd, ra, immediate },
                35 => Instruction::Lbzu { rd, ra, immediate },
                36 => Instruction::Stw { rs: rd, ra, immediate },
                37 => Instruction::Stwu { rs: rd, ra, immediate },
                38 => Instruction::Stb { rs: rd, ra, immediate },
                39 => Instruction::Stbu { rs: rd, ra, immediate },
                40 => Instruction::Lhz { rd, ra, immediate },
                41 => Instruction::Lhzu { rd, ra, immediate },
                42 => Instruction::Lha { rd, ra, immediate },
                43 => Instruction::Lhau { rd, ra, immediate },
                44 => Instruction::Sth { rs: rd, ra, immediate },
                45 => Instruction::Sthu { rs: rd, ra, immediate },
                _ => Instruction::Unknown,
            }
        }
        _ => Instruction::Unknown,
    };

    if matches!(instruction, Instruction::Unknown) {
        println!("unknown opcode {opcode:#04x} (instruction {value:#010x})");
    }

    instruction
}
