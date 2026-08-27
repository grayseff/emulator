pub enum Instruction {
    Add {
        rd: u32,
        ra: u32,
        rb: u32,
    },

    Addi {
        rd: u32,
        ra: u32,
        immediate: i32,
    },

    Subf {
        rd: u32,
        ra: u32,
        rb: u32,
    },

    And {
        rd: u32,
        ra: u32,
        rb: u32,
    },

    Or {
        rd: u32,
        ra: u32,
        rb: u32,
    },

    Xor {
        rd: u32,
        ra: u32,
        rb: u32,
    },

    Cmp {
        bf: u32,
        ra: u32,
        rb: u32,
    },

    B {
        li: i32,
        aa: bool,
        lk: bool,
    },

    Unknown,
}





pub fn decode(value: u32) -> Instruction {
    let opcode = value >> 26;

    match opcode {
        14 => {
            let rd = (value >> 21) & 0x1F;
            let ra = (value >> 16) & 0x1F;
            let immediate = (value & 0xFFFF) as u16 as i16 as i32;

            Instruction::Addi {
                rd,
                ra,
                immediate,
            }
        }

        31 => {
            let rd = (value >> 21) & 0x1F;
            let ra = (value >> 16) & 0x1F;
            let rb = (value >> 11) & 0x1F;
            let xo = (value >> 1) & 0x3FF;

            match xo {
                0 => {
                    let bf = rd >> 2;

                    Instruction::Cmp {
                        bf,
                        ra,
                        rb,
                    }
                }

                40 => Instruction::Subf { rd, ra, rb },

                266 => Instruction::Add { rd, ra, rb },

                28 => Instruction::And { rd, ra, rb },

                316 => Instruction::Xor { rd, ra, rb },

                444 => Instruction::Or { rd, ra, rb },

                _ => Instruction::Unknown,
            }
        }

        18 => {
            // B-form — we'll fill this in  
            todo!()
        }

        _ => Instruction::Unknown,
    }
}
