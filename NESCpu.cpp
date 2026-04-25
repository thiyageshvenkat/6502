#include <iostream>
#include <cstdint>

struct CPU {
  // Registers
  uint8_t A = 0; // Accumulator
  uint8_t X = 0; // Index X
  uint8_t Y = 0; // Index Y
  uint8_t SP = 0xFD; // Stack Pointer
  uint8_t P = 0; // Status flags
  uint16_t PC = 0; // Program counter

  uint8_t memory[65536]{}; // intialize memory

  uint8_t read(uint16_t addr) { // read helper function
    return memory[addr];
  }

  void write(uint16_t addr, uint8_t value) { // write helper function
    memory[addr] = value;
  }

  void setZN(uint8_t value) { // bit manipulation to speed up
    P &= ~(0x02 | 0x80);
    P |= (value == 0) << 1; // sets flag for if zero
    P |= (value & 0x80); // sets flag for if negative
  }
};

// Adressing Modes

uint8_t IMM(CPU& cpu) {
  return cpu.read(cpu.PC++);
}

uint8_t ZP(CPU& cpu) {
  return cpu.read(cpu.read(cpu.PC++));
}
void LDA(CPU& cpu, uint8_t value) {
  cpu.A = value;
  cpu.setZN(cpu.A);
}

// Instructions

struct Instruction {
  const char* name; // name, for debugging purposes
  uint8_t (*addrmode)(CPU&); // what address mode is this instruction
  void (*execute)(CPU&, uint8_t); // point to the actual function itself
  uint8_t bytes; // 1 byte -> implicit commands
                 // 2 byte -> normal commands
                 // 3 byte -> commands with a two byte value
  int cycles; // some commands take more cycles than others
};

Instruction table[256] = {};

void initTable() {
  table[0xA9] = {"LDA", IMM, LDA, 2, 2};
  table[0xA5] = {"LDA", ZP,  LDA, 2, 3};
}

int main() {
  CPU cpu;
  initTable(); // fill table

  cpu.PC = 0x0000; // program counter initialized to beginning
  cpu.memory[0x0000] = 0xA9; /// LDA opcode with IMM addrmode
  cpu.memory[0x0001] = 0x42; // operand: value 0x42
                             //
  uint8_t opcode = cpu.read(cpu.PC++);
  Instruction& inst = table[opcode]; // retrieve instruction

  uint8_t value = inst.addrmode(cpu);
  inst.execute(cpu, value);

  std::cout << "A = " << (int)cpu.A << "\n";
}
