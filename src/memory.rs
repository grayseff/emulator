

pub struct Memory {
    data: Vec<u8>,
}

impl Memory {
    pub fn new(size: usize) -> Self {
        Self {
            data: vec![0;size],
        }
    }
    pub fn write8(&mut self, address: u32, value: u8) {
        self.data[address as usize] = value;
    }
    pub fn write16(&mut self, address: u32, value: u16) {
        let address = address as usize;
    
        self.data[address] = (value >> 8) as u8;
        self.data[address + 1] = value as u8;
    }
    pub fn write32(&mut self, address: u32, value:u32) {
        let address = address as usize;

        self.data[address]      = ((value >> 24) & 0xFF) as u8;
        self.data[address + 1]  = ((value >> 16) & 0xFF) as u8;
        self.data[address + 2]  = ((value >>  8) & 0xFF) as u8;
        self.data[address + 3]  = (value & 0xFF) as u8;
    }
    pub fn read8(&self, address: u32) -> u8 {
        self.data[address as usize]
    }
    pub fn read16(&self, address: u32) -> u16 {
        let address = address as usize;
    
        ((self.data[address] as u16) << 8)
            | self.data[address + 1] as u16
    }
        pub fn read32(&self, address: u32) -> u32 {
            let address = address as usize;
    
            ((self.data[address] as u32) << 24)
                | ((self.data[address +1] as u32) << 16)
                | ((self.data[address +2] as u32) << 8)
                | (self.data[address +3] as u32)
    
    }
    
}
