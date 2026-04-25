#include <iostream>
#include <cstdint>

uint8_t mem[65536];


class op {
  public:
    uint8_t bytes;
    uint8_t cycles;
    unsigned char array[4];
    bool write; // if write then 1, if read than 0
    unsigned char addressing_mode[0];
    bool crash; // if 1, CRASH is printed if the BRK instruction is not detected after 100 half-cycles
}

class A {
  public:
    uint8_t data;
}

class X {
  public:
    uint8_t data;
}

class Y {
  public:
    uint8_t data;
}

class SP {
  public:
    uint16_t data;
}


class PC {
  public:
    uint16_t data;
}

class P {
  public:
    bool array[7] data;
}
class op::LDA {
  
}
int main() {

}
