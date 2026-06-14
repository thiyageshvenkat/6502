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

static std::string compact(std::string s) {
  std::string out;
  for (char c : s) {
    if (!std::isspace((unsigned char)c)) {
      out.push_back(c);
    }
  }
  return out;
}

static std::string upper(std::string s) {
  for (char& c : s) {
    c = (char)std::toupper((unsigned char)c);
  }
  return s;
}

static bool endsWith(const std::string& s, const std::string& suffix) {
  return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::string stripOperandSyntax(std::string s) {
  s = upper(compact(trim(s)));

  if (!s.empty() && s[0] == '#') {
    s = s.substr(1);
  }

  if (s.size() >= 4 && s[0] == '(' && endsWith(s, ",X)")) {
    return s.substr(1, s.size() - 4);
  }
  if (s.size() >= 4 && s[0] == '(' && endsWith(s, "),Y")) {
    return s.substr(1, s.size() - 4);
  }
  if (s.size() >= 3 && s[0] == '(' && s.back() == ')') {
    return s.substr(1, s.size() - 2);
  }
  if (s.size() >= 3 && (endsWith(s, ",X") || endsWith(s, ",Y"))) {
    return s.substr(0, s.size() - 2);
  }

  return s;
}

static uint16_t parseNumber(std::string s) { // extract number from operand
  s = stripOperandSyntax(s);
  if (s.empty()) { throw std::runtime_error("Missing numeric operand"); }
  if (s[0] == '$') { return std::stoi(s.substr(1), nullptr, 16); } // edge case
  return std::stoi(s, nullptr, 10);
}

static bool isZeroPageOperand(const std::string& arg) {
  std::string number = stripOperandSyntax(arg);
  if (number.empty()) { return false; }
  if (number[0] == '$') {
    return number.size() <= 3; // "$" plus two hex digits
  }
  return parseNumber(number) <= 0xFF;
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

static int findOpcodeType(const std::string& rawName, const std::string& rawArg) {
  std::string name = upper(rawName);
  std::string arg = upper(compact(trim(rawArg)));

  // IMP: no operand
  if (arg.empty()) {
    return findOpcode(name, IMP);
  }

  // ACC: A
  if (arg == "A") {
    return findOpcode(name, ACC);
  }

  // IMM: #$01
  if (arg[0] == '#') {
    return findOpcode(name, IMM);
  }

  // IZX: ($nn,X)
  if (arg.size() >= 4 && arg[0] == '(' && endsWith(arg, ",X)")) {
    if (parseNumber(arg) <= 0xFF) {
      return findOpcode(name, IZX);
    }
    return -1;
  }

  // IZY: ($nn),Y
  if (arg.size() >= 4 && arg[0] == '(' && endsWith(arg, "),Y")) {
    if (parseNumber(arg) <= 0xFF) {
      return findOpcode(name, IZY);
    }
    return -1;
  }

  // IND: ($nnnn)
  if (arg.size() >= 3 && arg[0] == '(' && arg.back() == ')') {
    return findOpcode(name, IND);
  }

  // ZPX/ABSX: $nn,X or $nnnn,X
  if (endsWith(arg, ",X")) {
    if (isZeroPageOperand(arg)) {
      int zpx = findOpcode(name, ZPX);
      if (zpx != -1) { return zpx; }
    }

    return findOpcode(name, ABSX);
  }

  // ZPY/ABSY: $nn,Y or $nnnn,Y
  if (endsWith(arg, ",Y")) {
    if (isZeroPageOperand(arg)) {
      int zpy = findOpcode(name, ZPY);
      if (zpy != -1) { return zpy; }
    }

    return findOpcode(name, ABSY);
  }

  // REL, for branches
  int rel = findOpcode(name, REL);
  if (rel != -1) {
    return rel;
  }

  // ZP/ABS: $nn or $nnnn
  if (isZeroPageOperand(arg)) {
    int zp = findOpcode(name, ZP);
    if (zp != -1) { return zp; }
  }

  return findOpcode(name, ABS);
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
