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
  int8_t offset = (int8_t)cpu.read(cpu.PC++); // signed branch offset
  cpu.address = cpu.PC + offset; // branches start from next instruction
  return 0;
}

uint8_t ZPX(CPU& cpu) {
  cpu.address = (cpu.read(cpu.PC++) + cpu.X) & 0xFF; // zero-page wraps at $FF
  return 0;
}

uint8_t ZPY(CPU& cpu) {
  cpu.address = (cpu.read(cpu.PC++) + cpu.Y) & 0xFF;
  return 0;
}

uint8_t IND(CPU& cpu) {
  uint8_t lo = cpu.read(cpu.PC++);
  uint8_t hi = cpu.read(cpu.PC++);
  uint16_t ptr = ((uint16_t)hi << 8) | lo; // reconstruct location of pointer

  uint8_t address_lo = cpu.read(ptr);
  uint8_t address_hi = cpu.read((ptr & 0xFF00) | ((ptr + 1) & 0x00FF)); // JMP IND 6502 page-wrap bug

  cpu.address = ((uint16_t)address_hi << 8) | address_lo;
  return 0;
}

uint8_t ACC(CPU &cpu) { // for instructions that operate directly on the accumulator (e.g. ASL A)
  cpu.atAccumulator = true;
  return 0;
}

uint8_t IMP(CPU& cpu) {
  return 0;
}

uint8_t IZX(CPU& cpu) {
  uint8_t zp = cpu.read(cpu.PC++); // zero-page address
  uint8_t ptr = (zp + cpu.X) & 0xFF;

  uint8_t lo = cpu.read(ptr);
  uint8_t hi = cpu.read((ptr + 1) & 0xFF);

  cpu.address = ((uint16_t)hi << 8) | lo;
  return 0;
}

uint8_t IZY(CPU& cpu) {
  uint8_t zp = cpu.read(cpu.PC++); // zero-page address
  uint8_t ptr = zp;

  uint8_t lo = cpu.read(ptr);
  uint8_t hi = cpu.read((ptr + 1) & 0xFF);

  cpu.address = ((uint16_t)hi << 8) | lo;
  cpu.address += cpu.Y;
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

void STX(CPU& cpu) {
  cpu.write(cpu.address, cpu.X);
}

void STY(CPU& cpu) {
  cpu.write(cpu.address, cpu.Y);
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

void BCC(CPU& cpu) {
  if (!(cpu.P & 0x01)) { cpu.PC = cpu.address; }
}

void BCS(CPU& cpu) {
  if (cpu.P & 0x01) { cpu.PC = cpu.address; }
}

void BPL(CPU& cpu) {
  if (!(cpu.P & 0x80)) { cpu.PC = cpu.address; }
}

void BMI(CPU& cpu) {
  if (cpu.P & 0x80) { cpu.PC = cpu.address; }
}

void BVC(CPU& cpu) {
  if (!(cpu.P & 0x40)) { cpu.PC = cpu.address; }
}

void BVS(CPU& cpu) {
  if (cpu.P & 0x40) { cpu.PC = cpu.address; }
}

void CMP(CPU& cpu) {
  uint8_t value = cpu.fetch();
  uint16_t temp = cpu.A - value;
  // set carry if A >= value
  if (cpu.A >= value) cpu.P |= 0x01;
  else cpu.P &= ~0x01;
  cpu.setZN(temp & 0xFF);
}

void CPX(CPU& cpu) {
  uint8_t value = cpu.fetch();
  uint16_t temp = cpu.X - value;
  // set carry if X >= value
  if (cpu.X >= value) cpu.P |= 0x01;
  else cpu.P &= ~0x01;
  cpu.setZN(temp & 0xFF);
}

void CPY(CPU& cpu) {
  uint8_t value = cpu.fetch();
  uint16_t temp = cpu.Y - value;
  // set carry if Y >= value
  if (cpu.Y >= value) cpu.P |= 0x01;
  else cpu.P &= ~0x01;
  cpu.setZN(temp & 0xFF);
}

void NOP(CPU& cpu) { /* do nothing */ }
void CLC(CPU& cpu) { cpu.P &= ~0x01; } // clear carry
void SEC(CPU& cpu) { cpu.P |= 0x01; }  // set carry
void CLI(CPU& cpu) { cpu.P &= ~0x04; }
void SEI(CPU& cpu) { cpu.P |=  0x04; }
void CLV(CPU& cpu) { cpu.P &= ~0x40; }
void CLD(CPU& cpu) { cpu.P &= ~0x08; }
void SED(CPU& cpu) { cpu.P |=  0x08; }

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
  if (sum > 0xFF) {
    cpu.P |= 0x01;   // set carry
  } else {
    cpu.P &= ~0x01;  // clear carry
  }
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
  cpu.write(0x0100 + cpu.SP, cpu.A); // stack lives on page $01
  cpu.SP--;
}

void PLA(CPU& cpu) {
  cpu.SP++;
  cpu.A = cpu.read(0x0100 + cpu.SP);
  cpu.setZN(cpu.A);
}

void PHP(CPU& cpu) {
  cpu.write(0x0100 + cpu.SP, cpu.P | 0x10 | 0x20); // push P with B and unused bit set
  cpu.SP--;
}

void PLP(CPU& cpu) {
  cpu.SP++;
  cpu.P = (cpu.read(0x0100 + cpu.SP) | 0x20); // cpu.P |= => unused bit 5 is always set to 1, minor 6502 quirk
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

void BRK(CPU& cpu) {
  cpu.PC++; // BRK is treated as a 2-byte instruction; skip padding byte

  // Push PC hi/lo bytes onto stack
  cpu.write(0x0100 + cpu.SP, (cpu.PC >> 8) & 0xFF);
  cpu.SP--;
  cpu.write(0x0100 + cpu.SP, cpu.PC & 0xFF);
  cpu.SP--;

  // Push status with B flag and unused bit set
  cpu.write(0x0100 + cpu.SP, cpu.P | 0x10 | 0x20);
  cpu.SP--;

  // Set interrupt disable flag
  cpu.P |= 0x04;

  // Load IRQ/BRK vector from $FFFE/$FFFF
  uint8_t lo = cpu.read(0xFFFE);
  uint8_t hi = cpu.read(0xFFFF);
  cpu.PC = ((uint16_t)hi << 8) | lo;
}

void RTI(CPU& cpu) { // inverse of BRK, returns from interrupts
  // Pull status
  cpu.SP++;
  cpu.P = cpu.read(0x0100 + cpu.SP);
  cpu.P |= 0x20; // unused bit 5 is always set to 1
  cpu.P &= ~0x10; // clear internal B flag

  // Pull PC lo/hi byte
  cpu.SP++;
  uint8_t lo = cpu.read(0x0100 + cpu.SP);
  cpu.SP++;
  uint8_t hi = cpu.read(0x0100 + cpu.SP);

  cpu.PC = ((uint16_t)hi << 8) | lo;
}

void BIT(CPU& cpu) { // tests memory/status bits without changing A.
  uint8_t value = cpu.fetch();

  // Z = 1 if (A & value) == 0  // A masks which bits to test
  if ((cpu.A & value) == 0) {
    cpu.P |= 0x02;
  } else {
    cpu.P &= ~0x02;
  }

  // N = value bit 7            // test high/sign/status bit with BMI/BPL
  if (value & 0x80) {
    cpu.P |= 0x80;
  } else {
    cpu.P &= ~0x80;
  }

  // V = value bit 6            // test status bit with BVS/BVC
  if (value & 0x40) {
    cpu.P |= 0x40;
  } else {
    cpu.P &= ~0x40;
  }
}

void ASL(CPU &cpu) {
  uint8_t value;

  if (cpu.atAccumulator) {
    value = cpu.A;
  } else {
    value = cpu.fetch();
  }

  // Shift left
  uint8_t result = value << 1;

  // Set carry flag to bit that was shifted out
  if (value & 0x80) {
    cpu.P |= 0x01;   // set carry
  } else {
    cpu.P &= ~0x01;  // clear carry
  }

  // Store result back
  if (cpu.atAccumulator) {
    cpu.A = result;
    cpu.setZN(cpu.A);
  } else {
    cpu.write(cpu.address, result);
    cpu.setZN(result);
  }
}

void LSR(CPU& cpu) {
  uint8_t value;

  if (cpu.atAccumulator) {
    value = cpu.A;
  } else {
    value = cpu.fetch();
  }

  uint8_t result = value >> 1;

  // Set carry flag to bit that was shifted out
  if (value & 0x01) {
    cpu.P |= 0x01;   // set carry
  } else {
    cpu.P &= ~0x01;  // clear carry
  }

  if (cpu.atAccumulator) {
    cpu.A = result;
    cpu.setZN(cpu.A);
  } else {
    cpu.write(cpu.address, result);
    cpu.setZN(result);
  }
}

void ROL(CPU& cpu) {
  uint8_t value;

  if (cpu.atAccumulator) {
    value = cpu.A;
  } else {
    value = cpu.fetch();
  }

  uint8_t result = (value << 1) | (cpu.P & 0x01); // rotate left through carry

  // Set carry flag to bit that was rotated out
  if (value & 0x80) {
    cpu.P |= 0x01;   // set carry
  } else {
    cpu.P &= ~0x01;  // clear carry
  }

  if (cpu.atAccumulator) {
    cpu.A = result;
    cpu.setZN(cpu.A);
  } else {
    cpu.write(cpu.address, result);
    cpu.setZN(result);
  }
}

void ROR(CPU& cpu) {
  uint8_t value;

  if (cpu.atAccumulator) {
    value = cpu.A;
  } else {
    value = cpu.fetch();
  }

  uint8_t result = (value >> 1) | ((cpu.P & 0x01) << 7); // rotate right through carry

  // Set carry flag to bit that was rotated out
  if (value & 0x01) {
    cpu.P |= 0x01;   // set carry
  } else {
    cpu.P &= ~0x01;  // clear carry
  }

  if (cpu.atAccumulator) {
    cpu.A = result;
    cpu.setZN(cpu.A);
  } else {
    cpu.write(cpu.address, result);
    cpu.setZN(result);
  }
}

Instruction table[256] = {};

void initTable() {
  table[0x00] = {"BRK", IMP,  BRK, 1, 7};
  table[0x01] = {"ORA", IZX,  ORA, 2, 6};
  table[0x05] = {"ORA", ZP,   ORA, 2, 3};
  table[0x06] = {"ASL", ZP,   ASL, 2, 5};
  table[0x08] = {"PHP", IMP,  PHP, 1, 3};
  table[0x09] = {"ORA", IMM,  ORA, 2, 2};
  table[0x0A] = {"ASL", ACC,  ASL, 1, 2};
  table[0x0D] = {"ORA", ABS,  ORA, 3, 4};
  table[0x0E] = {"ASL", ABS,  ASL, 3, 6};
  table[0x10] = {"BPL", REL,  BPL, 2, 2};
  table[0x11] = {"ORA", IZY,  ORA, 2, 5};
  table[0x15] = {"ORA", ZPX,  ORA, 2, 4};
  table[0x16] = {"ASL", ZPX,  ASL, 2, 6};
  table[0x18] = {"CLC", IMP,  CLC, 1, 2};
  table[0x19] = {"ORA", ABSY, ORA, 3, 4};
  table[0x1D] = {"ORA", ABSX, ORA, 3, 4};
  table[0x1E] = {"ASL", ABSX, ASL, 3, 7};
  table[0x20] = {"JSR", ABS,  JSR, 3, 6};
  table[0x21] = {"AND", IZX,  AND, 2, 6};
  table[0x24] = {"BIT", ZP,   BIT, 2, 3};
  table[0x25] = {"AND", ZP,   AND, 2, 3};
  table[0x26] = {"ROL", ZP,   ROL, 2, 5};
  table[0x28] = {"PLP", IMP,  PLP, 1, 4};
  table[0x29] = {"AND", IMM,  AND, 2, 2};
  table[0x2A] = {"ROL", ACC,  ROL, 1, 2};
  table[0x2C] = {"BIT", ABS,  BIT, 3, 4};
  table[0x2D] = {"AND", ABS,  AND, 3, 4};
  table[0x2E] = {"ROL", ABS,  ROL, 3, 6};
  table[0x30] = {"BMI", REL,  BMI, 2, 2};
  table[0x31] = {"AND", IZY,  AND, 2, 5};
  table[0x35] = {"AND", ZPX,  AND, 2, 4};
  table[0x36] = {"ROL", ZPX,  ROL, 2, 6};
  table[0x38] = {"SEC", IMP,  SEC, 1, 2};
  table[0x39] = {"AND", ABSY, AND, 3, 4};
  table[0x3D] = {"AND", ABSX, AND, 3, 4};
  table[0x3E] = {"ROL", ABSX, ROL, 3, 7};
  table[0x40] = {"RTI", IMP,  RTI, 1, 6};
  table[0x41] = {"EOR", IZX,  EOR, 2, 6};
  table[0x45] = {"EOR", ZP,   EOR, 2, 3};
  table[0x46] = {"LSR", ZP,   LSR, 2, 5};
  table[0x48] = {"PHA", IMP,  PHA, 1, 3};
  table[0x49] = {"EOR", IMM,  EOR, 2, 2};
  table[0x4A] = {"LSR", ACC,  LSR, 1, 2};
  table[0x4C] = {"JMP", ABS,  JMP, 3, 3};
  table[0x4D] = {"EOR", ABS,  EOR, 3, 4};
  table[0x4E] = {"LSR", ABS,  LSR, 3, 6};
  table[0x50] = {"BVC", REL,  BVC, 2, 2};
  table[0x51] = {"EOR", IZY,  EOR, 2, 5};
  table[0x55] = {"EOR", ZPX,  EOR, 2, 4};
  table[0x56] = {"LSR", ZPX,  LSR, 2, 6};
  table[0x58] = {"CLI", IMP,  CLI, 1, 2};
  table[0x59] = {"EOR", ABSY, EOR, 3, 4};
  table[0x5D] = {"EOR", ABSX, EOR, 3, 4};
  table[0x5E] = {"LSR", ABSX, LSR, 3, 7};
  table[0x60] = {"RTS", IMP,  RTS, 1, 6};
  table[0x61] = {"ADC", IZX,  ADC, 2, 6};
  table[0x65] = {"ADC", ZP,   ADC, 2, 3};
  table[0x66] = {"ROR", ZP,   ROR, 2, 5};
  table[0x68] = {"PLA", IMP,  PLA, 1, 4};
  table[0x69] = {"ADC", IMM,  ADC, 2, 2};
  table[0x6A] = {"ROR", ACC,  ROR, 1, 2};
  table[0x6C] = {"JMP", IND,  JMP, 3, 5};
  table[0x6D] = {"ADC", ABS,  ADC, 3, 4};
  table[0x6E] = {"ROR", ABS,  ROR, 3, 6};
  table[0x70] = {"BVS", REL,  BVS, 2, 2};
  table[0x71] = {"ADC", IZY,  ADC, 2, 5};
  table[0x75] = {"ADC", ZPX,  ADC, 2, 4};
  table[0x76] = {"ROR", ZPX,  ROR, 2, 6};
  table[0x78] = {"SEI", IMP,  SEI, 1, 2};
  table[0x79] = {"ADC", ABSY, ADC, 3, 4};
  table[0x7D] = {"ADC", ABSX, ADC, 3, 4};
  table[0x7E] = {"ROR", ABSX, ROR, 3, 7};
  table[0x81] = {"STA", IZX,  STA, 2, 6};
  table[0x84] = {"STY", ZP,   STY, 2, 3};
  table[0x85] = {"STA", ZP,   STA, 2, 3};
  table[0x86] = {"STX", ZP,   STX, 2, 3};
  table[0x88] = {"DEY", IMP,  DEY, 1, 2};
  table[0x8A] = {"TXA", IMP,  TXA, 1, 2};
  table[0x8C] = {"STY", ABS,  STY, 3, 4};
  table[0x8D] = {"STA", ABS,  STA, 3, 4};
  table[0x8E] = {"STX", ABS,  STX, 3, 4};
  table[0x90] = {"BCC", REL,  BCC, 2, 2};
  table[0x91] = {"STA", IZY,  STA, 2, 6};
  table[0x94] = {"STY", ZPX,  STY, 2, 4};
  table[0x95] = {"STA", ZPX,  STA, 2, 4};
  table[0x96] = {"STX", ZPY,  STX, 2, 4};
  table[0x98] = {"TYA", IMP,  TYA, 1, 2};
  table[0x99] = {"STA", ABSY, STA, 3, 5};
  table[0x9A] = {"TXS", IMP,  TXS, 1, 2};
  table[0x9D] = {"STA", ABSX, STA, 3, 5};
  table[0xA0] = {"LDY", IMM,  LDY, 2, 2};
  table[0xA1] = {"LDA", IZX,  LDA, 2, 6};
  table[0xA2] = {"LDX", IMM,  LDX, 2, 2};
  table[0xA4] = {"LDY", ZP,   LDY, 2, 3};
  table[0xA5] = {"LDA", ZP,   LDA, 2, 3};
  table[0xA6] = {"LDX", ZP,   LDX, 2, 3};
  table[0xA8] = {"TAY", IMP,  TAY, 1, 2};
  table[0xA9] = {"LDA", IMM,  LDA, 2, 2};
  table[0xAA] = {"TAX", IMP,  TAX, 1, 2};
  table[0xAC] = {"LDY", ABS,  LDY, 3, 4};
  table[0xAD] = {"LDA", ABS,  LDA, 3, 4};
  table[0xAE] = {"LDX", ABS,  LDX, 3, 4};
  table[0xB0] = {"BCS", REL,  BCS, 2, 2};
  table[0xB1] = {"LDA", IZY,  LDA, 2, 5};
  table[0xB4] = {"LDY", ZPX,  LDY, 2, 4};
  table[0xB5] = {"LDA", ZPX,  LDA, 2, 4};
  table[0xB6] = {"LDX", ZPY,  LDX, 2, 4};
  table[0xB8] = {"CLV", IMP,  CLV, 1, 2};
  table[0xB9] = {"LDA", ABSY, LDA, 3, 4};
  table[0xBA] = {"TSX", IMP,  TSX, 1, 2};
  table[0xBC] = {"LDY", ABSX, LDY, 3, 4};
  table[0xBD] = {"LDA", ABSX, LDA, 3, 4};
  table[0xBE] = {"LDX", ABSY, LDX, 3, 4};
  table[0xC0] = {"CPY", IMM,  CPY, 2, 2};
  table[0xC1] = {"CMP", IZX,  CMP, 2, 6};
  table[0xC4] = {"CPY", ZP,   CPY, 2, 3};
  table[0xC5] = {"CMP", ZP,   CMP, 2, 3};
  table[0xC6] = {"DEC", ZP,   DEC, 2, 5};
  table[0xC8] = {"INY", IMP,  INY, 1, 2};
  table[0xC9] = {"CMP", IMM,  CMP, 2, 2};
  table[0xCA] = {"DEX", IMP,  DEX, 1, 2};
  table[0xCC] = {"CPY", ABS,  CPY, 3, 4};
  table[0xCD] = {"CMP", ABS,  CMP, 3, 4};
  table[0xCE] = {"DEC", ABS,  DEC, 3, 6};
  table[0xD0] = {"BNE", REL,  BNE, 2, 2};
  table[0xD1] = {"CMP", IZY,  CMP, 2, 5};
  table[0xD5] = {"CMP", ZPX,  CMP, 2, 4};
  table[0xD6] = {"DEC", ZPX,  DEC, 2, 6};
  table[0xD8] = {"CLD", IMP,  CLD, 1, 2};
  table[0xD9] = {"CMP", ABSY, CMP, 3, 4};
  table[0xDD] = {"CMP", ABSX, CMP, 3, 4};
  table[0xDE] = {"DEC", ABSX, DEC, 3, 7};
  table[0xE0] = {"CPX", IMM,  CPX, 2, 2};
  table[0xE1] = {"SBC", IZX,  SBC, 2, 6};
  table[0xE4] = {"CPX", ZP,   CPX, 2, 3};
  table[0xE5] = {"SBC", ZP,   SBC, 2, 3};
  table[0xE6] = {"INC", ZP,   INC, 2, 5};
  table[0xE8] = {"INX", IMP,  INX, 1, 2};
  table[0xE9] = {"SBC", IMM,  SBC, 2, 2};
  table[0xEA] = {"NOP", IMP,  NOP, 1, 2};
  table[0xEC] = {"CPX", ABS,  CPX, 3, 4};
  table[0xED] = {"SBC", ABS,  SBC, 3, 4};
  table[0xEE] = {"INC", ABS,  INC, 3, 6};
  table[0xF0] = {"BEQ", REL,  BEQ, 2, 2};
  table[0xF1] = {"SBC", IZY,  SBC, 2, 5};
  table[0xF5] = {"SBC", ZPX,  SBC, 2, 4};
  table[0xF6] = {"INC", ZPX,  INC, 2, 6};
  table[0xF8] = {"SED", IMP,  SED, 1, 2};
  table[0xF9] = {"SBC", ABSY, SBC, 3, 4};
  table[0xFD] = {"SBC", ABSX, SBC, 3, 4};
  table[0xFE] = {"INC", ABSX, INC, 3, 7};
}


StepTrace step(CPU& cpu) {
  uint16_t oldPC = cpu.PC;
  uint8_t opcode = cpu.read(cpu.PC++);
  Instruction& inst = table[opcode];

  cpu.atAccumulator = false;

  if (inst.addrmode) inst.addrmode(cpu);
  if (inst.execute) inst.execute(cpu);

  return {oldPC, opcode, inst.name};
}
