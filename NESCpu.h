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
  /*
  Bit 7: Negative (N)
  Bit 6: Overflow (V)
  Bit 5: Unused (always 1)
  Bit 4: Break (B)
  Bit 3: Decimal (D)
  Bit 2: Interrupt Disable (I)
  Bit 1: Zero (Z)
  Bit 0: Carry (C)
  */
  uint16_t PC = 0;      // Program Counter
  bool atAccumulator = false;     // whether the instruction operates directly on the accumulator (e.g. ASL A)
  bool jammed = false;  // unofficial KIL/JAM opcodes lock the CPU
  uint8_t memory[65536]{}; // initialize full 64KB memory

  // read helper function (reads a byte from memory)
  uint8_t read(uint16_t addr);

  // reset function to initialize PC from reset vector
  void reset();

  // write helper function (writes a byte to memory)
  void write(uint16_t addr, uint8_t value);

  // fetches operand from the address set by addressing mode
  uint8_t fetch();

  // sets Zero and Negative flags based on value
  void setZN(uint8_t value); // bit manipulation to speed up
};
