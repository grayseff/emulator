use emulator::cpu::Cpu;
use emulator::memory::Memory;

fn main() {
    let mut cpu = Cpu::new();
    let mut ram = Memory::new(1024);

    cpu.gpr[20] = 50;
    cpu.gpr[5] = 25;

    cpu.gpr[21] = 0b1010;
    cpu.gpr[22] = 0b1100;

 }
