#pragma once
#include "NESCpu.h"
#include <cstdint>

// executes one instruction and returns trace info
StepTrace step(CPU& cpu);

struct Instruction {
  const char* name; // name, for debugging purposes

  uint8_t (*addrmode)(CPU&); 
  // what addressing mode is this instruction
  // determines how the operand address is calculated

  void (*execute)(CPU&); 
  // pointer to the actual instruction function (what the opcode does)

  uint8_t bytes; 
  // 1 byte -> implicit commands (no operands)
  // 2 byte -> opcode + 1 byte operand
  // 3 byte -> opcode + 2 byte operand (absolute addressing)

  int cycles; 
  // number of CPU cycles this instruction takes
};

// global opcode table (256 possible opcodes)
extern Instruction table[256];

// initializes the opcode table
void initTable();
