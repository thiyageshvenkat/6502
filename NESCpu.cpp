#include "NESCpu.h"

uint8_t CPU::read(uint16_t addr) { // read helper function
  return memory[addr];
}

void CPU::write(uint16_t addr, uint8_t value) { // write helper function
  memory[addr] = value;
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
