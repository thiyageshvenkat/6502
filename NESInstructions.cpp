#include "NESInstructions.h"
#include <cstdint>
#include <iostream>

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

uint8_t ABSX(CPU& cpu) {
  uint8_t lo = cpu.read(cpu.PC++);
  uint8_t hi = cpu.read(cpu.PC++);
  cpu.address = ((hi << 8) | lo) + cpu.X;
  return 0;
}

uint8_t ABSY(CPU& cpu) {
  uint8_t lo = cpu.read(cpu.PC++);
  uint8_t hi = cpu.read(cpu.PC++);
  cpu.address = ((hi << 8) | lo) + cpu.Y;
  return 0;
}

uint8_t REL(CPU& cpu) {
  int8_t offset = (int8_t)cpu.read(cpu.PC++);
  cpu.address = cpu.PC + offset;
  return 0;
}

uint8_t ZPX(CPU& cpu) {
  cpu.address = (cpu.read(cpu.PC++) + cpu.X) & 0xFF;
  return 0;
}

uint8_t IND(CPU& cpu) {
  uint8_t lo = cpu.read(cpu.PC++);
  uint8_t hi = cpu.read(cpu.PC++);
  uint16_t ptr = (hi << 8) | lo; // reconstruct location of pointer

  // read the pointer
  uint8_t address_lo = cpu.read(ptr);
  uint8_t address_hi = cpu.read(ptr + 1);

  // reconstruct address pointer is pointing to
  cpu.address = (address_hi << 8) | address_lo;
  return 0;
}

// Instructions

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

void BEQ(CPU& cpu) {
  if (cpu.P & 0x02) { cpu.PC = cpu.address; } // Z flag
}

void BNE(CPU& cpu) {
  if (!(cpu.P & 0x02)) { cpu.PC = cpu.address; } // opposite of BEQ
}

void CMP(CPU& cpu) {
  uint8_t value = cpu.fetch();
  uint16_t temp = cpu.A - value;
  // set carry if A >= value
  if (cpu.A >= value) cpu.P |= 0x01;
  else cpu.P &= ~0x01;
  cpu.setZN(temp & 0xFF);
}

void NOP(CPU& cpu) { /* do nothing */ }
void CLC(CPU& cpu) { cpu.P &= ~0x01; } // clear carry
void SEC(CPU& cpu) { cpu.P |= 0x01; }  // set carry

void INC(CPU& cpu) {
  uint8_t value = cpu.read(cpu.address) + 1;
  cpu.write(cpu.address, value);
  cpu.setZN(value);
}

void DEC(CPU& cpu) {
  uint8_t value = cpu.read(cpu.address) - 1;
  cpu.write(cpu.address, value);
  cpu.setZN(value);
}

void JMP(CPU& cpu) {
  cpu.PC = cpu.address;
}

void ADC(CPU& cpu) { // good operation to learn about my emulator architecture
  uint8_t value = cpu.fetch(); // fetch operand

  // Perform addition with carry flag (C is bit 0 of P)
  uint16_t sum = cpu.A + value + (cpu.P & 0x01);

  // Set carry flag (C) if result exceeds 8 bits
  if (sum > 0xFF)
    cpu.P |= 0x01;   // set carry
  else
    cpu.P &= ~0x01;  // clear carry

  // Store lower 8 bits of result back into A
  // Extra bit goes to carry flag
  cpu.A = sum & 0xFF;

  // Update zero and negative flags based on result
  cpu.setZN(cpu.A);
}

void SBC(CPU& cpu) { // good operation to learn about my emulator architecture
  uint8_t value = cpu.fetch(); // fetch operand

  // Perform subtraction with carry flag (C is bit 0 of P)
  // SBC = A - value - (1 - C)
  uint8_t carry = cpu.P & 0x01;
  uint16_t difference = cpu.A - value - (1 - carry);

  // Set carry flag (C) if no borrow occurred
  // (C = 1 means no borrow, C = 0 means borrow)
  if (cpu.A >= value + (1 - carry))
    cpu.P |= 0x01;   // set carry
  else
    cpu.P &= ~0x01;  // clear carry

  // Store lower 8 bits of result back into A
  // Extra bit goes to carry flag
  cpu.A = difference & 0xFF;

  // Update zero and negative flags based on result
  cpu.setZN(cpu.A);
}

void AND(CPU& cpu) {
  cpu.A &= cpu.fetch();
  cpu.setZN(cpu.A);
}

void ORA(CPU& cpu) {
  cpu.A |= cpu.fetch();
  cpu.setZN(cpu.A);
}

void EOR(CPU& cpu) {
  cpu.A ^= cpu.fetch();
  cpu.setZN(cpu.A);
}

void PHA(CPU& cpu) {
  cpu.write(0x0100 + cpu.SP, cpu.A);
  cpu.SP--;
}

void PLA(CPU& cpu) {
  cpu.SP++;
  cpu.A = cpu.read(0x0100 + cpu.SP);
  cpu.setZN(cpu.A);
}

void PHP(CPU& cpu) {
  cpu.write(0x0100 + cpu.SP, cpu.P);
  cpu.SP--;
}

void PLP(CPU& cpu) {
  cpu.SP++;
  cpu.P = cpu.read(0x0100 + cpu.SP);
}

void JSR(CPU& cpu) {
  cpu.PC--; // to ensure when RTS is called it ends up in the correct place

  // push the two bytes of the address onto stack
  cpu.write(0x0100 + cpu.SP, (cpu.PC >> 8) & 0xFF);
  cpu.SP--;

  cpu.write(0x0100 + cpu.SP, cpu.PC & 0xFF);
  cpu.SP--;

  cpu.PC = cpu.address; // go to location of subroutine designated by inputted address
}

void RTS(CPU& cpu) {
  // reconstruct from little-endian two bytes
  cpu.SP++;
  uint8_t lo = cpu.read(0x0100 + cpu.SP);

  cpu.SP++;
  uint8_t hi = cpu.read(0x0100 + cpu.SP);

  cpu.PC = ((uint16_t)hi << 8) | lo; // put bytes together
  cpu.PC++;
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

  table[0xF0] = {"BEQ", REL, BEQ, 2, 2};
  table[0xD0] = {"BNE", REL, BNE, 2, 2};

  table[0xC9] = {"CMP", IMM, CMP, 2, 2};
  table[0xC5] = {"CMP", ZP,  CMP, 2, 3};
  table[0xCD] = {"CMP", ABS, CMP, 3, 4};

  table[0xEA] = {"NOP", nullptr, NOP, 1, 2};
  table[0xE6] = {"INC", ZP,  INC, 2, 5};
  table[0xEE] = {"INC", ABS, INC, 3, 6};

  table[0xC6] = {"DEC", ZP,  DEC, 2, 5};
  table[0xCE] = {"DEC", ABS, DEC, 3, 6};

  table[0xB5] = {"LDA", ZPX, LDA, 2, 4};
  table[0x95] = {"STA", ZPX, STA, 2, 4};
  table[0xF6] = {"INC", ZPX, INC, 2, 6};
  table[0xD6] = {"DEC", ZPX, DEC, 2, 6};

  table[0x4C] = {"JMP", ABS, JMP, 3, 3};
  table[0x69] = {"ADC", IMM, ADC, 2, 2};
  table[0x65] = {"ADC", ZP,  ADC, 2, 3};
  table[0x6D] = {"ADC", ABS, ADC, 3, 4};
  table[0xE9] = {"SBC", IMM, SBC, 2, 2};
  table[0xE5] = {"SBC", ZP,  SBC, 2, 3};
  table[0xED] = {"SBC", ABS, SBC, 3, 4};
  table[0x18] = {"CLC", nullptr, CLC, 1, 2};
  table[0x38] = {"SEC", nullptr, SEC, 1, 2};

  table[0x29] = {"AND", IMM, AND, 2, 2};
  table[0x25] = {"AND", ZP,  AND, 2, 3};
  table[0x2D] = {"AND", ABS, AND, 3, 4};

  table[0x09] = {"ORA", IMM, ORA, 2, 2};
  table[0x05] = {"ORA", ZP,  ORA, 2, 3};
  table[0x0D] = {"ORA", ABS, ORA, 3, 4};

  table[0x49] = {"EOR", IMM, EOR, 2, 2};
  table[0x45] = {"EOR", ZP,  EOR, 2, 3};
  table[0x4D] = {"EOR", ABS, EOR, 3, 4};
  table[0x48] = {"PHA", nullptr, PHA, 1, 3};
  table[0x68] = {"PLA", nullptr, PLA, 1, 4};
  table[0x08] = {"PHP", nullptr, PHP, 1, 3};
  table[0x28] = {"PLP", nullptr, PLP, 1, 4};
  table[0x20] = {"JSR", ABS,     JSR, 3, 6};
  table[0x60] = {"RTS", nullptr, RTS, 1, 6};
}


StepTrace step(CPU& cpu) {
  uint16_t oldPC = cpu.PC;
  uint8_t opcode = cpu.read(cpu.PC++);
  Instruction& inst = table[opcode];

  if (inst.addrmode) inst.addrmode(cpu);
  if (inst.execute) inst.execute(cpu);

  return {oldPC, opcode, inst.name};
}
