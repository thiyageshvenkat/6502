#include "NESAssembler.h"
#include "NESInstructions.h"

#include <sstream>
#include <stdexcept>
#include <fstream>
#include <cctype>
#include <string>
#include <vector>

using AddrMode = uint8_t (*)(CPU&); // alias for the function type of address mode functions

static std::string trim(std::string s) { // trim whitespace before and after instruction, remove comments
  size_t comment = s.find(';');
  if (comment != std::string::npos) { // remove comment
    s = s.substr(0, comment);
  }
  while (!s.empty() && std::isspace((unsigned char)s.front())) { s.erase(s.begin()); }
  while (!s.empty() && std::isspace((unsigned char)s.back())) { s.pop_back(); }
  return s;
}

static uint16_t parseNumber(std::string s) { // extract number from operand
  if (s[0] == '#') { s = s.substr(1); }
  if (s[0] == '$') { return std::stoi(s.substr(1), nullptr, 16); } // edge case
  return std::stoi(s, nullptr, 10);
}

static int findOpcode(const std::string& name, AddrMode mode) {
  for (int i = 0; i < 256; i++) { // O(N): check each instruction in instruction table for exact address mode and name
    if (!table[i].name) { continue; }
    if (name != table[i].name) { continue; }
    if (table[i].addrmode != mode) { continue; }
    return i;
  }

  return -1;
}

static int findOpcodeType(const std::string& name, const std::string& arg) {
  if (arg.empty()) { return findOpcode(name, nullptr); } // IMP: no operand
  if (arg[0] == '#') { return findOpcode(name, IMM); } // IMM: #$01
  int rel = findOpcode(name, REL);
  if (rel != -1) { return rel; } // REL, for branches
  uint16_t value = parseNumber(arg); // absolute/zero-page number
  int zp = findOpcode(name, ZP);
  if (value <= 0xFF && zp != -1) { return zp; } // ZP: $00, one byte address (<=0xFF)
  int abs = findOpcode(name, ABS);
  if (abs != -1) { return abs; } // ABS

  return -1;
}

std::vector<uint8_t> assembleFile(const std::string& src) {
  std::vector<uint8_t> bytes; // return assembled machine-code bytes
  std::ifstream input(src);
  if (!input) { throw std::runtime_error("Could not open assembly file: " + src); } // if file not present
  std::string line;

  while (std::getline(input, line)) { // get line from input
    line = trim(line);
    if (line.empty()) { continue; }

    std::istringstream iss(line);

    std::string op;
    std::string arg;

    iss >> op;
    std::getline(iss, arg);
    arg = trim(arg); // parse operand

    // --- find opcode byte ---
    int opcode = findOpcodeType(op, arg); // this calls helper functions to search in the instruction table

    if (opcode == -1) { throw std::runtime_error("NO OPCODE FOUND FOR: " + line); } // edge case for no/invalid opcode

    // --- push opcode into bytes vector ---
    bytes.push_back((uint8_t)opcode);

    // --- push operand into bytes vector
    if (table[opcode].bytes == 2) {
      bytes.push_back((uint8_t)parseNumber(arg));
    }

    else if (table[opcode].bytes == 3) { // if two bytes as operand, store in little-endian format
      uint16_t value = parseNumber(arg);
      bytes.push_back(value & 0xFF);
      bytes.push_back((value >> 8) & 0xFF);
    }
  }

  return bytes;
}
