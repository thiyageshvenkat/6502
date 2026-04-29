#pragma once
#include <cstdint>

// used for returning information about each executed step
struct StepTrace {
  uint16_t pc;        // program counter before executing instruction
  uint8_t opcode;     // raw opcode value
  const char* name;   // instruction name (for debugging)
};

struct CPU {
  uint16_t address = 0; // stores where the operand is located
  uint8_t fetched = 0;  // reads the operand byte from the location determined by the addressing mode

  // Registers
  uint8_t A = 0;        // Accumulator
  uint8_t X = 0;        // Index register X
  uint8_t Y = 0;        // Index register Y
  uint8_t SP = 0xFD;    // Stack Pointer (starts at 0xFD for 6502)
  uint8_t P = 0;        // Status flags register
  uint16_t PC = 0;      // Program Counter

  uint8_t memory[65536]{}; // initialize full 64KB memory

  // read helper function (reads a byte from memory)
  uint8_t read(uint16_t addr);

  // write helper function (writes a byte to memory)
  void write(uint16_t addr, uint8_t value);

  // fetches operand from the address set by addressing mode
  uint8_t fetch();

  // sets Zero and Negative flags based on value
  void setZN(uint8_t value); // bit manipulation to speed up
};
