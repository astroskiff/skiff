#ifndef LIBSKIFF_INSTRUCTIONS_HPP
#define LIBSKIFF_INSTRUCTIONS_HPP

#include <cstdint>
#include <unordered_map>

namespace libskiff {
namespace bytecode {
namespace instructions {

constexpr uint8_t NOP = 0x00;
constexpr uint8_t EXIT = 0x01;
constexpr uint8_t BLT = 0x02;
constexpr uint8_t BGT = 0x03;
constexpr uint8_t BEQ = 0x04;
constexpr uint8_t JMP = 0x05;
constexpr uint8_t CALL = 0x06;
constexpr uint8_t RET = 0x07;
constexpr uint8_t MOV = 0x08;
constexpr uint8_t ADD = 0x09;
constexpr uint8_t SUB = 0x0A;
constexpr uint8_t DIV = 0x0B;
constexpr uint8_t MUL = 0x0C;
constexpr uint8_t ADDF = 0x0D;
constexpr uint8_t SUBF = 0x0E;
constexpr uint8_t DIVF = 0x0F;
constexpr uint8_t MULF = 0x10;
constexpr uint8_t LSH = 0x11;
constexpr uint8_t RSH = 0x12;
constexpr uint8_t AND = 0x13;
constexpr uint8_t OR = 0x14;
constexpr uint8_t XOR = 0x15;
constexpr uint8_t NOT = 0x16;
constexpr uint8_t BLTF = 0x17;
constexpr uint8_t BGTF = 0x18;
constexpr uint8_t BEQF = 0x19;
constexpr uint8_t ASEQ = 0x1A;
constexpr uint8_t ASNE = 0x1B;
constexpr uint8_t PUSH_W = 0x1C;
constexpr uint8_t PUSH_DW = 0x1D;
constexpr uint8_t PUSH_QW = 0x1E;
constexpr uint8_t POP_W = 0x1F;
constexpr uint8_t POP_DW = 0x20;
constexpr uint8_t POP_QW = 0x21;
constexpr uint8_t ALLOC = 0x22;
constexpr uint8_t FREE = 0x23;
constexpr uint8_t SW = 0x24;
constexpr uint8_t SDW = 0x25;
constexpr uint8_t SQW = 0x26;
constexpr uint8_t LW = 0x27;
constexpr uint8_t LDW = 0x28;
constexpr uint8_t LQW = 0x29;
constexpr uint8_t SYSCALL = 0x2A;
constexpr uint8_t DEBUG = 0x2B;
constexpr uint8_t EIRQ = 0x2C;
constexpr uint8_t DIRQ = 0x2D;

//! \brief Retrieve the map that details the size of each instruction in bytes
static inline std::unordered_map<uint8_t, uint8_t> get_instruction_to_size_map()
{
  return {{libskiff::bytecode::instructions::NOP, 1},
          {libskiff::bytecode::instructions::EXIT, 1},
          {libskiff::bytecode::instructions::BLT, 11},
          {libskiff::bytecode::instructions::BGT, 11},
          {libskiff::bytecode::instructions::BEQ, 11},
          {libskiff::bytecode::instructions::JMP, 9},
          {libskiff::bytecode::instructions::CALL, 9},
          {libskiff::bytecode::instructions::RET, 1},
          {libskiff::bytecode::instructions::MOV, 10},
          {libskiff::bytecode::instructions::ADD, 4},
          {libskiff::bytecode::instructions::SUB, 4},
          {libskiff::bytecode::instructions::DIV, 4},
          {libskiff::bytecode::instructions::MUL, 4},
          {libskiff::bytecode::instructions::ADDF, 4},
          {libskiff::bytecode::instructions::SUBF, 4},
          {libskiff::bytecode::instructions::DIVF, 4},
          {libskiff::bytecode::instructions::MULF, 4},
          {libskiff::bytecode::instructions::LSH, 4},
          {libskiff::bytecode::instructions::RSH, 4},
          {libskiff::bytecode::instructions::AND, 4},
          {libskiff::bytecode::instructions::OR, 4},
          {libskiff::bytecode::instructions::XOR, 4},
          {libskiff::bytecode::instructions::NOT, 3},
          {libskiff::bytecode::instructions::BLTF, 11},
          {libskiff::bytecode::instructions::BGTF, 11},
          {libskiff::bytecode::instructions::BEQF, 11},
          {libskiff::bytecode::instructions::ASEQ, 3},
          {libskiff::bytecode::instructions::ASNE, 3},
          {libskiff::bytecode::instructions::PUSH_W, 2},
          {libskiff::bytecode::instructions::PUSH_DW, 2},
          {libskiff::bytecode::instructions::PUSH_QW, 2},
          {libskiff::bytecode::instructions::POP_W, 2},
          {libskiff::bytecode::instructions::POP_DW, 2},
          {libskiff::bytecode::instructions::POP_QW, 2},
          {libskiff::bytecode::instructions::ALLOC, 3},
          {libskiff::bytecode::instructions::FREE, 2},
          {libskiff::bytecode::instructions::SW, 4},
          {libskiff::bytecode::instructions::SDW, 4},
          {libskiff::bytecode::instructions::SQW, 4},
          {libskiff::bytecode::instructions::LW, 4},
          {libskiff::bytecode::instructions::LDW, 4},
          {libskiff::bytecode::instructions::LQW, 4},
          {libskiff::bytecode::instructions::SYSCALL, 9},
          {libskiff::bytecode::instructions::DEBUG, 9},
          {libskiff::bytecode::instructions::EIRQ, 1},
          {libskiff::bytecode::instructions::DIRQ, 1}};
}

} // namespace instructions
} // namespace bytecode
} // namespace libskiff

#endif
