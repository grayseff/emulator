

pub enum InstructionType {
    Unknown,
    Addi,
    Add,
}

pub struct Instruction {
    pub instruction_type: InstructionType,
    
    pub opcode: u32,
    pub rd: u32,
    pub ra: u32,
    pub rb: u32,
    pub xo: u32,
    pub immediate: i32,
}


pub fn decode(value: u32) -> Instruction {
   let opcode = value >> 26;
    
   match opcode {
        14 => {//D type 
            let instruction_type = InstructionType::Addi;
            let rd = (value >> 21) & 0x1F; 
            let ra = (value >> 16) & 0x1F; 
            let immediate = (value & 0xffff) as u16 as i16 as i32;
            
            Instruction {
                instruction_type,
                opcode,
                rd,
                ra,
                rb: 0,
                xo: 0,
                immediate,
            }
        }
        31 => {// X form
           let rd = (value >> 21) & 0x1f;
           let ra = (value >> 16) & 0x1f;
           let rb = (value >> 11) & 0x1f;
           let xo = (value >>1) & 0x3ff;
           let instruction_type = if xo == 266 {
               InstructionType::Add
           } else {
                println!("{} is unknown", xo);
                InstructionType::Unknown
           };

           Instruction {
                instruction_type,
                opcode,
                rd,
                ra,
                rb,
                xo,
                immediate: 0,
           }
        }
        _ => {//unknown
            println!("type of {} unknown", opcode);
            Instruction {
                instruction_type: InstructionType::Unknown,
                opcode,
                rd: 0,
                ra: 0,
                rb: 0,
                xo: 0,
                immediate: 0,
            }
        
        }
   }

}
