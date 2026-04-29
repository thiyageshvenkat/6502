# 6502
6502 compiler, IDE (Dear ImGui)

So far mostly functional 6502 chip, needs opcodes
PPU and other elements of the NES still need to be emulated

## Run
```bash
g++ NES.cpp NESCpu.cpp NESInstructions.cpp -o NESCpu
.\NESCpu

```
