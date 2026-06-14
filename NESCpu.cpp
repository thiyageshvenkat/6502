#include "NESCpu.h"

uint8_t CPU::read(uint16_t addr) { // read helper function
  return memory[addr];
}

void CPU::write(uint16_t addr, uint8_t value) { // write helper function
  memory[addr] = value;
}

void CPU::reset() { // initialize PC from reset vector
  uint16_t low = read(0xFFFC);
  uint16_t high = read(0xFFFD);
  PC = ((uint16_t)high << 8) | low;
  atAccumulator = false;
  jammed = false;
}

uint8_t CPU::fetch() {
  fetched = read(address);
  return fetched;
}

void CPU::setZN(uint8_t value) { // bit manipulation to speed up
  P &= ~(0x02 | 0x80);
  P |= (value == 0) << 1; // sets flag for if zero
  P |= (value & 0x80); // sets flag for if negative
}
