#include "NESCpu.h"
#include "NESInstructions.h"
#include <iostream>

int main() {
  CPU cpu;
  initTable();

  cpu.PC = 0x0000;

  cpu.memory[0x0000] = 0xA9; // LDA #1
  cpu.memory[0x0001] = 0x01;

  cpu.memory[0x0002] = 0x85; // STA $80
  cpu.memory[0x0003] = 0x80;

  cpu.memory[0x0004] = 0xA9; // LDA #1
  cpu.memory[0x0005] = 0x01;

  cpu.memory[0x0006] = 0x85; // STA $81
  cpu.memory[0x0007] = 0x81;

  cpu.memory[0x0008] = 0xA5; // LDA $80
  cpu.memory[0x0009] = 0x80;

  cpu.memory[0x000A] = 0x18; // CLC

  cpu.memory[0x000B] = 0x65; // ADC $81
  cpu.memory[0x000C] = 0x81;

  cpu.memory[0x000D] = 0x85; // STA $82
  cpu.memory[0x000E] = 0x82;

  cpu.memory[0x000F] = 0xA5; // LDA $81
  cpu.memory[0x0010] = 0x81;

  cpu.memory[0x0011] = 0x85; // STA $80
  cpu.memory[0x0012] = 0x80;

  cpu.memory[0x0013] = 0xA5; // LDA $82
  cpu.memory[0x0014] = 0x82;

  cpu.memory[0x0015] = 0x85; // STA $81
  cpu.memory[0x0016] = 0x81;

  cpu.memory[0x0017] = 0x4C; // JMP $0008
  cpu.memory[0x0018] = 0x08;
  cpu.memory[0x0019] = 0x00;

  for (int i = 0; i < 100; i++) {
    step(cpu);

    if (cpu.PC == 0x0008) {
      std::cout << "a=" << (int)cpu.memory[0x80]
        << " b=" << (int)cpu.memory[0x81] << "\n";
    }
  }

  return 0;
}
