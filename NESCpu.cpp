#include <iostream>
#include <cstdint>

struct CPU {
  uint16_t address = 0; // stores where the operand located 
  uint8_t fetched = 0; // reads the operand byte from the location determined by the addressing mode
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

  uint8_t fetch() {
    fetched = read(address);
    return fetched;
  }
  void setZN(uint8_t value) { // bit manipulation to speed up
    P &= ~(0x02 | 0x80);
    P |= (value == 0) << 1; // sets flag for if zero
    P |= (value & 0x80); // sets flag for if negative
  }
};

// Adressing Modes

uint8_t IMM(CPU& cpu) {
  cpu.address = cpu.PC++;
  return 0;
}

uint8_t ZP(CPU& cpu) {
  cpu.address = cpu.read(cpu.PC++);
  return 0;
}

uint8_t ABS(CPU& cpu) { // must switch lo and hi because input is little-endian format
  uint8_t lo = cpu.read(cpu.PC++);
  uint8_t hi = cpu.read(cpu.PC++);
  cpu.address = (hi << 8) | lo;
  return 0;
}

// Instructions

struct Instruction {
  const char* name; // name, for debugging purposes
  uint8_t (*addrmode)(CPU&); // what address mode is this instruction
  void (*execute)(CPU&); // point to the actual function itself
  uint8_t bytes; // 1 byte -> implicit commands
                 // 2 byte -> normal commands
                 // 3 byte -> commands with a two byte value
  int cycles; // some commands take more cycles than others
};

void LDA(CPU& cpu) {
  cpu.A = cpu.fetch();
  cpu.setZN(cpu.A);
}

void LDX(CPU& cpu) {
  cpu.X = cpu.fetch();
  cpu.setZN(cpu.X);
}

void LDY(CPU& cpu) {
  cpu.Y = cpu.fetch();
  cpu.setZN(cpu.Y);
}

void STA(CPU& cpu) {
  cpu.write(cpu.address, cpu.A);
}

void TAX(CPU& cpu) {
  cpu.X = cpu.A;
  cpu.setZN(cpu.X);
}

void TXA(CPU& cpu) {
  cpu.A = cpu.X;
  cpu.setZN(cpu.A);
}

void TAY(CPU& cpu) {
  cpu.Y = cpu.A;
  cpu.setZN(cpu.Y);
}

void TYA(CPU& cpu) {
  cpu.A = cpu.Y;
  cpu.setZN(cpu.A);
}

void TSX(CPU& cpu) {
  cpu.X = cpu.SP;
  cpu.setZN(cpu.X);
}

void TXS(CPU& cpu) {
  cpu.SP = cpu.X;
}

void DEX(CPU& cpu) {
  cpu.X -= 1;
  cpu.setZN(cpu.X);
}

void DEY(CPU& cpu) {
  cpu.Y -= 1;
  cpu.setZN(cpu.Y);
}

void INX(CPU& cpu) {
  cpu.X += 1;
  cpu.setZN(cpu.X);
}

void INY(CPU& cpu) {
  cpu.Y += 1;
  cpu.setZN(cpu.Y);
}

Instruction table[256] = {};

void initTable() {
  table[0xA9] = {"LDA", IMM, LDA, 2, 2};
  table[0xA5] = {"LDA", ZP,  LDA, 2, 3};
  table[0xAD] = {"LDA", ABS, LDA, 3, 4};

  table[0x85] = {"STA", ZP,  STA, 2, 3};
  table[0x8D] = {"STA", ABS, STA, 3, 4};

  table[0xA2] = {"LDX", IMM, LDX, 2, 2};
  table[0xA6] = {"LDX", ZP,  LDX, 2, 3};
  table[0xAE] = {"LDX", ABS, LDX, 3, 4};

  table[0xA0] = {"LDY", IMM, LDY, 2, 2};
  table[0xA4] = {"LDY", ZP,  LDY, 2, 3};
  table[0xAC] = {"LDY", ABS, LDY, 3, 4};

  table[0xAA] = {"TAX", nullptr, TAX, 1, 2};
  table[0xA8] = {"TAY", nullptr, TAY, 1, 2};
  table[0x8A] = {"TXA", nullptr, TXA, 1, 2};
  table[0x98] = {"TYA", nullptr, TYA, 1, 2};
  table[0xE8] = {"INX", nullptr, INX, 1, 2};
  table[0xC8] = {"INY", nullptr, INY, 1, 2};
  table[0xCA] = {"DEX", nullptr, DEX, 1, 2};
  table[0x88] = {"DEY", nullptr, DEY, 1, 2};
  table[0xBA] = {"TSX", nullptr, TSX, 1, 2};
  table[0x9A] = {"TXS", nullptr, TXS, 1, 2};
}

void step(CPU& cpu) {
  uint8_t opcode = cpu.read(cpu.PC++);
  Instruction& inst = table[opcode];

  if (inst.addrmode) {inst.addrmode(cpu);}; // if address is provided
  inst.execute(cpu);
}

int main() {
  CPU cpu;
  initTable();

  cpu.PC = 0x0000;

  // LDA #$42
  cpu.memory[0x0000] = 0xA9;
  cpu.memory[0x0001] = 0x42;

  // STA $10
  cpu.memory[0x0002] = 0x85;
  cpu.memory[0x0003] = 0x10;

  step(cpu); // runs LDA
  step(cpu); // runs STA

  std::cout << "A = " << (int)cpu.A << "\n";
  std::cout << "mem[0x10] = " << (int)cpu.memory[0x10] << "\n";
}
