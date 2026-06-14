#include "NESCpu.h"
#include "NESInstructions.h"
#include "NESAssembler.h"
#include <iostream>

int main() {
  CPU cpu;
  initTable();

  std::vector<uint8_t> program = assembleFile("program.asm");

  for (size_t i = 0; i < program.size(); i++) {
    cpu.memory[0x8000 + i] = program[i];
  }

  // set reset vector to 0x8000, where our program will be loaded
  cpu.memory[0xFFFC] = 0x00;
  cpu.memory[0xFFFD] = 0x80;
  cpu.memory[0xFFFE] = 0x00;
  cpu.memory[0xFFFF] = 0x90;
  
  cpu.reset();

  for (int i = 0; i < 100; i++) {
    step(cpu);

    std::cout << "PC=$" << std::hex << cpu.PC
      << " a=" << std::dec << (int)cpu.memory[0x00]
      << " b=" << (int)cpu.memory[0x01] << "\n";
  }

  return 0;
}
